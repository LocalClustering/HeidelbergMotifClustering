/*******************************************************************************
 * This file is part of Mt-KaHyPar.
 *
 * Copyright (C) 2021 Tobias Heuer <tobias.heuer@kit.edu>
 *
 * Mt-KaHyPar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mt-KaHyPar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mt-KaHyPar.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/


#include "mt-kahypar/partition/refinement/flows/problem_construction.h"

#include <unordered_map>

#include "tbb/parallel_for.h"

namespace mt_kahypar {

void ProblemConstruction::BFSData::clearQueue() {
  while ( !queue.empty() ) queue.pop();
  while ( !next_queue.empty() ) next_queue.pop();
}

void ProblemConstruction::BFSData::reset() {
  current_distance = 0;
  queue_weight_block_0 = 0;
  queue_weight_block_1 = 0;
  lock_queue = false;
  clearQueue();
  std::fill(visited_hn.begin(), visited_hn.end(), false);
  std::fill(visited_he.begin(), visited_he.end(), false);
  std::fill(contained_hes.begin(), contained_hes.end(), false);
  std::fill(locked_blocks.begin(), locked_blocks.end(), false);
}

HypernodeID ProblemConstruction::BFSData::pop_hypernode() {
  ASSERT(!queue.empty());
  const HypernodeID hn = queue.front();
  queue.pop();
  return hn;
}

void ProblemConstruction::BFSData::add_pins_of_hyperedge_to_queue(
  const HyperedgeID& he,
  const PartitionedHypergraph& phg,
  const size_t max_bfs_distance,
  const HypernodeWeight max_weight_block_0,
  const HypernodeWeight max_weight_block_1) {
  if ( current_distance <= max_bfs_distance && !lock_queue ) {
    if ( !visited_he[he] ) {
      for ( const HypernodeID& pin : phg.pins(he) ) {
        if ( !visited_hn[pin] ) {
          const PartitionID block = phg.partID(pin);
          const bool is_block_0 = blocks.i == block;
          const bool is_block_1 = blocks.j == block;
          if ( (is_block_0 || is_block_1) && !locked_blocks[block] ) {
            next_queue.push(pin);
            queue_weight_block_0 += is_block_0 ? phg.nodeWeight(pin) : 0;
            queue_weight_block_1 += is_block_1 ? phg.nodeWeight(pin) : 0;
          }
          visited_hn[pin] = true;
        }
      }
      visited_he[he] = true;
    }
  }

  if ( queue_weight_block_0 >= max_weight_block_0 &&
       queue_weight_block_1 >= max_weight_block_1 ) {
    lock_queue = true;
  }
}

namespace {
  using assert_map = std::unordered_map<HyperedgeID, bool>;
}

Subhypergraph ProblemConstruction::construct(const SearchID search_id,
                                             QuotientGraph& quotient_graph,
                                             const PartitionedHypergraph& phg) {
  Subhypergraph sub_hg;
  BFSData& bfs = _local_bfs.local();
  bfs.reset();
  bfs.blocks = quotient_graph.getBlockPair(search_id);
  sub_hg.block_0 = bfs.blocks.i;
  sub_hg.block_1 = bfs.blocks.j;
  sub_hg.weight_of_block_0 = 0;
  sub_hg.weight_of_block_1 = 0;
  sub_hg.num_pins = 0;
  const HypernodeWeight max_weight_block_0 =
    _scaling * _context.partition.perfect_balance_part_weights[sub_hg.block_1] - phg.partWeight(sub_hg.block_1);
  const HypernodeWeight max_weight_block_1 =
    _scaling * _context.partition.perfect_balance_part_weights[sub_hg.block_0] - phg.partWeight(sub_hg.block_0);
  const size_t max_bfs_distance = _context.refinement.flows.max_bfs_distance;


  // We initialize the BFS with all cut hyperedges running
  // between the involved block associated with the search
  bfs.clearQueue();
  quotient_graph.doForAllCutHyperedgesOfSearch(search_id, [&](const HyperedgeID& he) {
    bfs.add_pins_of_hyperedge_to_queue(he, phg, max_bfs_distance,
      max_weight_block_0, max_weight_block_1);
  });
  bfs.swap_with_next_queue();

  // BFS
  while ( !bfs.is_empty() &&
          !isMaximumProblemSizeReached(sub_hg,
            max_weight_block_0, max_weight_block_1, bfs.locked_blocks) ) {
    HypernodeID hn = bfs.pop_hypernode();
    PartitionID block = phg.partID(hn);
    const bool is_block_contained = block == sub_hg.block_0 || block == sub_hg.block_1;
    if ( is_block_contained && !bfs.locked_blocks[block] ) {
      if ( sub_hg.block_0  == block ) {
        sub_hg.nodes_of_block_0.push_back(hn);
        sub_hg.weight_of_block_0 += phg.nodeWeight(hn);
      } else {
        ASSERT(sub_hg.block_1 == block);
        sub_hg.nodes_of_block_1.push_back(hn);
        sub_hg.weight_of_block_1 += phg.nodeWeight(hn);
      }
      sub_hg.num_pins += phg.nodeDegree(hn);

      // Push all neighbors of the added vertex into the queue
      for ( const HyperedgeID& he : phg.incidentEdges(hn) ) {
        bfs.add_pins_of_hyperedge_to_queue(he, phg, max_bfs_distance,
          max_weight_block_0, max_weight_block_1);
        if ( !bfs.contained_hes[phg.uniqueEdgeID(he)] ) {
          sub_hg.hes.push_back(he);
          bfs.contained_hes[phg.uniqueEdgeID(he)] = true;
        }
      }
    }

    if ( bfs.is_empty() ) {
      bfs.swap_with_next_queue();
    }
  }

  DBG << "Search ID:" << search_id << "-" << sub_hg;

  // Check if all touched hyperedges are contained in subhypergraph
  ASSERT([&]() {
    assert_map expected_hes;
    for ( const HyperedgeID& he : sub_hg.hes ) {
      const HyperedgeID id = phg.uniqueEdgeID(he);
      if ( expected_hes.count(id) > 0 ) {
        LOG << "Hyperedge" << he << "is contained multiple times in subhypergraph!";
        return false;
      }
      expected_hes[id] = true;
    }

    for ( const HypernodeID& hn : sub_hg.nodes_of_block_0 ) {
      for ( const HyperedgeID& he : phg.incidentEdges(hn) ) {
        const HyperedgeID id = phg.uniqueEdgeID(he);
        if ( expected_hes.count(id) == 0 ) {
          LOG << "Hyperedge" << he << "not contained in subhypergraph!";
          return false;
        }
        expected_hes[id] = false;
      }
    }

    for ( const HypernodeID& hn : sub_hg.nodes_of_block_1 ) {
      for ( const HyperedgeID& he : phg.incidentEdges(hn) ) {
        const HyperedgeID id = phg.uniqueEdgeID(he);
        if ( expected_hes.count(id) == 0 ) {
          LOG << "Hyperedge" << he << "not contained in subhypergraph!";
          return false;
        }
        expected_hes[id] = false;
      }
    }

    for ( const auto& entry : expected_hes ) {
      const HyperedgeID he = entry.first;
      const bool visited = !entry.second;
      if ( !visited ) {
        LOG << "HyperedgeID" << he << "should be not part of subhypergraph!";
        return false;
      }
    }
    return true;
  }(), "Subhypergraph construction failed!");

  return sub_hg;
}

MT_KAHYPAR_ATTRIBUTE_ALWAYS_INLINE bool ProblemConstruction::isMaximumProblemSizeReached(
  const Subhypergraph& sub_hg,
  const HypernodeWeight max_weight_block_0,
  const HypernodeWeight max_weight_block_1,
  vec<bool>& locked_blocks) const {
  if ( sub_hg.weight_of_block_0 >= max_weight_block_0 ) {
    locked_blocks[sub_hg.block_0] = true;
  }
  if ( sub_hg.weight_of_block_1 >= max_weight_block_1 ) {
    locked_blocks[sub_hg.block_1] = true;
  }
  if ( sub_hg.num_pins >= _context.refinement.flows.max_num_pins ) {
    locked_blocks[sub_hg.block_0] = true;
    locked_blocks[sub_hg.block_1] = true;
  }

  return locked_blocks[sub_hg.block_0] && locked_blocks[sub_hg.block_1];
}

} // namespace mt_kahypar