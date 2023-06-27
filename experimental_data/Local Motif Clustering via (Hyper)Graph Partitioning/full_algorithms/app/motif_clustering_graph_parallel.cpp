/******************************************************************************
 * app/motif_clustering_graph_parallel.cpp 
 *****************************************************************************/

#include <argtable3.h>
#include <iostream>
#include <math.h>
#include <regex.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <tuple>
#include <fstream>

#include "algorithms/bfs_depth.h"
#include "algorithms/triangle_listing.h"
#include "partition/uncoarsening/refinement/label_propagation_refinement/label_propagation_refinement.h"
/* #include "partition/uncoarsening/refinement/kway_motif_cond_refinement/kway_motif_cond_refinement.h" */
#include "partition/uncoarsening/refinement/refinement.h"

#include <memory>
#include <vector>

#include "libkaminpar.h"

#include "balance_configuration.h"
#include "data_structure/graph_access.h"
#include "graph_io.h"
#include "macros_assertions.h"
#include "mapping/mapping_algorithms.h"
#include "mmap_graph_io.h"
#include "parse_parameters.h"
#include "partition/graph_partitioner.h"
#include "partition/partition_config.h"
#include "partition/uncoarsening/refinement/cycle_improvements/cycle_refinement.h"
#include "quality_metrics.h"
#include "random_functions.h"
#include "timer.h"

/* using namespace std; */


/*
 * This program detects motif clusterings in the input graph.
 * Output - Nodes in the motif cluster + motif conductance score
 */

int main(int argn, char **argv)
{
	// loading parse parameters
	PartitionConfig partition_config;
	std::string graph_filename;

	bool is_graph_weighted = false;
	bool suppress_output   = false;
	bool recursive         = false;

	int ret_code = parse_parameters(argn, argv, 
			partition_config, 
			graph_filename, 
			is_graph_weighted, 
			suppress_output, recursive); 

	if(ret_code) {
		return 0;
	}
	random_functions::setSeed(partition_config.seed+partition_config.seed_node);

	std::ofstream ofs;
	ofs.open("/dev/null");
	if(suppress_output) {
		std::cout.rdbuf(ofs.rdbuf()); 
	}

	partition_config.LogDump(stdout);
	partition_config.k = 2;

	NodeID seed_node = 0;
	std::vector<short int> bfsDepths = partition_config.bfsDepths; //choice of buffer size depths
	seed_node = (NodeID)partition_config.seed_node - 1;

	std::cout<< "Seed Node:" << seed_node << std::endl;

	graph_access G;     

	double enum_time = 0;
	timer t2;
	timer t;


	// Load graph
	kahip::mmap_io::graph_from_metis_file(G, graph_filename);
	std::cout << "io time: " << t.elapsed()  << std::endl;

	std::cout <<  "Graph has " <<  G.number_of_nodes() <<  " nodes and " <<  G.number_of_edges() <<  " edges"  << std::endl;

	std::cout << "********************** perform Motif Clustering **********************" << std::endl;

	t.restart();

	//execute the operation alpha times, where alpha is the number of buffer sizes
	bfs_depth bfs;
	triangle_listing triangle;

	int size_of_final_cluster = 0;
	//bool feasible_solution_found = false;
	double min_motif_conductance = 100;
	short int opt_depth = 0;
	double opt_imbalance = 1;
	bool was_over_100 = false;
	//std::vector<kahypar_partition_id_t> opt_partition;
	//std::vector<NodeID> opt_mapping; //stores optimal subgraph -> graph mapping
	//std::vector <NodeID> *opt_mapping2 = new std::vector <NodeID>(); //stores optimal hypergraph -> graph mapping
	//std::vector<std::vector<NodeID>> *triangles; //stores list of triangles
	std::vector<NodeID> mapping(G.number_of_nodes(), -1); //stores graph -> subgraph mapping
	std::vector<NodeID> *subgraph_map = NULL;
	std::vector<bool> lastLayerNodes; //true at index [i] if node i is in the last layer - subgraph size
	graph_access *G_temp=NULL;
	double motif_conductance;
	double current_imbalance;
	std::vector<NodeID> best_community;

	// Vectors for BFS
	std::vector<short int> level(G.number_of_nodes(), -1);
	std::vector<bool> visited(G.number_of_nodes(), false);
	std::vector<bool> touched(G.number_of_nodes(), false);

	//bool noMotif = false; //to test whether seed node belongs to any motif

	for(int alpha = 0; alpha < bfsDepths.size() && (alpha==0 || t.elapsed() < partition_config.repetition_timelimit); alpha++){
/* std::cout << "Point 1\n"; */
		if (G_temp != NULL) delete G_temp;
/* std::cout << "Point a\n"; */
		if (subgraph_map != NULL) {
			for(const auto & j: *subgraph_map) {
				lastLayerNodes[mapping[j]] = false;
				mapping[j]= -1;
			}
			delete subgraph_map;
		}
/* std::cout << "Point 2\n"; */

		bool force_over_100 = (alpha==bfsDepths.size()-1 && !was_over_100);
		std::vector<NodeID> seed_nodes = {seed_node};
/* std::cout << "Point 3\n"; */
		G_temp = bfs.runBFS_graph(G, seed_nodes, bfsDepths[alpha], mapping, subgraph_map, lastLayerNodes,
				level, visited, touched, force_over_100); // creating a ball up to chosen depths
		std::cout << "Depth = " << bfsDepths[alpha] << std::endl;
		std::cout << "Nodes in Enumeration = " << G_temp->number_of_nodes() << std::endl; 

		NodeID subgraph_node_count;

		t2.restart();
		long long num_triangles = triangle.triangle_run_graph(*G_temp, lastLayerNodes); // set edge weight to number of triangles that edge belongs to
		enum_time += t2.elapsed();

		subgraph_node_count = G_temp->number_of_nodes();
		if (subgraph_node_count > 100) was_over_100 = true;

		int index = 2;
		int lastLayerNodeCount = 0;
		std::vector<NodeID> map_model_original;
		map_model_original.push_back((*subgraph_map)[0]);
		map_model_original.push_back((*subgraph_map)[0]); // this is the artificial node, thus values doesn't matter
		forall_nodes((*G_temp), node) {
			if(node == 0) {
				continue;    
			}
			if(lastLayerNodes[node] == true) {
				G_temp->setPartitionIndex(node, 1);
				lastLayerNodeCount++;
			}
			else{
				G_temp->setPartitionIndex(node, index);
				index++;
				map_model_original.push_back((*subgraph_map)[node]);
			}
		} endfor
		G_temp->set_partition_count(index); 

		graph_access S; //contracted subgraph
		complete_boundary boundary(G_temp);
		boundary.fastComputeQuotientGraphRemoveZeroEdges(S,index);
		S.setNodeWeight(1,1);

		std::cout << "Nodes in Model = " << S.number_of_nodes() << std::endl;
		std::cout << "Enumerated Motifs: " << num_triangles << std::endl; 
		std::cout << "Edges in Model: " << S.number_of_edges() << std::endl;



		std::vector<libkaminpar::EdgeID> nodes;
		nodes.push_back(0);
		forall_nodes(S,n) { 
			nodes.push_back(S.get_first_invalid_edge(n));
		} endfor
		std::vector<libkaminpar::NodeID> edges;
		forall_edges(S,e) { 
			edges.push_back(S.getEdgeTarget(e));
		} endfor

		libkaminpar::Partitioner partitioner = libkaminpar::PartitionerBuilder
			::from_adjacency_array(nodes.size() - 1, nodes.data(), edges.data()) 
			.create();
		
		partitioner.set_option("--threads", std::to_string(partition_config.no_threads) );    // number of cores



		//partition subgraph with chosen imbalance values
		for(int beta = 0; beta < partition_config.beta && (beta==0 || t.elapsed() < partition_config.repetition_timelimit); beta++) {
			NodeID min_volume;
			S.set_partition_count(2); 
			balance_configuration bc;
			partition_config.imbalance = random_functions::nextInt(0,99);
			bc.configurate_balance(partition_config, S);


			partitioner.set_option("--epsilon", std::to_string((double)partition_config.imbalance/100) ); // allowed imbalance

			std::cout.setstate(std::ios_base::failbit);
			auto partition = partitioner.partition(2); // compute 16-way partition
			std::cout.clear();

			forall_nodes(S,n) { 
				S.setPartitionIndex(n,partition[n]);
			} endfor


			/* graph_partitioner partitioner; */
			quality_metrics qm;

			/* partitioner.perform_partitioning(partition_config, S); */

			S.set_partition_count(2); 
			S.setPartitionIndex(0, 1-S.getPartitionIndex(1));

			std::vector<NodeID> fixed_nodes = {0,1};

			if (partition_config.label_prop_ls) {
				if (partition_config.print_cut) {
					motif_conductance = qm.local_conductance(S, S.getPartitionIndex(1), partition_config.triangle_count);
					current_imbalance = qm.balance(S)-1;
					std::cout << "BEFORE Imbalance: " << current_imbalance << " " << "Motif Conductance: " << motif_conductance << std::endl;
					std::cout << "cut \t\t"         << qm.edge_cut(S)                 << std::endl;
				}
				label_propagation_refinement refine;
				refine.perform_refinement_conductance(partition_config, fixed_nodes, S, S.getPartitionIndex(1));
			}

			/* if (partition_config.fm_ls) { */
			/* 	if (partition_config.print_cut) { */
			/* 		motif_conductance = qm.local_conductance(S, S.getPartitionIndex(1), partition_config.triangle_count); */
			/* 		current_imbalance = qm.balance(S)-1; */
			/* 		std::cout << "MIDDLE Imbalance: " << current_imbalance << " " << "Motif Conductance: " << motif_conductance << std::endl; */
			/* 		std::cout << "cut \t\t"         << qm.edge_cut(S)                 << std::endl; */
			/* 	} */
			/* 	complete_boundary my_boundary(&S); */
			/* 	my_boundary.build(); */
			/* 	kway_motif_cond_refinement refine; */
			/* 	refine.perform_refinement(partition_config, */ 
			/* 				  S, */ 
			/* 				  my_boundary); */
			/* } */

			motif_conductance = qm.local_conductance(S, S.getPartitionIndex(1), partition_config.triangle_count);
			current_imbalance = qm.balance(S)-1;

			std::cout << "Imbalance: " << current_imbalance << " " << "Motif Conductance: " << motif_conductance << std::endl;
			if (partition_config.print_cut) {
				std::cout << "cut \t\t"         << qm.edge_cut(S)                 << std::endl;
			}

			if(motif_conductance < min_motif_conductance) { //[TO DO]
				min_motif_conductance = motif_conductance;
				opt_depth = bfsDepths[alpha];
				opt_imbalance = current_imbalance;
				NodeID community_size = 0;
				best_community.clear();
				forall_nodes(S, n) {
					if (S.getPartitionIndex(n) != S.getPartitionIndex(1)) { 
						community_size++;
						best_community.push_back(map_model_original[n]);
					}
				} endfor
				size_of_final_cluster = community_size;
			}


		}

		std::cout << "---------------------------------" << std::endl;
/* std::cout << "Point 0\n"; */
	}

	std::cout << "Total Clustering Time: " << t.elapsed()  << std::endl;
	std::cout << "Triangle Enumeration Time = " << enum_time << std::endl;
	std::cout << "Best Result - Depth: " << opt_depth << " Imbalance: " << opt_imbalance << " Motif Conductance: " << min_motif_conductance << std::endl;
	std::cout << "Size of cluster: " << size_of_final_cluster << std::endl;


	if (partition_config.write_cluster) {
		// write the partition to the disc 
		std::stringstream filename;
		if(!partition_config.filename_output.compare("")) {
			filename << "tmppartition" << partition_config.k;
		} else {
			filename << partition_config.filename_output;
		}
		graph_io::writeCommunity(best_community, filename.str());
	}

	return 0;
}

