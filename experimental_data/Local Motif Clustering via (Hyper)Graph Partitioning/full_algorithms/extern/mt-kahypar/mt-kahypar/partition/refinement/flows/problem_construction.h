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

#pragma once

#include "tbb/enumerable_thread_specific.h"

#include "mt-kahypar/definitions.h"
#include "mt-kahypar/partition/context.h"
#include "mt-kahypar/datastructures/sparse_map.h"
#include "mt-kahypar/partition/refinement/flows/refiner_adapter.h"
#include "mt-kahypar/partition/refinement/flows/quotient_graph.h"
#include "mt-kahypar/parallel/stl/scalable_vector.h"
#include "mt-kahypar/parallel/stl/scalable_queue.h"
#include "mt-kahypar/parallel/atomic_wrapper.h"

namespace mt_kahypar {

class ProblemConstruction {

  static constexpr bool debug = false;

  /**
   * Contains data required to grow two region around
   * the cut of two blocks of the partition.
   */
  struct BFSData {
    explicit BFSData(const HypernodeID num_nodes,
                     const HyperedgeID num_edges,
                     const PartitionID k) :
      current_distance(0),
      queue(),
      next_queue(),
      visited_hn(num_nodes, false),
      visited_he(num_edges, false),
      contained_hes(num_edges, false),
      locked_blocks(k, false),
      queue_weight_block_0(0),
      queue_weight_block_1(0),
      lock_queue(false)  { }

    void clearQueue();

    void reset();

    HypernodeID pop_hypernode();

    void add_pins_of_hyperedge_to_queue(const HyperedgeID& he,
                                        const PartitionedHypergraph& phg,
                                        const size_t max_bfs_distance,
                                        const HypernodeWeight max_weight_block_0,
                                        const HypernodeWeight max_weight_block_1);

    bool is_empty() const {
      return queue.empty();
    }

    bool is_next_empty() const {
      return next_queue.empty();
    }

    void swap_with_next_queue() {
      if ( !is_next_empty() ) {
        std::swap(queue, next_queue);
        ++current_distance;
      }
    }

    BlockPair blocks;
    size_t current_distance;
    parallel::scalable_queue<HypernodeID> queue;
    parallel::scalable_queue<HypernodeID> next_queue;
    vec<bool> visited_hn;
    vec<bool> visited_he;
    vec<bool> contained_hes;
    vec<bool> locked_blocks;
    HypernodeWeight queue_weight_block_0;
    HypernodeWeight queue_weight_block_1;
    bool lock_queue;
  };

 public:
  explicit ProblemConstruction(const Hypergraph& hg,
                               const Context& context) :
    _context(context),
    _scaling(1.0 + _context.refinement.flows.alpha *
      std::min(0.05, _context.partition.epsilon)),
    _local_bfs(hg.initialNumNodes(), hg.initialNumEdges(), context.partition.k) { }

  ProblemConstruction(const ProblemConstruction&) = delete;
  ProblemConstruction(ProblemConstruction&&) = delete;

  ProblemConstruction & operator= (const ProblemConstruction &) = delete;
  ProblemConstruction & operator= (ProblemConstruction &&) = delete;

  Subhypergraph construct(const SearchID search_id,
                          QuotientGraph& quotient_graph,
                          const PartitionedHypergraph& phg);

 private:

  MT_KAHYPAR_ATTRIBUTE_ALWAYS_INLINE bool isMaximumProblemSizeReached(
    const Subhypergraph& sub_hg,
    const HypernodeWeight max_weight_block_0,
    const HypernodeWeight max_weight_block_1,
    vec<bool>& locked_blocks) const;

  const Context& _context;
  double _scaling;

  // ! Contains data required for BFS construction algorithm
  tbb::enumerable_thread_specific<BFSData> _local_bfs;
};

}  // namespace kahypar
