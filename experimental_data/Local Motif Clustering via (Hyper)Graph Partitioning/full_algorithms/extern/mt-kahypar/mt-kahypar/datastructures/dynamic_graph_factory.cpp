/*******************************************************************************
 * This file is part of Mt-KaHyPar.
 *
 * Copyright (C) 2019 Lars Gottesbüren <lars.gottesbueren@kit.edu>
 * Copyright (C) 2019 Tobias Heuer <tobias.heuer@kit.edu>
 * Copyright (C) 2022 Nikolai Maas <nikolai.maas@student.kit.edu>
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

#include "dynamic_graph_factory.h"

#include <tbb/parallel_for.h>
#include <tbb/parallel_invoke.h>

#include "mt-kahypar/parallel/parallel_prefix_sum.h"
#include "mt-kahypar/parallel/stl/scalable_vector.h"
#include "mt-kahypar/utils/timer.h"

namespace mt_kahypar::ds {

DynamicGraph DynamicGraphFactory::construct(
        const HypernodeID num_nodes,
        const HyperedgeID num_edges,
        const HyperedgeVector& edge_vector,
        const HyperedgeWeight* edge_weight,
        const HypernodeWeight* node_weight,
        const bool stable_construction_of_incident_edges) {
  ASSERT(edge_vector.size() == num_edges);

  EdgeVector edges;
  edges.reserve(num_edges);
  for (const auto& e : edge_vector) {
    if (e.size() != 2) {
      ERROR("Using graph data structure; but the input hypergraph is not a graph.");
    }
    edges.push_back({e[0], e[1]});
  }
  return construct_from_graph_edges(num_nodes, num_edges, edges,
                                    edge_weight, node_weight,
                                    stable_construction_of_incident_edges);
}

DynamicGraph DynamicGraphFactory::construct_from_graph_edges(
        const HypernodeID num_nodes,
        const HyperedgeID num_edges,
        const EdgeVector& edge_vector,
        const HyperedgeWeight* edge_weight,
        const HypernodeWeight* node_weight,
        const bool stable_construction_of_incident_edges) {
  ASSERT(edge_vector.size() == num_edges);
  DynamicGraph graph;
  graph._num_edges = 2 * num_edges;

  // TODO: calculate required id range
  tbb::parallel_invoke([&] {
    graph._nodes.resize(num_nodes + 1);
    tbb::parallel_for(ID(0), num_nodes, [&](const HypernodeID n) {
      // setup nodes
      DynamicGraph::Node& node = graph._nodes[n];
      node.enable();
      if ( node_weight ) {
        node.setWeight(node_weight[n]);
      }
    });
    // Compute total weight of graph
    graph.updateTotalWeight(parallel_tag_t());
  }, [&] {
    graph._adjacency_array = DynamicAdjacencyArray(num_nodes, edge_vector, edge_weight);
    if (stable_construction_of_incident_edges) {
      graph._adjacency_array.sortIncidentEdges();
    }
  }, [&] {
    graph._acquired_nodes.assign(
      num_nodes, parallel::IntegralAtomicWrapper<bool>(false));
  }, [&] {
    graph._contraction_tree.initialize(num_nodes);
  });

  // Compute total weight of the graph
  graph.updateTotalWeight(parallel_tag_t());
  return graph;
}


std::pair<DynamicGraph, parallel::scalable_vector<HypernodeID> > DynamicGraphFactory::compactify(const DynamicGraph& graph) {
  HypernodeID num_nodes = 0;
  HyperedgeID num_edges = 0;
  parallel::scalable_vector<HypernodeID> hn_mapping;
  parallel::scalable_vector<HyperedgeID> he_mapping;
  // Computes a mapping for vertices and edges to a consecutive range of IDs
  // in the compactified hypergraph via a parallel prefix sum
  tbb::parallel_invoke([&] {
    hn_mapping.assign(graph.numNodes() + 1, 0);
    graph.doParallelForAllNodes([&](const HypernodeID hn) {
      hn_mapping[hn + 1] = ID(1);
    });

    parallel::TBBPrefixSum<HypernodeID, parallel::scalable_vector> hn_mapping_prefix_sum(hn_mapping);
    tbb::parallel_scan(tbb::blocked_range<size_t>(
      0UL, graph.numNodes() + 1), hn_mapping_prefix_sum);
    num_nodes = hn_mapping_prefix_sum.total_sum();
  }, [&] {
    he_mapping.assign(graph._num_edges + 1, 0);
    graph.doParallelForAllEdges([&](const HyperedgeID& he) {
      if (graph.edgeSource(he) < graph.edgeTarget(he)) {
        he_mapping[he + 1] = ID(1);
      }
    });

    parallel::TBBPrefixSum<HyperedgeID, parallel::scalable_vector> he_mapping_prefix_sum(he_mapping);
    tbb::parallel_scan(tbb::blocked_range<size_t>(
      0UL, graph._num_edges + 1), he_mapping_prefix_sum);
    num_edges = he_mapping_prefix_sum.total_sum();
  });

  // Remap pins of each hyperedge
  parallel::scalable_vector<std::pair<HypernodeID, HypernodeID>> edge_vector;
  parallel::scalable_vector<HyperedgeWeight> edge_weights;
  parallel::scalable_vector<HypernodeWeight> node_weights;
  tbb::parallel_invoke([&] {
    node_weights.resize(num_nodes);
    graph.doParallelForAllNodes([&](const HypernodeID hn) {
      const HypernodeID mapped_hn = hn_mapping[hn];
      ASSERT(mapped_hn < num_nodes);
      node_weights[mapped_hn] = graph.nodeWeight(hn);
    });
  }, [&] {
    edge_vector.resize(num_edges);
    edge_weights.resize(num_edges);
    graph.doParallelForAllEdges([&](const HyperedgeID he) {
      if (graph.edgeSource(he) < graph.edgeTarget(he)) {
        const HyperedgeID mapped_he = he_mapping[he];
        ASSERT(mapped_he < num_edges);
        edge_weights[mapped_he] = graph.edgeWeight(he);
        edge_vector[mapped_he] = {hn_mapping[graph.edgeSource(he)], hn_mapping[graph.edgeTarget(he)]};
      }
    });
  });

  // Construct compactified graph
  DynamicGraph compactified_graph = DynamicGraphFactory::construct_from_graph_edges(
    num_nodes, num_edges, edge_vector, edge_weights.data(), node_weights.data());
  compactified_graph._removed_degree_zero_hn_weight = graph._removed_degree_zero_hn_weight;
  compactified_graph._total_weight += graph._removed_degree_zero_hn_weight;

  // Set community ids
  graph.doParallelForAllNodes([&](const HypernodeID& hn) {
    const HypernodeID mapped_hn = hn_mapping[hn];
    compactified_graph.setCommunityID(mapped_hn, graph.communityID(hn));
  });

  parallel::parallel_free(he_mapping, edge_weights, node_weights, edge_vector);

  return std::make_pair(std::move(compactified_graph), std::move(hn_mapping));
}

}
