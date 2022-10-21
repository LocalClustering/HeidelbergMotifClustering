/*******************************************************************************
 * This file is part of Mt-KaHyPar.
 *
 * Copyright (C) 2020 Tobias Heuer <tobias.heuer@kit.edu>
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

#include "mt-kahypar/definitions.h"
#include "mt-kahypar/partition/context.h"
#include "mt-kahypar/parallel/stl/scalable_vector.h"

namespace mt_kahypar {

class LargeHyperedgeRemover {

 #define LARGE_HE_THRESHOLD ID(50000)

 public:
  LargeHyperedgeRemover(const Context& context) :
    _context(context),
    _removed_hes() { }

  LargeHyperedgeRemover(const LargeHyperedgeRemover&) = delete;
  LargeHyperedgeRemover & operator= (const LargeHyperedgeRemover &) = delete;

  LargeHyperedgeRemover(LargeHyperedgeRemover&&) = delete;
  LargeHyperedgeRemover & operator= (LargeHyperedgeRemover &&) = delete;

  // ! Removes large hyperedges from the hypergraph
  // ! Returns the number of removed large hyperedges.
  HypernodeID removeLargeHyperedges(Hypergraph& hypergraph) {
    HypernodeID num_removed_large_hyperedges = 0;
    #ifndef USE_GRAPH_PARTITIONER
    for ( const HyperedgeID& he : hypergraph.edges() ) {
      if ( hypergraph.edgeSize(he) > largeHyperedgeThreshold() ) {
        hypergraph.removeLargeEdge(he);
        _removed_hes.push_back(he);
        ++num_removed_large_hyperedges;
      }
    }
    std::reverse(_removed_hes.begin(), _removed_hes.end());
    #else
    unused(hypergraph);
    #endif
    return num_removed_large_hyperedges;
  }

  // ! Before we start a v-cycle, we reset the hypergraph data structure.
  // ! This causes that all removed hyperedges in the dynamic hypergraph are
  // ! reinserted to the incident nets of each vertex. By simply calling this
  // ! function, we remove all large hyperedges again.
  void removeLargeHyperedgesInNLevelVCycle(Hypergraph& hypergraph) {
    for ( const HyperedgeID& he : _removed_hes ) {
      hypergraph.enableHyperedge(he);
      hypergraph.removeLargeEdge(he);
    }
  }

  // ! Restores all previously removed large hyperedges
  void restoreLargeHyperedges(PartitionedHypergraph& hypergraph) {
    HyperedgeWeight delta = 0;
    for ( const HyperedgeID& he : _removed_hes ) {
      hypergraph.restoreLargeEdge(he);
      if ( _context.partition.objective == kahypar::Objective::cut ) {
         delta += (hypergraph.connectivity(he) > 1 ? hypergraph.edgeWeight(he) : 0);
       } else {
         delta += (hypergraph.connectivity(he) - 1) * hypergraph.edgeWeight(he);
       }
    }

    if ( _context.partition.verbose_output && delta > 0 ) {
      LOG << RED << "Restoring of" << _removed_hes.size() << "large hyperedges (|e| >"
          << largeHyperedgeThreshold() << ") increased" << _context.partition.objective
          << "by" << delta << END;
    }
  }

  HypernodeID largeHyperedgeThreshold() const {
    return std::max(_context.partition.large_hyperedge_size_threshold, LARGE_HE_THRESHOLD);
  }

  void reset() {
    _removed_hes.clear();
  }

 private:
  const Context& _context;
  parallel::scalable_vector<HypernodeID> _removed_hes;
};

}  // namespace mt_kahypar
