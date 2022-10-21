/*******************************************************************************
 * This file is part of Mt-KaHyPar.
 *
 * Copyright (C) 2020 Lars Gottesbüren <lars.gottesbueren@kit.edu>
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


#include "mt-kahypar/datastructures/sparse_map.h"
#include "mt-kahypar/datastructures/buffered_vector.h"

#include "mt-kahypar/macros.h"
#include "mt-kahypar/datastructures/graph.h"
#include "mt-kahypar/partition/context.h"
#include "mt-kahypar/parallel/atomic_wrapper.h"
#include "mt-kahypar/utils/randomize.h"
#include "mt-kahypar/utils/reproducible_random.h"


#include "gtest/gtest_prod.h"

namespace mt_kahypar::metrics {
  double modularity(const Graph& graph, const ds::Clustering& communities);
}

namespace mt_kahypar::community_detection {


class ParallelLocalMovingModularity {
 private:
  using LargeIncidentClusterWeights = ds::FixedSizeSparseMap<PartitionID, ArcWeight>;
  using CacheEfficientIncidentClusterWeights = ds::FixedSizeSparseMap<PartitionID, ArcWeight>;

 public:
  static constexpr bool debug = false;
  static constexpr bool enable_heavy_assert = false;

  ParallelLocalMovingModularity(const Context& context,
                                         size_t numNodes,
                                         const bool disable_randomization = false) :
          _context(context),
          _max_degree(numNodes),
          _vertex_degree_sampling_threshold(context.preprocessing.community_detection.vertex_degree_sampling_threshold),
          _cluster_volumes(numNodes),
          non_sampling_incident_cluster_weights(numNodes),
          _disable_randomization(disable_randomization),
          prng(context.partition.seed),
          volume_updates_to(0),
          volume_updates_from(0)
  { }

  ~ParallelLocalMovingModularity();

  bool localMoving(Graph& graph, ds::Clustering& communities);

 private:
  size_t parallelNonDeterministicRound(const Graph& graph, ds::Clustering& communities);
  size_t synchronousParallelRound(const Graph& graph, ds::Clustering& communities);
  size_t sequentialRound(const Graph& graph, ds::Clustering& communities);

  struct ClearList {
    vec<double> weights;
    vec<PartitionID> used;
    ClearList(size_t n) : weights(n) { }
  };


  MT_KAHYPAR_ATTRIBUTE_ALWAYS_INLINE bool ratingsFitIntoSmallSparseMap(const Graph& graph,
                                                                       const HypernodeID u)  {
    static constexpr size_t cache_efficient_map_size = CacheEfficientIncidentClusterWeights::MAP_SIZE / 3UL;
    return std::min(_vertex_degree_sampling_threshold, _max_degree) > cache_efficient_map_size &&
           graph.degree(u) <= cache_efficient_map_size;
  }

  LargeIncidentClusterWeights construct_large_incident_cluster_weight_map() {
    return LargeIncidentClusterWeights(3UL * std::min(_max_degree, _vertex_degree_sampling_threshold), 0);
  }

  // ! Only for testing
  void initializeClusterVolumes(const Graph& graph, ds::Clustering& communities);

  MT_KAHYPAR_ATTRIBUTE_ALWAYS_INLINE PartitionID computeMaxGainCluster(const Graph& graph,
                                                                       const ds::Clustering& communities,
                                                                       const NodeID u) {
    return computeMaxGainCluster(graph, communities, u, non_sampling_incident_cluster_weights.local());
  }

  MT_KAHYPAR_ATTRIBUTE_ALWAYS_INLINE PartitionID computeMaxGainCluster(const Graph& graph,
                                                                       const ds::Clustering& communities,
                                                                       const NodeID u,
                                                                       ClearList& incident_cluster_weights) {
    const PartitionID from = communities[u];
    PartitionID bestCluster = communities[u];

    auto& weights = incident_cluster_weights.weights;
    auto& used = incident_cluster_weights.used;

    for (const Arc& arc : graph.arcsOf(u, _vertex_degree_sampling_threshold)) {
      const auto cv = communities[arc.head];
      if (weights[cv] == 0.0) used.push_back(cv);
      weights[cv] += arc.weight;
    }

    const ArcWeight volume_from = _cluster_volumes[from].load(std::memory_order_relaxed);
    const ArcWeight volU = graph.nodeVolume(u);
    const ArcWeight weight_from = weights[from];

    const double volMultiplier = _vol_multiplier_div_by_node_vol * volU;
    double bestGain = weight_from - volMultiplier * (volume_from - volU);
    double best_weight_to = weight_from;
    for (const auto to : used) {
      // if from == to, we would have to remove volU from volume_to as well.
      // just skip it. it has (adjusted) gain zero.
      if (from != to) {
        double gain = modularityGain(weights[to], _cluster_volumes[to].load(std::memory_order_relaxed), volMultiplier);
        if (gain > bestGain) {
          bestCluster = to;
          bestGain = gain;
          best_weight_to = weights[to];
        }
      }
      weights[to] = 0.0;
    }
    used.clear();

    // changing communities and volumes in parallel causes non-determinism in debug mode

    unused(best_weight_to);
    HEAVY_PREPROCESSING_ASSERT(verifyGain(graph, communities, u, bestCluster, bestGain, weight_from, best_weight_to));

    return bestCluster;
  }


  inline double modularityGain(const ArcWeight weight_to,
                               const ArcWeight volume_to,
                               const double multiplier) {
    return weight_to - multiplier * volume_to;
    // missing term is - weight_from + multiplier * (volume_from - volume_node)
  }

  inline long double adjustAdvancedModGain(double gain,
                                           const ArcWeight weight_from,
                                           const ArcWeight volume_from,
                                           const ArcWeight volume_node) const {
    return 2.0L * _reciprocal_total_volume *
      (gain - weight_from + _reciprocal_total_volume *
        volume_node * (volume_from - volume_node));
  }


  bool verifyGain(const Graph& graph, const ds::Clustering& communities, NodeID u, PartitionID to, double gain,
                  double weight_from, double weight_to);

  static std::pair<ArcWeight, ArcWeight> intraClusterWeightsAndSumOfSquaredClusterVolumes(const Graph& graph, const ds::Clustering& communities);

  const Context& _context;
  size_t _max_degree;
  const size_t _vertex_degree_sampling_threshold;
  double _reciprocal_total_volume = 0.0;
  double _vol_multiplier_div_by_node_vol = 0.0;
  vec<parallel::AtomicWrapper<ArcWeight>> _cluster_volumes;
  tbb::enumerable_thread_specific<ClearList> non_sampling_incident_cluster_weights;
  const bool _disable_randomization;

  utils::ParallelPermutation<HypernodeID> permutation;
  std::mt19937 prng;

  struct ClusterMove {
    PartitionID cluster;
    NodeID node;
    bool operator< (const ClusterMove& o) const {
      return std::tie(cluster, node) < std::tie(o.cluster, o.node);
    }
  };
  ds::BufferedVector<ClusterMove> volume_updates_to, volume_updates_from;



  FRIEND_TEST(ALouvain, ComputesMaxGainMove1);
  FRIEND_TEST(ALouvain, ComputesMaxGainMove2);
  FRIEND_TEST(ALouvain, ComputesMaxGainMove3);
  FRIEND_TEST(ALouvain, ComputesMaxGainMove4);
  FRIEND_TEST(ALouvain, ComputesMaxGainMove5);
  FRIEND_TEST(ALouvain, ComputesMaxGainMove6);
  FRIEND_TEST(ALouvain, ComputesMaxGainMove7);
  FRIEND_TEST(ALouvain, ComputesMaxGainMove8);
  FRIEND_TEST(ALouvain, ComputesMaxGainMove9);
  FRIEND_TEST(ALouvain, ComputesMaxGainMove10);
};
}
