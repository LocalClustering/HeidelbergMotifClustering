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

#include "command_line_options.h"

#include <boost/program_options.hpp>
#include <sys/ioctl.h>

#include <fstream>
#include <limits>

#include "mt-kahypar/io/partitioning_output.h"


namespace po = boost::program_options;

namespace mt_kahypar {
  namespace platform {
    int getTerminalWidth() {
      int columns = 0;
      struct winsize w = { };
      ioctl(0, TIOCGWINSZ, &w);
      columns = w.ws_col;
      return columns;
    }

    int getProcessID() {
      return getpid();
    }
  }  // namespace platform

  po::options_description createGeneralOptionsDescription(Context& context, const int num_columns) {
    po::options_description options("General Options", num_columns);
    options.add_options()
            ("help", "show help message")
            ("deterministic", po::value<bool>(&context.partition.deterministic)->value_name("<bool>")->default_value(false),
             "Shortcut to enables deterministic partitioning mode, where results are reproducible across runs. "
             "If set, the specific deterministic subroutines don't need to be set manually.")
            ("verbose,v", po::value<bool>(&context.partition.verbose_output)->value_name("<bool>")->default_value(true),
             "Verbose main partitioning output")
            ("write-partition-file",
             po::value<bool>(&context.partition.write_partition_file)->value_name("<bool>")->default_value(false),
             "If true, then partition output file is generated")
            ("partition-output-folder",
             po::value<std::string>(&context.partition.graph_partition_output_folder)->value_name("<string>"),
             "Output folder for partition file")
            ("input-file-format",
             po::value<std::string>()->value_name("<string>")->notifier([&](const std::string& s) {
               if (s == "hmetis") {
                 context.partition.file_format = FileFormat::hMetis;
               } else if (s == "metis") {
                 context.partition.file_format = FileFormat::Metis;
               }
             }),
             "Input file format: \n"
             " - hmetis : hMETIS hypergraph file format \n"
             " - metis : METIS graph file format")
            ("instance-type",
             po::value<std::string>()->value_name("<string>")->notifier([&](const std::string& type) {
               context.partition.instance_type = instanceTypeFromString(type);
             }),
             "Instance Type: \n"
             " - graph\n"
             " - hypergraph")
            ("preset-type",
             po::value<std::string>()->value_name("<string>")->notifier([&](const std::string& type) {
               context.partition.preset_type = presetTypeFromString(type);
             }),
             "Preset Type: \n"
             " - deterministic (Mt-KaHyPar-Det)\n"
             " - default (Mt-KaHyPar-D)\n"
             " - default_flows (Mt-KaHyPar-D-F)\n"
             " - quality (Mt-KaHyPar-Q)\n"
             " - quality_flows (Mt-KaHyPar-Q-F)\n")
            ("seed",
             po::value<int>(&context.partition.seed)->value_name("<int>")->default_value(0),
             "Seed for random number generator")
            ("num-vcycles",
             po::value<size_t>(&context.partition.num_vcycles)->value_name("<size_t>")->default_value(0),
             "Number of V-Cycles")
            ("maxnet-remove-factor",
             po::value<double>(&context.partition.large_hyperedge_size_threshold_factor)->value_name(
                     "<double>")->default_value(0.01),
             "Hyperedges larger than |V| * (this factor) are removed before partitioning.")
            ("maxnet-ignore",
             po::value<HyperedgeID>(&context.partition.ignore_hyperedge_size_threshold)->value_name(
                     "<uint64_t>")->default_value(1000),
             "Hyperedges larger than this threshold are ignored during partitioning.")
            ("show-detailed-timings",
             po::value<bool>(&context.partition.show_detailed_timings)->value_name("<bool>")->default_value(false),
             "If true, shows detailed subtimings of each multilevel phase at the end of the partitioning process.")
            ("show-detailed-clustering-timings",
             po::value<bool>(&context.partition.show_detailed_clustering_timings)->value_name("<bool>")->default_value(
                     false),
             "If true, shows detailed timings of each clustering iteration.")
            ("measure-detailed-uncontraction-timings",
             po::value<bool>(&context.partition.measure_detailed_uncontraction_timings)->value_name("<bool>")->default_value(
                     false),
             "If true, measure and show detailed timings for n-level uncontraction.")
            ("timings-output-depth",
             po::value<size_t>(&context.partition.timings_output_depth)->value_name("<size_t>"),
             "Number of levels shown in timing output")
            ("show-memory-consumption",
             po::value<bool>(&context.partition.show_memory_consumption)->value_name("<bool>")->default_value(false),
             "If true, shows detailed information on how much memory was allocated and how memory was reused throughout partitioning.")
            ("show-advanced-cut-analysis",
             po::value<bool>(&context.partition.show_advanced_cut_analysis)->value_name("<bool>")->default_value(false),
             "If true, calculates cut matrix, potential positive gain move matrix and connected cut hyperedge components after partitioning.")
            ("enable-progress-bar",
             po::value<bool>(&context.partition.enable_progress_bar)->value_name("<bool>")->default_value(false),
             "If true, shows a progress bar during coarsening and refinement phase.")
            ("time-limit", po::value<int>(&context.partition.time_limit)->value_name("<int>"),
             "Time limit in seconds (currently not supported)")
            ("sp-process,s",
             po::value<bool>(&context.partition.sp_process_output)->value_name("<bool>")->default_value(false),
             "Summarize partitioning results in RESULT line compatible with sqlplottools "
             "(https://github.com/bingmann/sqlplottools)")
            ("csv", po::value<bool>(&context.partition.csv_output)->value_name("<bool>")->default_value(false),
             "Summarize results in CSV format")
            ("algorithm-name",
             po::value<std::string>(&context.algorithm_name)->value_name("<std::string>")->default_value("MT-KaHyPar"),
             "An algorithm name to print into the summarized output (csv or sqlplottools). ")
            ("part-weights",
             po::value<std::vector<HypernodeWeight> >(&context.partition.max_part_weights)->multitoken()->notifier(
                     [&](auto) {
                             context.partition.use_individual_part_weights = true;
                     }),
             "Use the specified individual part weights instead of epsilon.");
    return options;
  }

  po::options_description createPreprocessingOptionsDescription(Context& context, const int num_columns) {
    po::options_description options("Preprocessing Options", num_columns);
    options.add_options()
            ("p-stable-io",
             po::value<bool>(&context.preprocessing.stable_construction_of_incident_edges)->value_name(
                     "<bool>")->default_value(false),
             "If true, the incident edges of a vertex are sorted after construction, so that the hypergraph "
             "data structure is independent of scheduling during construction.")
            ("p-enable-community-detection",
             po::value<bool>(&context.preprocessing.use_community_detection)->value_name("<bool>")->default_value(true),
             "If true, community detection is used as preprocessing step to restrict contractions to densely coupled regions in coarsening phase")
            #ifdef USE_GRAPH_PARTITIONER
            ("p-disable-community-detection-on-mesh-graphs",
             po::value<bool>(&context.preprocessing.disable_community_detection_for_mesh_graphs)->value_name("<bool>")->default_value(true),
             "If true, community detection is dynamically disabled for mesh graphs (as it is not effective for this type of graphs).")
            #endif
            ("p-louvain-edge-weight-function",
             po::value<std::string>()->value_name("<string>")->notifier(
                     [&](const std::string& type) {
                       context.preprocessing.community_detection.edge_weight_function = louvainEdgeWeightFromString(
                               type);
                     })->default_value("hybrid"),
             "Louvain edge weight functions:\n"
             "- hybrid\n"
             "- uniform\n"
             "- non_uniform\n"
             "- degree")
            ("p-max-louvain-pass-iterations",
             po::value<uint32_t>(&context.preprocessing.community_detection.max_pass_iterations)->value_name(
                     "<uint32_t>")->default_value(5),
             "Maximum number of iterations over all nodes of one louvain pass")
            ("p-louvain-low-memory-contraction",
             po::value<bool>(&context.preprocessing.community_detection.low_memory_contraction)->value_name(
                     "<bool>")->default_value(false),
             "Maximum number of iterations over all nodes of one louvain pass")
            ("p-louvain-min-vertex-move-fraction",
             po::value<long double>(&context.preprocessing.community_detection.min_vertex_move_fraction)->value_name(
                     "<long double>")->default_value(0.01),
             "Louvain pass terminates if less than that fraction of nodes moves during a pass")
            ("p-vertex-degree-sampling-threshold",
             po::value<size_t>(&context.preprocessing.community_detection.vertex_degree_sampling_threshold)->value_name(
                     "<size_t>")->default_value(std::numeric_limits<size_t>::max()),
             "If set, then neighbors of a vertex are sampled during rating if its degree is greater than this threshold.")
            ("p-num-sub-rounds",
             po::value<size_t>(&context.preprocessing.community_detection.num_sub_rounds_deterministic)->value_name(
                     "<size_t>")->default_value(16),
             "Number of sub-rounds used for deterministic community detection in preprocessing.");
    return options;
  }

  po::options_description createCoarseningOptionsDescription(Context& context,
                                                             const int num_columns) {
    po::options_description options("Coarsening Options", num_columns);
    options.add_options()
            ("c-type",
             po::value<std::string>()->value_name("<string>")->notifier(
                     [&](const std::string& ctype) {
                       context.coarsening.algorithm = mt_kahypar::coarseningAlgorithmFromString(ctype);
                     })->default_value("multilevel_coarsener"),
             "Coarsening Algorithm:\n"
             " - multilevel_coarsener"
             " - nlevel_coarsener"
             " - deterministic_multilevel_coarsener"
             )
            ("c-use-adaptive-edge-size",
             po::value<bool>(&context.coarsening.use_adaptive_edge_size)->value_name("<bool>")->default_value(true),
             "If true, the rating function uses the number of distinct cluster IDs of a net as edge size rather\n"
             "than its original size during multilevel coarsing")
            #ifdef KAHYPAR_ENABLE_EXPERIMENTAL_FEATURES
                        ("c-use-adaptive-max-node-weight",
                po::value<bool>(&context.coarsening.use_adaptive_max_allowed_node_weight)->value_name("<bool>")->default_value(false),
                "If true, we double the maximum allowed node weight each time if we are not able\n"
                "to significantly reduce the size of the hypergraph during coarsening.")
                ("c-adaptive-s",
                po::value<double>(&context.coarsening.max_allowed_weight_fraction)->value_name("<double>"),
                "The maximum allowed node weight is not allowed to become greater than\n"
                "((1 + epsilon) * w(H)/k) / (adaptive_s), if adaptive maximum node weight is enabled\n")
                ("c-adaptive-threshold",
                po::value<double>(&context.coarsening.adaptive_node_weight_shrink_factor_threshold)->value_name("<double>"),
                "The maximum allowed node weight is adapted, if the reduction ratio of vertices or pins\n"
                "is lower than this threshold\n")
            #endif
            ("c-s",
             po::value<double>(&context.coarsening.max_allowed_weight_multiplier)->value_name(
                     "<double>")->default_value(1),
             "The maximum weight of a vertex in the coarsest hypergraph H is:\n"
             "(s * w(H)) / (t * k)\n")
            ("c-t",
             po::value<HypernodeID>(&context.coarsening.contraction_limit_multiplier)->value_name(
                     "<int>")->default_value(160),
             "Coarsening stops when there are no more than t * k hypernodes left")
            ("c-min-shrink-factor",
             po::value<double>(&context.coarsening.minimum_shrink_factor)->value_name("<double>")->default_value(1.01),
             "Minimum factor a hypergraph must shrink in a multilevel pass. Otherwise, we terminate coarsening phase.")
            ("c-max-shrink-factor",
             po::value<double>(&context.coarsening.maximum_shrink_factor)->value_name("<double>")->default_value(2.5),
             "Maximum factor a hypergraph is allowed to shrink in a clustering pass")
            ("c-rating-score",
             po::value<std::string>()->value_name("<string>")->notifier(
                     [&](const std::string& rating_score) {
                       context.coarsening.rating.rating_function =
                               mt_kahypar::ratingFunctionFromString(rating_score);
                     })->default_value("heavy_edge"), "Rating function used to calculate scores for vertex pairs:\n"
                                                      "- heavy_edge")
            ("c-rating-heavy-node-penalty",
             po::value<std::string>()->value_name("<string>")->notifier(
                     [&](const std::string& penalty) {
                       context.coarsening.rating.heavy_node_penalty_policy =
                               heavyNodePenaltyFromString(penalty);
                     })->default_value("no_penalty"),
             "Penalty function to discourage heavy vertices:\n"
             "- multiplicative\n"
             "- no_penalty\n"
             "- edge_frequency_penalty")
            ("c-rating-acceptance-criterion",
             po::value<std::string>()->value_name("<string>")->notifier(
                     [&](const std::string& crit) {
                       context.coarsening.rating.acceptance_policy =
                               acceptanceCriterionFromString(crit);
                     })->default_value("best_prefer_unmatched"),
             "Acceptance/Tiebreaking criterion for contraction partners having the same score:\n"
             "- best\n"
             "- best_prefer_unmatched")
            ("c-vertex-degree-sampling-threshold",
             po::value<size_t>(&context.coarsening.vertex_degree_sampling_threshold)->value_name(
                     "<size_t>")->default_value(std::numeric_limits<size_t>::max()),
             "If set, then neighbors of a vertex are sampled during rating if its degree is greater than this threshold.")
            ("c-num-sub-rounds",
             po::value<size_t>(&context.coarsening.num_sub_rounds_deterministic)->value_name(
                     "<size_t>")->default_value(16),
             "Number of sub-rounds used for deterministic coarsening.");
    return options;
  }

  po::options_description createRefinementOptionsDescription(Context& context,
                                                             const int num_columns,
                                                             const bool initial_partitioning) {
    po::options_description options("Refinement Options", num_columns);
    options.add_options()
            ((initial_partitioning ? "i-r-refine-until-no-improvement" : "r-refine-until-no-improvement"),
             po::value<bool>((!initial_partitioning ? &context.refinement.refine_until_no_improvement :
                              &context.initial_partitioning.refinement.refine_until_no_improvement))->value_name(
                     "<bool>")->default_value(false),
             "Executes all refinement algorithms as long as they find an improvement on the current partition.")
            ((initial_partitioning ? "i-r-relative-improvement-threshold" : "r-relative-improvement-threshold"),
             po::value<double>((!initial_partitioning ? &context.refinement.relative_improvement_threshold :
                              &context.initial_partitioning.refinement.relative_improvement_threshold))->value_name(
                     "<double>")->default_value(0.0),
             "If the relative improvement during a refinement pass is less than this threshold, than refinement is aborted.")
            (( initial_partitioning ? "i-r-max-batch-size" : "r-max-batch-size"),
             po::value<size_t>((!initial_partitioning ? &context.refinement.max_batch_size :
                                &context.initial_partitioning.refinement.max_batch_size))->value_name("<size_t>")->default_value(1000),
             "Maximum size of an uncontraction batch (n-Level Partitioner).")
            (( initial_partitioning ? "i-r-min-border-vertices-per-thread" : "r-min-border-vertices-per-thread"),
             po::value<size_t>((!initial_partitioning ? &context.refinement.min_border_vertices_per_thread :
                                &context.initial_partitioning.refinement.min_border_vertices_per_thread))->value_name("<size_t>")->default_value(0),
             "Minimum number of border vertices per thread with which we perform a localized search (n-Level Partitioner).")
            ((initial_partitioning ? "i-r-lp-type" : "r-lp-type"),
             po::value<std::string>()->value_name("<string>")->notifier(
                     [&, initial_partitioning](const std::string& type) {
                       if (initial_partitioning) {
                         context.initial_partitioning.refinement.label_propagation.algorithm =
                                 labelPropagationAlgorithmFromString(type);
                       } else {
                         context.refinement.label_propagation.algorithm =
                                 labelPropagationAlgorithmFromString(type);
                       }
                     })->default_value("label_propagation_km1"),
             "Label Propagation Algorithm:\n"
             "- label_propagation_km1\n"
             "- label_propagation_cut\n"
             "- deterministic\n"
             "- do_nothing")
            ((initial_partitioning ? "i-r-lp-maximum-iterations" : "r-lp-maximum-iterations"),
             po::value<size_t>((!initial_partitioning ? &context.refinement.label_propagation.maximum_iterations :
                                &context.initial_partitioning.refinement.label_propagation.maximum_iterations))->value_name(
                     "<size_t>")->default_value(5),
             "Maximum number of label propagation rounds")
            ((initial_partitioning ? "i-r-sync-lp-sub-rounds" : "r-sync-lp-sub-rounds"),
             po::value<size_t>((!initial_partitioning ? &context.refinement.deterministic_refinement.num_sub_rounds_sync_lp :
                                &context.initial_partitioning.refinement.deterministic_refinement.num_sub_rounds_sync_lp))->value_name(
                     "<size_t>")->default_value(5),
             "Number of sub-rounds for deterministic synchronous label propagation")
            ((initial_partitioning ? "i-r-sync-lp-active-nodeset" : "r-sync-lp-active-nodeset"),
             po::value<bool>((!initial_partitioning ? &context.refinement.deterministic_refinement.use_active_node_set :
                                &context.initial_partitioning.refinement.deterministic_refinement.use_active_node_set))->value_name(
                     "<bool>")->default_value(true),
             "Number of sub-rounds for deterministic synchronous label propagation")
            ((initial_partitioning ? "i-r-lp-rebalancing" : "r-lp-rebalancing"),
             po::value<bool>((!initial_partitioning ? &context.refinement.label_propagation.rebalancing :
                              &context.initial_partitioning.refinement.label_propagation.rebalancing))->value_name(
                     "<bool>")->default_value(true),
             "If true, then zero gain moves are only performed if they improve the balance of the solution (only in label propagation)")
            ((initial_partitioning ? "i-r-lp-he-size-activation-threshold" : "r-lp-he-size-activation-threshold"),
             po::value<size_t>(
                     (!initial_partitioning ? &context.refinement.label_propagation.hyperedge_size_activation_threshold
                                            :
                      &context.initial_partitioning.refinement.label_propagation.hyperedge_size_activation_threshold))->value_name(
                     "<size_t>")->default_value(100),
             "LP refiner activates only neighbors of moved vertices that are part of hyperedges with a size less than this threshold")
            ((initial_partitioning ? "i-r-fm-type" : "r-fm-type"),
             po::value<std::string>()->value_name("<string>")->notifier(
                     [&, initial_partitioning](const std::string& type) {
                       if (initial_partitioning) {
                         context.initial_partitioning.refinement.fm.algorithm = fmAlgorithmFromString(type);
                       } else {
                         context.refinement.fm.algorithm = fmAlgorithmFromString(type);
                       }
                     })->default_value("fm_gain_cache"),
             "FM Algorithm:\n"
             "- fm_gain_cache\n"
             "- fm_gain_cache_on_demand\n"
             "- fm_gain_delta\n"
             "- fm_recompute_gain\n"
             "- do_nothing")
            ((initial_partitioning ? "i-r-fm-multitry-rounds" : "r-fm-multitry-rounds"),
             po::value<size_t>((initial_partitioning ? &context.initial_partitioning.refinement.fm.multitry_rounds :
                                &context.refinement.fm.multitry_rounds))->value_name("<size_t>")->default_value(10),
             "Number of FM rounds within one level of the multilevel hierarchy.")
            ((initial_partitioning ? "i-r-fm-perform-moves-global" : "r-fm-perform-moves-global"),
             po::value<bool>((initial_partitioning ? &context.initial_partitioning.refinement.fm.perform_moves_global :
                              &context.refinement.fm.perform_moves_global))->value_name("<bool>")->default_value(false),
             "If true, then all moves performed during FM are immediately visible to other searches.\n"
             "Otherwise, only move sequences that yield an improvement are applied to the global view of the partition.")
            ((initial_partitioning ? "i-r-fm-seed-nodes" : "r-fm-seed-nodes"),
             po::value<size_t>((initial_partitioning ? &context.initial_partitioning.refinement.fm.num_seed_nodes :
                                &context.refinement.fm.num_seed_nodes))->value_name("<size_t>")->default_value(25),
             "Number of nodes to start the 'highly localized FM' with.")
            (( initial_partitioning ? "i-r-fm-rollback-parallel" : "r-fm-rollback-parallel"),
             po::value<bool>((initial_partitioning ? &context.initial_partitioning.refinement.fm.rollback_parallel :
                              &context.refinement.fm.rollback_parallel))
                              ->value_name("<bool>")->default_value(true),
             "Perform gain and balance recalculation, and reverting to best prefix in parallel.")
            (( initial_partitioning ?
                              "i-r-fm-iter-moves-on-recalc" :
                              "r-fm-iter-moves-on-recalc"),
             po::value<bool>((initial_partitioning ?
                              &context.initial_partitioning.refinement.fm.iter_moves_on_recalc :
                              &context.refinement.fm.iter_moves_on_recalc))
                     ->value_name("<bool>")->default_value(false),
             "Touch only incident hyperedges of moved vertices for parallel gain recalculation.")
            ((initial_partitioning ? "i-r-fm-rollback-balance-violation-factor"
                                   : "r-fm-rollback-balance-violation-factor"),
             po::value<double>((initial_partitioning
                                ? &context.initial_partitioning.refinement.fm.rollback_balance_violation_factor :
                                &context.refinement.fm.rollback_balance_violation_factor))->value_name(
                     "<double>")->default_value(1.25),
             "Used to relax or disable the balance constraint during the rollback phase of parallel FM."
             "Set to 0 for disabling. Set to a value > 1.0 to multiply epsilon with this value.")
            ((initial_partitioning ? "i-r-fm-min-improvement" : "r-fm-min-improvement"),
             po::value<double>((initial_partitioning ? &context.initial_partitioning.refinement.fm.min_improvement :
                                &context.refinement.fm.min_improvement))->value_name("<double>")->default_value(-1.0),
             "Min improvement for FM (default disabled)")
            ((initial_partitioning ? "i-r-fm-release-nodes" : "r-fm-release-nodes"),
             po::value<bool>((initial_partitioning ? &context.initial_partitioning.refinement.fm.release_nodes :
                              &context.refinement.fm.release_nodes))->value_name("<bool>")->default_value(true),
             "FM releases nodes that weren't moved, so they might be found by another search.")
            ((initial_partitioning ? "i-r-fm-obey-minimal-parallelism" : "r-fm-obey-minimal-parallelism"),
             po::value<bool>(
                     (initial_partitioning ? &context.initial_partitioning.refinement.fm.obey_minimal_parallelism :
                      &context.refinement.fm.obey_minimal_parallelism))->value_name("<bool>")->default_value(true),
             "If true, then parallel FM refinement stops if more than a certain number of threads are finished.")
            ((initial_partitioning ? "i-r-fm-time-limit-factor" : "r-fm-time-limit-factor"),
             po::value<double>((initial_partitioning ? &context.initial_partitioning.refinement.fm.time_limit_factor :
                                &context.refinement.fm.time_limit_factor))->value_name("<double>")->default_value(0.25),
             "If the FM time exceeds time_limit := k * factor * coarsening_time, than the FM config is switched into a light version."
             "If the FM refiner exceeds 2 * time_limit, than the current multitry FM run is aborted and the algorithm proceeds to"
             "the next finer level.")
            #ifdef USE_STRONG_PARTITIONER
            ((initial_partitioning ? "i-r-use-global-fm" : "r-use-global-fm"),
             po::value<bool>((!initial_partitioning ? &context.refinement.global_fm.use_global_fm :
                              &context.initial_partitioning.refinement.global_fm.use_global_fm))->value_name(
                     "<bool>")->default_value(false),
             "If true, than we execute a globalized FM local search interleaved with the localized searches."
             "Note, gobalized FM local searches are performed in multilevel style (not after each batch uncontraction)")
            ((initial_partitioning ? "i-r-global-refine-until-no-improvement" : "r-global-refine-until-no-improvement"),
             po::value<bool>((!initial_partitioning ? &context.refinement.global_fm.refine_until_no_improvement :
                              &context.initial_partitioning.refinement.global_fm.refine_until_no_improvement))->value_name(
                     "<bool>")->default_value(false),
             "Executes a globalized FM local search as long as it finds an improvement on the current partition.")
            ((initial_partitioning ? "i-r-global-fm-seed-nodes" : "r-global-fm-seed-nodes"),
             po::value<size_t>((initial_partitioning ? &context.initial_partitioning.refinement.global_fm.num_seed_nodes :
                                &context.refinement.global_fm.num_seed_nodes))->value_name("<size_t>")->default_value(25),
             "Number of nodes to start the 'highly localized FM' with during the globalized FM local search.")
            ((initial_partitioning ? "i-r-global-fm-obey-minimal-parallelism" : "r-global-fm-obey-minimal-parallelism"),
             po::value<bool>(
                     (initial_partitioning ? &context.initial_partitioning.refinement.global_fm.obey_minimal_parallelism :
                      &context.refinement.global_fm.obey_minimal_parallelism))->value_name("<bool>")->default_value(true),
             "If true, then the globalized FM local search stops if more than a certain number of threads are finished.")
            #endif
            ;
    return options;
  }

  po::options_description createFlowRefinementOptionsDescription(Context& context,
                                                                 const int num_columns,
                                                                 const bool initial_partitioning) {
    po::options_description options("Initial Partitioning Options", num_columns);
    options.add_options()
            ((initial_partitioning ? "i-r-flow-algo" : "r-flow-algo"),
             po::value<std::string>()->value_name("<string>")->notifier(
                     [&, initial_partitioning](const std::string& algo) {
                       if ( initial_partitioning ) {
                        context.initial_partitioning.refinement.flows.algorithm = flowAlgorithmFromString(algo);
                       } else {
                        context.refinement.flows.algorithm = flowAlgorithmFromString(algo);
                       }
                     })->default_value("do_nothing"),
             "Flow Algorithms:\n"
             "- do_nothing\n"
             "- flow_cutter")
            ((initial_partitioning ? "i-r-flow-parallel-search-multiplier" : "r-flow-parallel-search-multiplier"),
             po::value<double>((initial_partitioning ? &context.initial_partitioning.refinement.flows.parallel_searches_multiplier :
                      &context.refinement.flows.parallel_searches_multiplier))->value_name("<double>"),
             "Active block scheduling starts min(num_threads, mult * k) parallel searches")
            ((initial_partitioning ? "i-r-flow-max-bfs-distance" : "r-flow-max-bfs-distance"),
             po::value<size_t>((initial_partitioning ? &context.initial_partitioning.refinement.flows.max_bfs_distance :
                      &context.refinement.flows.max_bfs_distance))->value_name("<size_t>"),
             "Flow problems are constructed via BFS search. The maximum BFS distance is the\n"
             "maximum distance from a cut hyperedge to any vertex of the problem.")
            ((initial_partitioning ? "i-r-flow-min-relative-improvement-per-round" : "r-flow-min-relative-improvement-per-round"),
             po::value<double>((initial_partitioning ? &context.initial_partitioning.refinement.flows.min_relative_improvement_per_round :
                      &context.refinement.flows.min_relative_improvement_per_round))->value_name("<double>"),
             "Minimum relative improvement per active block scheduling round. If improvement is smaller than flow algorithm terminates.")
            ((initial_partitioning ? "i-r-flow-time-limit-factor" : "r-flow-time-limit-factor"),
             po::value<double>((initial_partitioning ? &context.initial_partitioning.refinement.flows.time_limit_factor :
                      &context.refinement.flows.time_limit_factor))->value_name("<double>"),
             "The time limit for each flow problem is time_limit_factor * average running time of all previous searches.")
            ((initial_partitioning ? "i-r-flow-skip-small-cuts" : "r-flow-skip-small-cuts"),
             po::value<bool>((initial_partitioning ? &context.initial_partitioning.refinement.flows.skip_small_cuts :
                      &context.refinement.flows.skip_small_cuts))->value_name("<bool>"),
             "If true, than blocks with a cut <= 10 are not considered for refinement")
            ((initial_partitioning ? "i-r-flow-skip-unpromising-blocks" : "r-flow-skip-unpromising-blocks"),
             po::value<bool>((initial_partitioning ? &context.initial_partitioning.refinement.flows.skip_unpromising_blocks :
                      &context.refinement.flows.skip_unpromising_blocks))->value_name("<bool>"),
             "If true, than blocks for which we never found an improvement are skipped")
            ((initial_partitioning ? "i-r-flow-pierce-in-bulk" : "r-flow-pierce-in-bulk"),
             po::value<bool>((initial_partitioning ? &context.initial_partitioning.refinement.flows.pierce_in_bulk :
                              &context.refinement.flows.pierce_in_bulk))->value_name("<bool>"),
             "If true, then FlowCutter is accelerated by piercing multiple nodes at a time")
            ((initial_partitioning ? "i-r-flow-scaling" : "r-flow-scaling"),
             po::value<double>((initial_partitioning ? &context.initial_partitioning.refinement.flows.alpha :
                      &context.refinement.flows.alpha))->value_name("<double>"),
             "Size constraint for flow problem: (1 + alpha * epsilon) * c(V) / k - c(V_1) (alpha = r-flow-scaling)")
            ((initial_partitioning ? "i-r-flow-max-num-pins" : "r-flow-max-num-pins"),
             po::value<uint32_t>((initial_partitioning ? &context.initial_partitioning.refinement.flows.max_num_pins :
                      &context.refinement.flows.max_num_pins))->value_name("<uint32_t>"),
             "Maximum number of pins a flow problem is allowed to contain")
            ((initial_partitioning ? "i-r-flow-find-most-balanced-cut" : "r-flow-find-most-balanced-cut"),
             po::value<bool>((initial_partitioning ? &context.initial_partitioning.refinement.flows.find_most_balanced_cut :
                      &context.refinement.flows.find_most_balanced_cut))->value_name("<bool>"),
             "If true, than hyperflowcutter searches for the most balanced minimum cut.")
            ((initial_partitioning ? "i-r-flow-determine-distance-from-cut" : "r-flow-determine-distance-from-cut"),
             po::value<bool>((initial_partitioning ? &context.initial_partitioning.refinement.flows.determine_distance_from_cut :
                      &context.refinement.flows.determine_distance_from_cut))->value_name("<bool>"),
             "If true, than flow refiner determines distance of each node from cut which improves the piercing heuristic used in WHFC.");
    return options;
  }

  po::options_description createInitialPartitioningOptionsDescription(Context& context, const int num_columns) {
    po::options_description options("Initial Partitioning Options", num_columns);
    options.add_options()
            ("i-mode",
             po::value<std::string>()->value_name("<string>")->notifier(
                     [&](const std::string& mode) {
                       context.initial_partitioning.mode = modeFromString(mode);
                     })->default_value("rb"),
             "Mode of initial partitioning:\n"
             "- direct\n"
             "- deep\n"
             "- rb")
            ("i-enabled-ip-algos",
            po::value<std::vector<bool> >(&context.initial_partitioning.enabled_ip_algos)->multitoken(),
            "Indicate which IP algorithms should be executed. E.g. i-enabled-ip-algos=1 1 0 1 0 1 1 1 0\n"
            "indicates that\n"
            "  1.) greedy_round_robin_fm      (is executed)\n"
            "  2.) greedy_global_fm           (is executed)\n"
            "  3.) greedy_sequential_fm       (is NOT executed)\n"
            "  4.) random                     (is executed)\n"
            "  5.) bfs                        (is NOT executed)\n"
            "  6.) label_propagation          (is executed)\n"
            "  7.) greedy_round_robin_max_net (is executed)\n"
            "  8.) greedy_global_max_net      (is executed)\n"
            "  9.) greedy_sequential_max_net  (is NOT executed)\n"
            "Note vector must exactly contain 9 values otherwise partitioner will exit with failure")
            ("i-runs",
             po::value<size_t>(&context.initial_partitioning.runs)->value_name("<size_t>")->default_value(20),
             "Number of runs for each bipartitioning algorithm.")
            ("i-use-adaptive-ip-runs",
             po::value<bool>(&context.initial_partitioning.use_adaptive_ip_runs)->value_name("<bool>")->default_value(true),
             "If true, than each initial partitioner decides if it should further continue partitioning based on the"
             "quality produced by itself compared to the quality of the other partitioners. If it is not likely that the partitioner"
             "will produce a solution with a quality better than the current best, further runs of that partitioner are omitted.")
            ("i-min-adaptive-ip-runs",
             po::value<size_t>(&context.initial_partitioning.min_adaptive_ip_runs)->value_name("<size_t>")->default_value(5),
             "If adaptive IP runs is enabled, than each initial partitioner performs minimum min_adaptive_ip_runs runs before\n"
             "it decides if it should terminate.")
            ("i-population-size",
             po::value<size_t>(&context.initial_partitioning.population_size)->value_name("<size_t>")->default_value(16),
             "Size of population of flat bipartitions to perform secondary FM refinement on in deterministic mode."
             "Values < num threads are set to num threads. Does not affect behavior in non-deterministic mode.")
            ("i-perform-refinement-on-best-partitions",
             po::value<bool>(&context.initial_partitioning.perform_refinement_on_best_partitions)->value_name("<bool>")->default_value(false),
             "If true, then we perform an additional refinement on the best thread local partitions after IP.")
            ("i-fm-refinement-rounds",
             po::value<size_t>(&context.initial_partitioning.fm_refinment_rounds)->value_name("<size_t>")->default_value(1),
             "Maximum number of 2-way FM local searches on each bipartition produced by an initial partitioner.")
            ("i-remove-degree-zero-hns-before-ip",
             po::value<bool>(&context.initial_partitioning.remove_degree_zero_hns_before_ip)->value_name("<bool>")->default_value(true),
             "If true, degree-zero vertices are removed before initial partitioning.")
            ("i-lp-maximum-iterations",
             po::value<size_t>(&context.initial_partitioning.lp_maximum_iterations)->value_name(
                     "<size_t>")->default_value(20),
             "Maximum number of iterations of label propagation initial partitioner")
            ("i-lp-initial-block-size",
             po::value<size_t>(&context.initial_partitioning.lp_initial_block_size)->value_name(
                     "<size_t>")->default_value(5),
             "Initial block size used for label propagation initial partitioner");
    options.add(createRefinementOptionsDescription(context, num_columns, true));
    options.add(createFlowRefinementOptionsDescription(context, num_columns, true));
    return options;
  }

#ifdef KAHYPAR_ENABLE_EXPERIMENTAL_FEATURES
  po::options_description createSparsificationOptionsDescription(Context& context,
                                                               const int num_columns) {
  po::options_description sparsification_options("Sparsification Options", num_columns);
  sparsification_options.add_options()
    ("sp-use-degree-zero-contractions",
    po::value<bool>(&context.sparsification.use_degree_zero_contractions)->value_name("<bool>"),
    "If true, then vertices with degree zero are contracted to supervertices")
    ("sp-use-heavy-net-removal",
    po::value<bool>(&context.sparsification.use_heavy_net_removal)->value_name("<bool>"),
    "If true, then hyperedges with a weight greater than a certain threshold are removed before IP")
    ("sp-use-similiar-net-removal",
    po::value<bool>(&context.sparsification.use_similiar_net_removal)->value_name("<bool>"),
    "If true, then hyperedges with a jaccard similiarity greater than a certain threshold are removed before IP")
    ("sp-hyperedge-pin-weight-fraction",
    po::value<double>(&context.sparsification.hyperedge_pin_weight_fraction)->value_name("<double>"),
    "Hyperedges where the sum of the weights of all pins are greater than ((1 + eps)|V|/k) / fraction are removed before IP")
    ("sp-min-hash-footprint-size",
    po::value<size_t>(&context.sparsification.min_hash_footprint_size)->value_name("<size_t>"),
    "Number of locality sensitive hash functions used for similiar hyperedge removal")
    ("sp-jaccard-threshold",
    po::value<double>(&context.sparsification.jaccard_threshold)->value_name("<double>"),
    "Jaccard threshold for which to hyperedges are considered as similiar")
    ("sp-similiar-net-combiner-strategy",
    po::value<std::string>()->value_name("<string>")->notifier(
      [&](const std::string& strategy) {
      context.sparsification.similiar_net_combiner_strategy =
        similiarNetCombinerStrategyFromString(strategy);
    }),
    "Determines how similiar nets are combined:\n"
    "- union: set union of both nets\n"
    "- max_size: largest net\n"
    "- importance: net with most 'important' pins");

  return sparsification_options;
}
#endif

  po::options_description createSharedMemoryOptionsDescription(Context& context,
                                                               const int num_columns) {
    po::options_description shared_memory_options("Shared Memory Options", num_columns);
    shared_memory_options.add_options()
            ("s-num-threads,t",
             po::value<size_t>(&context.shared_memory.num_threads)->value_name("<size_t>"),
             "Number of Threads")
            ("s-static-balancing-work-packages",
             po::value<size_t>(&context.shared_memory.static_balancing_work_packages)->value_name("<size_t>"),
             "Some sub-routines (sorting, shuffling) used in the deterministic presets employ static load balancing."
             "This parameter sets the number of work packages, in order to achieve deterministic results across different numbers of threads."
             "The default value is 128, and these sub-routines have little work, so there should rarely be a reason to change it. Max value is 256."
             "It does not affect the non-deterministic configs, unless you activate one of the deterministic algorithms."
            )
            ("s-use-localized-random-shuffle",
             po::value<bool>(&context.shared_memory.use_localized_random_shuffle)->value_name("<bool>"),
             "If true, localized parallel random shuffle is performed.")
            ("s-shuffle-block-size",
             po::value<size_t>(&context.shared_memory.shuffle_block_size)->value_name("<size_t>"),
             "If we perform a localized random shuffle in parallel, we perform a parallel for over blocks of size"
             "'shuffle_block_size' and shuffle them sequential.");

    return shared_memory_options;
  }



  void processCommandLineInput(Context& context, int argc, char *argv[]) {
    const int num_columns = platform::getTerminalWidth();


    po::options_description required_options("Required Options", num_columns);
    required_options.add_options()
            ("hypergraph,h",
             po::value<std::string>(&context.partition.graph_filename)->value_name("<string>")->required(),
             "Hypergraph filename")
            ("blocks,k",
             po::value<PartitionID>(&context.partition.k)->value_name("<int>")->required(),
             "Number of blocks")
            ("epsilon,e",
             po::value<double>(&context.partition.epsilon)->value_name("<double>")->required(),
             "Imbalance parameter epsilon")
            ("objective,o",
             po::value<std::string>()->value_name("<string>")->required()->notifier([&](const std::string& s) {
               if (s == "cut") {
                 context.partition.objective = kahypar::Objective::cut;
               } else if (s == "km1") {
                 context.partition.objective = kahypar::Objective::km1;
               }
             }),
             "Objective: \n"
             " - cut : cut-net metric (FM only supports km1 metric) \n"
             " - km1 : (lambda-1) metric")
            ("mode,m",
             po::value<std::string>()->value_name("<string>")->required()->notifier(
                     [&](const std::string& mode) {
                       context.partition.mode = modeFromString(mode);
                     }),
             "Partitioning mode: \n"
             " - direct: direct k-way partitioning\n"
             " - rb: recursive bipartitioning\n"
             " - deep: deep multilevel partitioning"
             );

    po::options_description preset_options("Preset Options", num_columns);
    preset_options.add_options()
            ("preset,p", po::value<std::string>(&context.partition.preset_file)->value_name("<string>"),
             "Context Presets (see config directory):\n"
             " - <path-to-custom-ini-file>");

    po::options_description general_options = createGeneralOptionsDescription(context, num_columns);

    po::options_description preprocessing_options =
            createPreprocessingOptionsDescription(context, num_columns);
    po::options_description coarsening_options =
            createCoarseningOptionsDescription(context, num_columns);
    po::options_description initial_paritioning_options =
            createInitialPartitioningOptionsDescription(context, num_columns);
    po::options_description refinement_options =
            createRefinementOptionsDescription(context, num_columns, false);
    po::options_description flow_options =
            createFlowRefinementOptionsDescription(context, num_columns, false);
#ifdef KAHYPAR_ENABLE_EXPERIMENTAL_FEATURES
    po::options_description sparsification_options =
    createSparsificationOptionsDescription(context, num_columns);
#endif
    po::options_description shared_memory_options =
            createSharedMemoryOptionsDescription(context, num_columns);

    po::options_description cmd_line_options;
    cmd_line_options
            .add(required_options)
            .add(preset_options)
            .add(general_options)
            .add(preprocessing_options)
            .add(coarsening_options)
            .add(initial_paritioning_options)
            .add(refinement_options)
            .add(flow_options)
#ifdef KAHYPAR_ENABLE_EXPERIMENTAL_FEATURES
                    .add(sparsification_options)
#endif
            .add(shared_memory_options);

    po::variables_map cmd_vm;
    po::store(po::parse_command_line(argc, argv, cmd_line_options), cmd_vm);

    // placing vm.count("help") here prevents required attributes raising an
    // error if only help was supplied
    if (cmd_vm.count("help") != 0 || argc == 1) {
      if (context.partition.verbose_output) {
        mt_kahypar::io::printBanner();
      }
      LOG << cmd_line_options;
      exit(0);
    }

    po::notify(cmd_vm);

    if ( context.partition.preset_file != "" ) {
      std::ifstream file(context.partition.preset_file.c_str());
      if (!file) {
        ERROR("Could not load context file at: " + context.partition.preset_file);
      }

      po::options_description ini_line_options;
      ini_line_options.add(general_options)
              .add(preprocessing_options)
              .add(coarsening_options)
              .add(initial_paritioning_options)
              .add(refinement_options)
              .add(flow_options)
  #ifdef KAHYPAR_ENABLE_EXPERIMENTAL_FEATURES
              .add(sparsification_options)
  #endif
              .add(shared_memory_options);

      po::store(po::parse_config_file(file, ini_line_options, true), cmd_vm);
      po::notify(cmd_vm);
    }

    std::string epsilon_str = std::to_string(context.partition.epsilon);
    epsilon_str.erase(epsilon_str.find_last_not_of('0') + 1, std::string::npos);

    if (context.partition.graph_partition_output_folder != "") {
      std::string graph_base_name = context.partition.graph_filename.substr(
              context.partition.graph_filename.find_last_of("/") + 1);
      context.partition.graph_partition_filename =
              context.partition.graph_partition_output_folder + "/" + graph_base_name;
    } else {
      context.partition.graph_partition_filename =
              context.partition.graph_filename;
    }
    context.partition.graph_partition_filename =
            context.partition.graph_partition_filename
            + ".part"
            + std::to_string(context.partition.k)
            + ".epsilon"
            + epsilon_str
            + ".seed"
            + std::to_string(context.partition.seed)
            + ".KaHyPar";
    context.partition.graph_community_filename =
            context.partition.graph_filename + ".community";

    if (context.partition.deterministic) {
      context.preprocessing.stable_construction_of_incident_edges = true;
    }
  }


  void parseIniToContext(Context& context, const std::string& ini_filename) {
    std::ifstream file(ini_filename.c_str());
    if (!file) {
      ERROR("Could not load context file at: " << ini_filename);
    }
    const int num_columns = 80;

    po::options_description general_options =
            createGeneralOptionsDescription(context, num_columns);
    po::options_description preprocessing_options =
            createPreprocessingOptionsDescription(context, num_columns);
    po::options_description coarsening_options =
            createCoarseningOptionsDescription(context, num_columns);
    po::options_description initial_paritioning_options =
            createInitialPartitioningOptionsDescription(context, num_columns);
    po::options_description refinement_options =
            createRefinementOptionsDescription(context, num_columns, false);
    po::options_description flow_options =
            createFlowRefinementOptionsDescription(context, num_columns, false);
#ifdef KAHYPAR_ENABLE_EXPERIMENTAL_FEATURES
    po::options_description sparsification_options =
    createSparsificationOptionsDescription(context, num_columns);
#endif
    po::options_description shared_memory_options =
            createSharedMemoryOptionsDescription(context, num_columns);

    po::variables_map cmd_vm;
    po::options_description ini_line_options;
    ini_line_options.add(general_options)
            .add(preprocessing_options)
            .add(coarsening_options)
            .add(initial_paritioning_options)
            .add(refinement_options)
            .add(flow_options)
#ifdef KAHYPAR_ENABLE_EXPERIMENTAL_FEATURES
                    .add(sparsification_options)
#endif
            .add(shared_memory_options);

    po::store(po::parse_config_file(file, ini_line_options, true), cmd_vm);
    po::notify(cmd_vm);

    if (context.partition.deterministic) {
      context.preprocessing.stable_construction_of_incident_edges = true;
    }
  }

}
