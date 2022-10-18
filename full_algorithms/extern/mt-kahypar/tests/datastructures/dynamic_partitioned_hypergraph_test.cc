/*******************************************************************************
 * This file is part of Mt-KaHyPar.
 *
 * Copyright (C) 2019 Tobias Heuer <tobias.heuer@kit.edu>
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


#include <atomic>
#include <mt-kahypar/parallel/tbb_initializer.h>

#include "gmock/gmock.h"

#include "mt-kahypar/datastructures/static_hypergraph_factory.h"
#include "mt-kahypar/datastructures/dynamic_hypergraph_factory.h"
#include "mt-kahypar/datastructures/partitioned_hypergraph.h"

using ::testing::Test;

namespace mt_kahypar {
namespace ds {

template< typename PartitionedHG,
          typename HG,
          typename HGFactory>
struct PartitionedHypergraphTypeTraits {
  using PartitionedHyperGraph = PartitionedHG;
  using Hypergraph = HG;
  using Factory = HGFactory;
};

template<typename TypeTraits>
class APartitionedHypergraph : public Test {

 using PartitionedHyperGraph = typename TypeTraits::PartitionedHyperGraph;
 using Factory = typename TypeTraits::Factory;

 public:
 using Hypergraph = typename TypeTraits::Hypergraph;

  APartitionedHypergraph() :
    hypergraph(Factory::construct(
      7 , 4, { {0, 2}, {0, 1, 3, 4}, {3, 4, 6}, {2, 5, 6} })),
    partitioned_hypergraph(3, hypergraph, parallel_tag_t()) {
    initializePartition();
  }

  void initializePartition() {
    if ( hypergraph.nodeIsEnabled(0) ) partitioned_hypergraph.setNodePart(0, 0);
    if ( hypergraph.nodeIsEnabled(1) ) partitioned_hypergraph.setNodePart(1, 0);
    if ( hypergraph.nodeIsEnabled(2) ) partitioned_hypergraph.setNodePart(2, 0);
    if ( hypergraph.nodeIsEnabled(3) ) partitioned_hypergraph.setNodePart(3, 1);
    if ( hypergraph.nodeIsEnabled(4) ) partitioned_hypergraph.setNodePart(4, 1);
    if ( hypergraph.nodeIsEnabled(5) ) partitioned_hypergraph.setNodePart(5, 2);
    if ( hypergraph.nodeIsEnabled(6) ) partitioned_hypergraph.setNodePart(6, 2);
  }

  void verifyPartitionPinCounts(const HyperedgeID he,
                                const std::vector<HypernodeID>& expected_pin_counts) {
    ASSERT(expected_pin_counts.size() == static_cast<size_t>(partitioned_hypergraph.k()));
    for (PartitionID block = 0; block < 3; ++block) {
      ASSERT_EQ(expected_pin_counts[block], partitioned_hypergraph.pinCountInPart(he, block)) << V(he) << V(block);
    }
  }

  void verifyConnectivitySet(const HyperedgeID he,
                             const std::set<PartitionID>& connectivity_set) {
    ASSERT_EQ(connectivity_set.size(), partitioned_hypergraph.connectivity(he)) << V(he);
    PartitionID connectivity = 0;
    for (const PartitionID& block : partitioned_hypergraph.connectivitySet(he)) {
      ASSERT_TRUE(connectivity_set.find(block) != connectivity_set.end()) << V(he) << V(block);
      ++connectivity;
    }
    ASSERT_EQ(connectivity_set.size(), connectivity) << V(he);
  }

  void verifyPins(const Hypergraph& hg,
                  const std::vector<HyperedgeID> hyperedges,
                  const std::vector< std::set<HypernodeID> >& references,
                  bool log = false) {
    ASSERT(hyperedges.size() == references.size());
    for (size_t i = 0; i < hyperedges.size(); ++i) {
      const HyperedgeID he = hyperedges[i];
      const std::set<HypernodeID>& reference = references[i];
      size_t count = 0;
      for (const HypernodeID& pin : hg.pins(he)) {
        if (log) LOG << V(he) << V(pin);
        ASSERT_TRUE(reference.find(pin) != reference.end()) << V(he) << V(pin);
        count++;
      }
      ASSERT_EQ(count, reference.size());
    }
  }

  HyperedgeWeight compute_km1() {
    HyperedgeWeight km1 = 0;
    for (const HyperedgeID& he : partitioned_hypergraph.edges()) {
      km1 += std::max(partitioned_hypergraph.connectivity(he) - 1, 0) * partitioned_hypergraph.edgeWeight(he);
    }
    return km1;
  }

  void verifyAllKm1GainValues() {
    for ( const HypernodeID hn : hypergraph.nodes() ) {
      const PartitionID from = partitioned_hypergraph.partID(hn);
      for ( PartitionID to = 0; to < partitioned_hypergraph.k(); ++to ) {
        if ( from != to ) {
          const HyperedgeWeight km1_before = compute_km1();
          const HyperedgeWeight km1_gain = partitioned_hypergraph.km1Gain(hn, from, to);
          partitioned_hypergraph.changeNodePart(hn, from, to);
          const HyperedgeWeight km1_after = compute_km1();
          ASSERT_EQ(km1_gain, km1_before - km1_after);
          partitioned_hypergraph.changeNodePart(hn, to, from);
        }
      }
    }
  }

  Hypergraph hypergraph;
  PartitionedHyperGraph partitioned_hypergraph;
};

template <class F1, class F2>
void executeConcurrent(const F1& f1, const F2& f2) {
  std::atomic<int> cnt(0);
  tbb::parallel_invoke([&] {
    cnt++;
    while (cnt < 2) { }
    f1();
  }, [&] {
    cnt++;
    while (cnt < 2) { }
    f2();
  });
}


using ADynamicPartitionedHypergraph = APartitionedHypergraph<
                                        PartitionedHypergraphTypeTraits<
                                          PartitionedHypergraph<DynamicHypergraph, DynamicHypergraphFactory>,
                                          DynamicHypergraph,
                                          DynamicHypergraphFactory>>;

TEST_F(ADynamicPartitionedHypergraph, InitializesGainCorrectIfAlreadyContracted1) {
  hypergraph.registerContraction(0, 2);
  hypergraph.contract(2);
  hypergraph.removeSinglePinAndParallelHyperedges();

    partitioned_hypergraph.initializeGainCache();
  verifyAllKm1GainValues();
}

TEST_F(ADynamicPartitionedHypergraph, InitializesGainCorrectIfAlreadyContracted2) {
  partitioned_hypergraph.resetPartition();
  hypergraph.registerContraction(1, 2);
  hypergraph.registerContraction(0, 1);
  hypergraph.registerContraction(4, 5);
  hypergraph.registerContraction(3, 4);
  hypergraph.registerContraction(6, 3);
  hypergraph.contract(2);
  hypergraph.contract(5);
  hypergraph.removeSinglePinAndParallelHyperedges();
  ASSERT_FALSE(hypergraph.edgeIsEnabled(0));
  ASSERT_EQ(2, hypergraph.edgeWeight(1));
  ASSERT_FALSE(hypergraph.edgeIsEnabled(2));
  ASSERT_FALSE(hypergraph.edgeIsEnabled(3));

  initializePartition();
    partitioned_hypergraph.initializeGainCache();
  verifyAllKm1GainValues();
}

TEST_F(ADynamicPartitionedHypergraph, ComputesPinCountsCorrectlyIfWeRestoreSinglePinAndParallelNets1) {
  partitioned_hypergraph.resetPartition();
  hypergraph.registerContraction(0, 2);
  hypergraph.contract(2);
  auto removed_hyperedges = hypergraph.removeSinglePinAndParallelHyperedges();
  ASSERT_EQ(1, removed_hyperedges.size());
  ASSERT_EQ(0, removed_hyperedges[0].removed_hyperedge);

  initializePartition();
  partitioned_hypergraph.restoreSinglePinAndParallelNets(removed_hyperedges);
  verifyPartitionPinCounts(0, { 1, 0, 0 });
}

TEST_F(ADynamicPartitionedHypergraph, ComputesPinCountsCorrectlyIfWeRestoreSinglePinAndParallelNets2) {
  partitioned_hypergraph.resetPartition();
  hypergraph.registerContraction(1, 2);
  hypergraph.registerContraction(0, 1);
  hypergraph.registerContraction(4, 5);
  hypergraph.registerContraction(3, 4);
  hypergraph.registerContraction(6, 3);
  hypergraph.contract(2);
  hypergraph.contract(5);
  auto removed_hyperedges = hypergraph.removeSinglePinAndParallelHyperedges();

  initializePartition();
  partitioned_hypergraph.restoreSinglePinAndParallelNets(removed_hyperedges);
  verifyPartitionPinCounts(0, { 1, 0, 0 });
  verifyPartitionPinCounts(1, { 1, 0, 1 });
  verifyPartitionPinCounts(2, { 0, 0, 1 });
  verifyPartitionPinCounts(3, { 1, 0, 1 });
}

TEST_F(ADynamicPartitionedHypergraph, UpdatesGainCacheCorrectlyIfWeRestoreSinglePinAndParallelNets1) {
  partitioned_hypergraph.resetPartition();
  hypergraph.registerContraction(0, 2);
  hypergraph.contract(2);
  auto removed_hyperedges = hypergraph.removeSinglePinAndParallelHyperedges();

  initializePartition();
    partitioned_hypergraph.initializeGainCache();
  ASSERT_EQ(1, partitioned_hypergraph.moveFromBenefit(0));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(0, 1));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(0, 2));
  partitioned_hypergraph.restoreSinglePinAndParallelNets(removed_hyperedges);
  ASSERT_EQ(2, partitioned_hypergraph.moveFromBenefit(0));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(0, 1));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(0, 2));
  verifyAllKm1GainValues();
}

TEST_F(ADynamicPartitionedHypergraph, UpdatesGainCacheCorrectlyIfWeRestoreSinglePinAndParallelNets2) {
  partitioned_hypergraph.resetPartition();
  hypergraph.registerContraction(1, 2);
  hypergraph.registerContraction(0, 1);
  hypergraph.registerContraction(4, 5);
  hypergraph.registerContraction(3, 4);
  hypergraph.registerContraction(6, 3);
  hypergraph.contract(2);
  hypergraph.contract(5);
  auto removed_hyperedges = hypergraph.removeSinglePinAndParallelHyperedges();

  initializePartition();
    partitioned_hypergraph.initializeGainCache();
  ASSERT_EQ(2, partitioned_hypergraph.moveFromBenefit(0));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(0, 1));
  ASSERT_EQ(0, partitioned_hypergraph.moveToPenalty(0, 2));
  ASSERT_EQ(2, partitioned_hypergraph.moveFromBenefit(6));
  ASSERT_EQ(0, partitioned_hypergraph.moveToPenalty(6, 0));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(6, 1));
  partitioned_hypergraph.restoreSinglePinAndParallelNets(removed_hyperedges);
  ASSERT_EQ(3, partitioned_hypergraph.moveFromBenefit(0));
  ASSERT_EQ(3, partitioned_hypergraph.moveToPenalty(0, 1));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(0, 2));
  ASSERT_EQ(3, partitioned_hypergraph.moveFromBenefit(6));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(6, 0));
  ASSERT_EQ(3, partitioned_hypergraph.moveToPenalty(6, 1));
  verifyAllKm1GainValues();
}

TEST_F(ADynamicPartitionedHypergraph, ComputesCorrectPinCountsAfterUncontraction1) {
  partitioned_hypergraph.resetPartition();
  hypergraph.registerContraction(0, 2);
  hypergraph.contract(2);

  VersionedBatchVector hierarchy = hypergraph.createBatchUncontractionHierarchy(2);

  initializePartition();
  partitioned_hypergraph.uncontract(hierarchy.back().back());
  ASSERT_EQ(0, partitioned_hypergraph.partID(2));
  verifyPartitionPinCounts(0, { 2, 0, 0 });
  verifyPartitionPinCounts(1, { 2, 2, 0 });
  verifyPartitionPinCounts(2, { 0, 2, 1 });
  verifyPartitionPinCounts(3, { 1, 0, 2 });
}

TEST_F(ADynamicPartitionedHypergraph, ComputesCorrectPinCountsAfterUncontraction2) {
  partitioned_hypergraph.resetPartition();
  hypergraph.registerContraction(1, 2);
  hypergraph.registerContraction(0, 1);
  hypergraph.registerContraction(4, 5);
  hypergraph.registerContraction(3, 4);
  hypergraph.registerContraction(6, 3);
  hypergraph.contract(2);
  hypergraph.contract(5);

  VersionedBatchVector hierarchy = hypergraph.createBatchUncontractionHierarchy(2);

  initializePartition();

  while ( !hierarchy.empty() ) {
    BatchVector& batches = hierarchy.back();
    while ( !batches.empty() ) {
      const Batch& batch = batches.back();
      partitioned_hypergraph.uncontract(batch);
      batches.pop_back();
    }
    hierarchy.pop_back();
  }

  ASSERT_EQ(0, partitioned_hypergraph.partID(0));
  ASSERT_EQ(0, partitioned_hypergraph.partID(1));
  ASSERT_EQ(0, partitioned_hypergraph.partID(2));
  ASSERT_EQ(2, partitioned_hypergraph.partID(3));
  ASSERT_EQ(2, partitioned_hypergraph.partID(4));
  ASSERT_EQ(2, partitioned_hypergraph.partID(5));
  ASSERT_EQ(2, partitioned_hypergraph.partID(6));
  verifyPartitionPinCounts(0, { 2, 0, 0 });
  verifyPartitionPinCounts(1, { 2, 0, 2 });
  verifyPartitionPinCounts(2, { 0, 0, 3 });
  verifyPartitionPinCounts(3, { 1, 0, 2 });
}

TEST_F(ADynamicPartitionedHypergraph, UpdatesGainCacheCorrectlyAfterUncontraction1) {
  partitioned_hypergraph.resetPartition();
  hypergraph.registerContraction(0, 2);
  hypergraph.contract(2);

  VersionedBatchVector hierarchy = hypergraph.createBatchUncontractionHierarchy(2);

  initializePartition();
    partitioned_hypergraph.initializeGainCache();
  partitioned_hypergraph.uncontract(hierarchy.back().back());
  ASSERT_EQ(0, partitioned_hypergraph.moveFromBenefit(0));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(0, 1));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(0, 2));
  ASSERT_EQ(1, partitioned_hypergraph.moveFromBenefit(2));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(2, 1));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(2, 2));
  verifyAllKm1GainValues();
}

TEST_F(ADynamicPartitionedHypergraph, UpdatesGainCacheCorrectlyAfterUncontraction2) {
  partitioned_hypergraph.resetPartition();
  hypergraph.registerContraction(1, 2);
  hypergraph.registerContraction(0, 1);
  hypergraph.registerContraction(4, 5);
  hypergraph.registerContraction(3, 4);
  hypergraph.registerContraction(6, 3);
  hypergraph.contract(2);
  hypergraph.contract(5);

  VersionedBatchVector hierarchy = hypergraph.createBatchUncontractionHierarchy(2);

  initializePartition();
    partitioned_hypergraph.initializeGainCache();

  while ( !hierarchy.empty() ) {
    BatchVector& batches = hierarchy.back();
    while ( !batches.empty() ) {
      const Batch& batch = batches.back();
      partitioned_hypergraph.uncontract(batch);
      batches.pop_back();
    }
    hierarchy.pop_back();
  }

  ASSERT_EQ(0, partitioned_hypergraph.moveFromBenefit(0));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(0, 1));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(0, 2));
  ASSERT_EQ(0, partitioned_hypergraph.moveFromBenefit(1));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(1, 1));
  ASSERT_EQ(0, partitioned_hypergraph.moveToPenalty(1, 2));
  ASSERT_EQ(1, partitioned_hypergraph.moveFromBenefit(2));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(2, 1));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(2, 2));
  ASSERT_EQ(0, partitioned_hypergraph.moveFromBenefit(3));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(3, 0));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(3, 1));
  ASSERT_EQ(0, partitioned_hypergraph.moveFromBenefit(4));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(4, 0));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(4, 1));
  ASSERT_EQ(0, partitioned_hypergraph.moveFromBenefit(5));
  ASSERT_EQ(0, partitioned_hypergraph.moveToPenalty(5, 0));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(5, 1));
  ASSERT_EQ(0, partitioned_hypergraph.moveFromBenefit(6));
  ASSERT_EQ(1, partitioned_hypergraph.moveToPenalty(6, 0));
  ASSERT_EQ(2, partitioned_hypergraph.moveToPenalty(6, 1));
  verifyAllKm1GainValues();
}


}  // namespace ds
}  // namespace mt_kahypar