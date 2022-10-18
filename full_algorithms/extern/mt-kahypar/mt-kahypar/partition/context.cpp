/*******************************************************************************
 * This file is part of Mt-KaHyPar.
 *
 * Copyright (C) 2019 Lars Gottesbüren <lars.gottesbueren@kit.edu>
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

#include "context.h"

namespace mt_kahypar {

  std::ostream & operator<< (std::ostream& str, const PartitioningParameters& params) {
    str << "Partitioning Parameters:" << std::endl;
    str << "  Hypergraph:                         " << params.graph_filename << std::endl;
    if ( params.write_partition_file ) {
      str << "  Partition File:                     " << params.graph_partition_filename << std::endl;
    }
    str << "  Paradigm:                           " << params.paradigm << std::endl;
    str << "  Mode:                               " << params.mode << std::endl;
    str << "  Objective:                          " << params.objective << std::endl;
    str << "  Input File Format:                  " << params.file_format << std::endl;
    if ( params.instance_type != InstanceType::UNDEFINED ) {
      str << "  Instance Type:                      " << params.instance_type << std::endl;
    }
    if ( params.preset_type != PresetType::UNDEFINED ) {
      str << "  Preset Type:                        " << params.preset_type << std::endl;
    }
    str << "  k:                                  " << params.k << std::endl;
    str << "  epsilon:                            " << params.epsilon << std::endl;
    str << "  seed:                               " << params.seed << std::endl;
    str << "  Number of V-Cycles:                 " << params.num_vcycles << std::endl;
    str << "  Ignore HE Size Threshold:           " << params.ignore_hyperedge_size_threshold << std::endl;
    str << "  Large HE Size Threshold:            " << params.large_hyperedge_size_threshold << std::endl;
    if ( params.use_individual_part_weights ) {
      str << "  Individual Part Weights:            ";
      for ( const HypernodeWeight& w : params.max_part_weights ) {
        str << w << " ";
      }
      str << std::endl;
    }
    return str;
  }

  std::ostream & operator<< (std::ostream& str, const CommunityDetectionParameters& params) {
    str << "  Community Detection Parameters:" << std::endl;
    str << "    Edge Weight Function:                " << params.edge_weight_function << std::endl;
    str << "    Maximum Louvain-Pass Iterations:     " << params.max_pass_iterations << std::endl;
    str << "    Minimum Vertex Move Fraction:        " << params.min_vertex_move_fraction << std::endl;
    str << "    Vertex Degree Sampling Threshold:    " << params.vertex_degree_sampling_threshold << std::endl;
    str << "    Number of subrounds (deterministic): " << params.num_sub_rounds_deterministic << std::endl;
    return str;
  }

  std::ostream & operator<< (std::ostream& str, const PreprocessingParameters& params) {
    str << "Preprocessing Parameters:" << std::endl;
    str << "  Use Community Detection:            " << std::boolalpha << params.use_community_detection << std::endl;
    #ifdef USE_GRAPH_PARTITIONER
    str << "  Disable C. D. for Mesh Graphs:      " << std::boolalpha << params.disable_community_detection_for_mesh_graphs << std::endl;
    #endif
    if (params.use_community_detection) {
      str << std::endl << params.community_detection;
    }
    return str;
  }

  std::ostream & operator<< (std::ostream& str, const RatingParameters& params) {
    str << "  Rating Parameters:" << std::endl;
    str << "    Rating Function:                  " << params.rating_function << std::endl;
    str << "    Heavy Node Penalty:               " << params.heavy_node_penalty_policy << std::endl;
    str << "    Acceptance Policy:                " << params.acceptance_policy << std::endl;
    return str;
  }

  std::ostream & operator<< (std::ostream& str, const CoarseningParameters& params) {
    str << "Coarsening Parameters:" << std::endl;
    str << "  Algorithm:                          " << params.algorithm << std::endl;
    str << "  Use Adaptive Edge Size:             " << std::boolalpha << params.use_adaptive_edge_size << std::endl;
#ifdef KAHYPAR_ENABLE_EXPERIMENTAL_FEATURES
    str << "  Use Adaptive Max Node Weight:       " << std::boolalpha << params.use_adaptive_max_allowed_node_weight << std::endl;
#endif
    if ( params.use_adaptive_max_allowed_node_weight ) {
      str << "  Max Allowed Weight Fraction:        " << params.max_allowed_weight_fraction << std::endl;
      str << "  Adaptive Node Weight Threshold:     " << params.adaptive_node_weight_shrink_factor_threshold << std::endl;
      str << "  Initial Max Hypernode Weight:       " << params.max_allowed_node_weight << std::endl;
    } else {
      str << "  Max Allowed Weight Multiplier:      " << params.max_allowed_weight_multiplier << std::endl;
      str << "  Maximum Allowed Hypernode Weight:   " << params.max_allowed_node_weight << std::endl;
    }
    str << "  Contraction Limit Multiplier:       " << params.contraction_limit_multiplier << std::endl;
    str << "  Contraction Limit:                  " << params.contraction_limit << std::endl;
    str << "  Minimum Shrink Factor:              " << params.minimum_shrink_factor << std::endl;
    str << "  Maximum Shrink Factor:              " << params.maximum_shrink_factor << std::endl;
    str << "  Vertex Degree Sampling Threshold:   " << params.vertex_degree_sampling_threshold << std::endl;
    str << "  Number of subrounds (deterministic):" << params.num_sub_rounds_deterministic << std::endl;
    str << std::endl << params.rating;
    return str;
  }

  std::ostream & operator<< (std::ostream& str, const LabelPropagationParameters& params) {
    str << "  Label Propagation Parameters:" << std::endl;
    str << "    Algorithm:                        " << params.algorithm << std::endl;
    if ( params.algorithm != LabelPropagationAlgorithm::do_nothing ) {
      str << "    Maximum Iterations:               " << params.maximum_iterations << std::endl;
      str << "    Rebalancing:                      " << std::boolalpha << params.rebalancing << std::endl;
      str << "    HE Size Activation Threshold:     " << std::boolalpha << params.hyperedge_size_activation_threshold << std::endl;
    }
    return str;
  }

  std::ostream& operator<<(std::ostream& out, const FMParameters& params) {
    out << "  FM Parameters: \n";
    out << "    Algorithm:                        " << params.algorithm << std::endl;
    if ( params.algorithm != FMAlgorithm::do_nothing ) {
      out << "    Multitry Rounds:                  " << params.multitry_rounds << std::endl;
      out << "    Perform Moves Globally:           " << std::boolalpha << params.perform_moves_global << std::endl;
      out << "    Parallel Global Rollbacks:        " << std::boolalpha << params.rollback_parallel << std::endl;
      out << "    Rollback Bal. Violation Factor:   " << params.rollback_balance_violation_factor << std::endl;
      out << "    Num Seed Nodes:                   " << params.num_seed_nodes << std::endl;
      out << "    Enable Random Shuffle:            " << std::boolalpha << params.shuffle << std::endl;
      out << "    Obey Minimal Parallelism:         " << std::boolalpha << params.obey_minimal_parallelism << std::endl;
      out << "    Minimum Improvement Factor:       " << params.min_improvement << std::endl;
      out << "    Release Nodes:                    " << std::boolalpha << params.release_nodes << std::endl;
      out << "    Time Limit Factor:                " << params.time_limit_factor << std::endl;
    }
    out << std::flush;
    return out;
  }

  std::ostream& operator<<(std::ostream& out, const NLevelGlobalFMParameters& params) {
    if ( params.use_global_fm ) {
      out << "  Boundary FM Parameters: \n";
      out << "    Refine Until No Improvement:      " << std::boolalpha << params.refine_until_no_improvement << std::endl;
      out << "    Num Seed Nodes:                   " << params.num_seed_nodes << std::endl;
      out << "    Obey Minimal Parallelism:         " << std::boolalpha << params.obey_minimal_parallelism << std::endl;
    }
    return out;
  }

  std::ostream& operator<<(std::ostream& out, const FlowParameters& params) {
    out << "  Flow Parameters: \n";
    out << "    Algorithm:                        " << params.algorithm << std::endl;
    if ( params.algorithm != FlowAlgorithm::do_nothing ) {
      out << "    Flow Scaling:                     " << params.alpha << std::endl;
      out << "    Maximum Number of Pins:           " << params.max_num_pins << std::endl;
      out << "    Find Most Balanced Cut:           " << std::boolalpha << params.find_most_balanced_cut << std::endl;
      out << "    Determine Distance From Cut:      " << std::boolalpha << params.determine_distance_from_cut << std::endl;
      out << "    Parallel Searches Multiplier:     " << params.parallel_searches_multiplier << std::endl;
      out << "    Number of Parallel Searches:      " << params.num_parallel_searches << std::endl;
      out << "    Maximum BFS Distance:             " << params.max_bfs_distance << std::endl;
      out << "    Min Rel. Improvement Per Round:   " << params.min_relative_improvement_per_round << std::endl;
      out << "    Time Limit Factor:                " << params.time_limit_factor << std::endl;
      out << "    Skip Small Cuts:                  " << std::boolalpha << params.skip_small_cuts << std::endl;
      out << "    Skip Unpromising Blocks:          " << std::boolalpha << params.skip_unpromising_blocks << std::endl;
      out << "    Pierce in Bulk:                   " << std::boolalpha << params.pierce_in_bulk << std::endl;
      out << std::flush;
    }
    return out;
  }

  std::ostream& operator<<(std::ostream& out, const DeterministicRefinementParameters& params) {
    out << "    Number of sub-rounds for Sync LP:  " << params.num_sub_rounds_sync_lp << std::endl;
    out << "    Use active node set:               " << std::boolalpha << params.use_active_node_set << std::endl;
    out << "    recalculate gains on second apply: " << std::boolalpha
        << params.recalculate_gains_on_second_apply << std::endl;
    return out;
  }

  std::ostream & operator<< (std::ostream& str, const RefinementParameters& params) {
    str << "Refinement Parameters:" << std::endl;
    str << "  Refine Until No Improvement:        " << std::boolalpha << params.refine_until_no_improvement << std::endl;
    str << "  Relative Improvement Threshold:     " << params.relative_improvement_threshold << std::endl;
#ifdef USE_STRONG_PARTITIONER
    str << "  Maximum Batch Size:                 " << params.max_batch_size << std::endl;
    str << "  Min Border Vertices Per Thread:     " << params.min_border_vertices_per_thread << std::endl;
#endif
    str << "\n" << params.label_propagation;
    str << "\n" << params.fm;
#ifdef USE_STRONG_PARTITIONER
    str << "\n" << params.global_fm;
#endif
    str << "\n" << params.flows;
    return str;
  }

  std::ostream & operator<< (std::ostream& str, const SparsificationParameters& params) {
    str << "Sparsification Parameters:" << std::endl;
    str << "  Use Degree-Zero HN Contractions:    " << std::boolalpha << params.use_degree_zero_contractions << std::endl;
    str << "  Use Heavy Net Removal:              " << std::boolalpha << params.use_heavy_net_removal << std::endl;
    str << "  Use Similiar Net Removal:           " << std::boolalpha << params.use_similiar_net_removal << std::endl;
    if ( params.use_heavy_net_removal ) {
      str << "  Hyperedge Pin Weight Fraction:      " << params.hyperedge_pin_weight_fraction << std::endl;
      str << "  Maximum Hyperedge Pin Weight:       " << params.max_hyperedge_pin_weight << std::endl;
    }
    if ( params.use_similiar_net_removal ) {
      str << "  Min-Hash Footprint Size:            " << params.min_hash_footprint_size << std::endl;
      str << "  Jaccard Threshold:                  " << params.jaccard_threshold << std::endl;
      str << "  Similiar Net Combiner Strategy:     " << params.similiar_net_combiner_strategy << std::endl;
    }
    return str;
  }

  std::ostream & operator<< (std::ostream& str, const InitialPartitioningParameters& params) {
    str << "Initial Partitioning Parameters:" << std::endl;
    str << "  Initial Partitioning Mode:          " << params.mode << std::endl;
    str << "  Number of Runs:                     " << params.runs << std::endl;
    str << "  Use Adaptive IP Runs:               " << std::boolalpha << params.use_adaptive_ip_runs << std::endl;
    if ( params.use_adaptive_ip_runs ) {
      str << "  Min Adaptive IP Runs:               " << params.min_adaptive_ip_runs << std::endl;
    }
    str << "  Perform Refinement On Best:         " << std::boolalpha << params.perform_refinement_on_best_partitions << std::endl;
    str << "  Fm Refinement Rounds:               " << params.fm_refinment_rounds << std::endl;
    str << "  Remove Degree-Zero HNs Before IP:   " << std::boolalpha << params.remove_degree_zero_hns_before_ip << std::endl;
    str << "  Maximum Iterations of LP IP:        " << params.lp_maximum_iterations << std::endl;
    str << "  Initial Block Size of LP IP:        " << params.lp_initial_block_size << std::endl;
    str << "\nInitial Partitioning ";
    str << params.refinement << std::endl;
    return str;
  }

  std::ostream & operator<< (std::ostream& str, const SharedMemoryParameters& params) {
    str << "Shared Memory Parameters:             " << std::endl;
    str << "  Number of Threads:                  " << params.num_threads << std::endl;
    str << "  Number of used NUMA nodes:          " << TBBInitializer::instance().num_used_numa_nodes() << std::endl;
    str << "  Use Localized Random Shuffle:       " << std::boolalpha << params.use_localized_random_shuffle << std::endl;
    str << "  Random Shuffle Block Size:          " << params.shuffle_block_size << std::endl;
    return str;
  }


  bool Context::useSparsification() const {
    return sparsification.use_degree_zero_contractions ||
           sparsification.use_heavy_net_removal ||
           sparsification.use_similiar_net_removal;
  }

  void Context::setupPartWeights(const HypernodeWeight total_hypergraph_weight) {
    if (partition.use_individual_part_weights) {
      ASSERT(static_cast<size_t>(partition.k) == partition.max_part_weights.size());
      const HypernodeWeight max_part_weights_sum = std::accumulate(partition.max_part_weights.cbegin(),
                                                                   partition.max_part_weights.cend(), 0);
      double weight_fraction = total_hypergraph_weight / static_cast<double>(max_part_weights_sum);
      HypernodeWeight perfect_part_weights_sum = 0;
      partition.perfect_balance_part_weights.clear();
      for (const HyperedgeWeight& part_weight : partition.max_part_weights) {
        const HypernodeWeight perfect_weight = ceil(weight_fraction * part_weight);
        partition.perfect_balance_part_weights.push_back(perfect_weight);
        perfect_part_weights_sum += perfect_weight;
      }

      if (max_part_weights_sum < total_hypergraph_weight) {
        ERROR("Sum of individual part weights is less than the total hypergraph weight. Finding a valid partition is impossible.\n"
                << "Total hypergraph weight: " << total_hypergraph_weight << "\n"
                << "Sum of part weights:     " << max_part_weights_sum);
      } else {
        // To avoid rounding issues, epsilon should be calculated using the sum of the perfect part weights instead of
        // the total hypergraph weight. See also recursive_bipartitioning_initial_partitioner
        partition.epsilon = std::min(0.99, max_part_weights_sum / static_cast<double>(std::max(perfect_part_weights_sum, 1)) - 1);
      }
    } else {
      partition.perfect_balance_part_weights.clear();
      partition.perfect_balance_part_weights.push_back(ceil(
              total_hypergraph_weight
              / static_cast<double>(partition.k)));
      for (PartitionID part = 1; part != partition.k; ++part) {
        partition.perfect_balance_part_weights.push_back(
                partition.perfect_balance_part_weights[0]);
      }
      partition.max_part_weights.clear();
      partition.max_part_weights.push_back((1 + partition.epsilon)
                                          * partition.perfect_balance_part_weights[0]);
      for (PartitionID part = 1; part != partition.k; ++part) {
        partition.max_part_weights.push_back(partition.max_part_weights[0]);
      }
    }

    setupSparsificationParameters();
  }

  void Context::setupContractionLimit(const HypernodeWeight total_hypergraph_weight) {
    // Setup contraction limit
    if (initial_partitioning.mode == Mode::deep_multilevel) {
      coarsening.contraction_limit =
              2 * std::max(shared_memory.num_threads, static_cast<size_t>(partition.k)) *
              coarsening.contraction_limit_multiplier;
    } else {
      coarsening.contraction_limit =
              coarsening.contraction_limit_multiplier * partition.k;
    }

    // Setup maximum allowed vertex and high-degree vertex weight
    setupMaximumAllowedNodeWeight(total_hypergraph_weight);
  }

  void Context::setupMaximumAllowedNodeWeight(const HypernodeWeight total_hypergraph_weight) {
    HypernodeWeight min_block_weight = std::numeric_limits<HypernodeWeight>::max();
    for ( PartitionID part_id = 0; part_id < partition.k; ++part_id ) {
      min_block_weight = std::min(min_block_weight, partition.max_part_weights[part_id]);
    }

    double hypernode_weight_fraction =
            coarsening.max_allowed_weight_multiplier
            / coarsening.contraction_limit;
    coarsening.max_allowed_node_weight =
            std::ceil(hypernode_weight_fraction * total_hypergraph_weight);
    coarsening.max_allowed_node_weight =
            std::min(coarsening.max_allowed_node_weight, min_block_weight);
  }

  void Context::setupSparsificationParameters() {
    if ( sparsification.use_heavy_net_removal ) {
      HypernodeWeight max_block_weight = 0;
      for ( PartitionID block = 0; block < partition.k; ++block ) {
        max_block_weight = std::max(max_block_weight, partition.max_part_weights[block]);
      }

      sparsification.max_hyperedge_pin_weight = max_block_weight /
                                                sparsification.hyperedge_pin_weight_fraction;
    }
  }

  void Context::sanityCheck() {
    if ( partition.paradigm == Paradigm::nlevel &&
         coarsening.algorithm == CoarseningAlgorithm::multilevel_coarsener ) {
        ALGO_SWITCH("Coarsening algorithm" << coarsening.algorithm << "is only supported in multilevel mode."
                                           << "Do you want to use the n-level version instead (Y/N)?",
                    "Partitioning with" << coarsening.algorithm
                                        << "coarsener in n-level mode is not supported!",
                    coarsening.algorithm,
                    CoarseningAlgorithm::nlevel_coarsener);
    } else if ( partition.paradigm == Paradigm::multilevel &&
                coarsening.algorithm == CoarseningAlgorithm::nlevel_coarsener ) {
        ALGO_SWITCH("Coarsening algorithm" << coarsening.algorithm << "is only supported in n-Level mode."
                                           << "Do you want to use the multilevel version instead (Y/N)?",
                    "Partitioning with" << coarsening.algorithm
                                        << "coarsener in multilevel mode is not supported!",
                    coarsening.algorithm,
                    CoarseningAlgorithm::multilevel_coarsener);
    }

    if (partition.objective == kahypar::Objective::cut) {
      if ( refinement.label_propagation.algorithm == LabelPropagationAlgorithm::label_propagation_km1 ) {
        ALGO_SWITCH("Refinement algorithm" << refinement.label_propagation.algorithm << "only works for km1 metric."
                                           << "Do you want to use the cut version of the label propagation refiner (Y/N)?",
                    "Partitioning with" << refinement.label_propagation.algorithm
                                        << "refiner in combination with cut metric is not supported!",
                    refinement.label_propagation.algorithm,
                    LabelPropagationAlgorithm::label_propagation_cut);
      }

      if ( refinement.fm.algorithm != FMAlgorithm::do_nothing ) {
        ALGO_SWITCH("Refinement algorithm" << refinement.fm.algorithm << "only works for km1 metric."
                                           << "Do you want to disable FM refinement (Y/N)?",
                    "Partitioning with" << refinement.fm.algorithm
                                        << "refiner in combination with cut metric is not supported!",
                    refinement.fm.algorithm,
                    FMAlgorithm::do_nothing);
      }
    } else if (partition.objective == kahypar::Objective::km1 &&
               refinement.label_propagation.algorithm == LabelPropagationAlgorithm::label_propagation_cut) {
      ALGO_SWITCH("Refinement algorithm" << refinement.label_propagation.algorithm << "only works for cut metric."
                                         << "Do you want to use the km1 version of the label propagation refiner (Y/N)?",
                  "Partitioning with" << refinement.label_propagation.algorithm
                                      << "refiner in combination with km1 metric is not supported!",
                  refinement.label_propagation.algorithm,
                  LabelPropagationAlgorithm::label_propagation_km1);
    }

    if (partition.objective == kahypar::Objective::cut) {
      if ( initial_partitioning.refinement.label_propagation.algorithm ==
           LabelPropagationAlgorithm::label_propagation_km1 ) {
        ALGO_SWITCH("Initial Partitioning Refinement algorithm"
                            << initial_partitioning.refinement.label_propagation.algorithm
                            << "only works for km1 metric."
                            << "Do you want to use the cut version of the label propagation refiner (Y/N)?",
                    "Partitioning with" << initial_partitioning.refinement.label_propagation.algorithm
                                        << "refiner in combination with cut metric is not supported!",
                    initial_partitioning.refinement.label_propagation.algorithm,
                    LabelPropagationAlgorithm::label_propagation_cut);
      }

      if ( initial_partitioning.refinement.fm.algorithm != FMAlgorithm::do_nothing ) {
        ALGO_SWITCH("Initial Partitioning Refinement algorithm"
                            << initial_partitioning.refinement.fm.algorithm
                            << "only works for km1 metric."
                            << "Do you want to disable FM refinement (Y/N)?",
                    "Partitioning with" << initial_partitioning.refinement.fm.algorithm
                                        << "refiner in combination with cut metric is not supported!",
                    initial_partitioning.refinement.fm.algorithm,
                    FMAlgorithm::do_nothing);
      }
    } else if (partition.objective == kahypar::Objective::km1 &&
               initial_partitioning.refinement.label_propagation.algorithm ==
               LabelPropagationAlgorithm::label_propagation_cut) {
      ALGO_SWITCH("Initial Partitioning Refinement algorithm"
                          << initial_partitioning.refinement.label_propagation.algorithm
                          << "only works for cut metric."
                          << "Do you want to use the km1 version of the label propagation refiner (Y/N)?",
                  "Partitioning with" << initial_partitioning.refinement.label_propagation.algorithm
                                      << "refiner in combination with km1 metric is not supported!",
                  initial_partitioning.refinement.label_propagation.algorithm,
                  LabelPropagationAlgorithm::label_propagation_km1);
    }

    ASSERT(partition.use_individual_part_weights != partition.max_part_weights.empty());
    if (partition.use_individual_part_weights && static_cast<size_t>(partition.k) != partition.max_part_weights.size()) {
      ALGO_SWITCH("Individual part weights specified, but number of parts doesn't match k."
                          << "Do you want to use k =" << partition.max_part_weights.size() << "instead (Y/N)?",
                  "Number of parts is not equal to k!",
                  partition.k,
                  partition.max_part_weights.size());
    }


    shared_memory.static_balancing_work_packages = std::clamp(shared_memory.static_balancing_work_packages, 4UL, 256UL);

    if ( partition.deterministic ) {
      coarsening.algorithm = CoarseningAlgorithm::deterministic_multilevel_coarsener;

      // disable FM until we have a deterministic version
      refinement.fm.algorithm = FMAlgorithm::do_nothing;
      initial_partitioning.refinement.fm.algorithm = FMAlgorithm::do_nothing;

      // disable adaptive IP
      initial_partitioning.use_adaptive_ip_runs = false;


      // switch silently
      auto lp_algo = refinement.label_propagation.algorithm;
      if ( lp_algo != LabelPropagationAlgorithm::do_nothing && lp_algo != LabelPropagationAlgorithm::deterministic ) {
        refinement.label_propagation.algorithm = LabelPropagationAlgorithm::deterministic;
      }

      lp_algo = initial_partitioning.refinement.label_propagation.algorithm;
      if ( lp_algo != LabelPropagationAlgorithm::do_nothing && lp_algo != LabelPropagationAlgorithm::deterministic ) {
        initial_partitioning.refinement.label_propagation.algorithm = LabelPropagationAlgorithm::deterministic;
      }
    }
  }

  void Context::setupThreadsPerFlowSearch() {
    if ( refinement.flows.algorithm == FlowAlgorithm::flow_cutter ) {
      // = min(t, min(tau * k, k * (k - 1) / 2))
      // t = number of threads
      // k * (k - 1) / 2 = maximum number of edges in the quotient graph
      refinement.flows.num_parallel_searches = partition.k == 2 ? 1 :
        std::min(shared_memory.num_threads, std::min(std::max(1UL, static_cast<size_t>(
          refinement.flows.parallel_searches_multiplier * partition.k)),
            static_cast<size_t>((partition.k * (partition.k - 1)) / 2) ));
    }
  }

  std::ostream & operator<< (std::ostream& str, const Context& context) {
    str << "*******************************************************************************\n"
        << "*                            Partitioning Context                             *\n"
        << "*******************************************************************************\n"
        << context.partition
        << "-------------------------------------------------------------------------------\n"
        << context.preprocessing
        << "-------------------------------------------------------------------------------\n"
        << context.coarsening
        << "-------------------------------------------------------------------------------\n"
        << context.initial_partitioning
        << "-------------------------------------------------------------------------------\n"
        << context.refinement
        << "-------------------------------------------------------------------------------\n"
        #ifdef KAHYPAR_ENABLE_EXPERIMENTAL_FEATURES
        << context.sparsification
        << "-------------------------------------------------------------------------------\n"
        #endif
        << context.shared_memory
        << "-------------------------------------------------------------------------------";
    return str;
  }

}
