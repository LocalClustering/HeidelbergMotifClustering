/******************************************************************************
 * motif_clustering_graph.cpp 
 * Author - XXX
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
#include "algorithms/mqi.h"
#include "partition/uncoarsening/refinement/label_propagation_refinement/label_propagation_refinement.h"
/* #include "partition/uncoarsening/refinement/kway_motif_cond_refinement/kway_motif_cond_refinement.h" */
#include "partition/uncoarsening/refinement/refinement.h"

#include <memory>
#include <vector>

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
	mqi mqi_flow;

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
	double motif_conductance, updated_motif_conductance, current_min_motif_conductance;
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

		int index = 1;
		int lastLayerNodeCount = 0;
		std::vector<NodeID> map_model_original;
		map_model_original.push_back((*subgraph_map)[0]);
		//map_model_original.push_back((*subgraph_map)[0]); // this is the artificial node, thus values doesn't matter
		
		forall_nodes((*G_temp), node) {
			if(node == 0) {
				continue;    
			}
			if(lastLayerNodes[node] == true) {
				//G_temp->setPartitionIndex(node, 1);
				G_temp->setPartitionIndex(node, index);
				index++;
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

		forall_nodes(S, node) {
			if(lastLayerNodes[node] == true) {
				S.setPartitionIndex(node, 1);
			}
			else{
				S.setPartitionIndex(node, 0);
			}
		} endfor

		quality_metrics qm;
		motif_conductance = qm.local_conductance(S, 1, partition_config.triangle_count);
		
		current_min_motif_conductance = motif_conductance;
		updated_motif_conductance = current_min_motif_conductance;
		int numTries = 0;
		while (updated_motif_conductance <= current_min_motif_conductance && numTries < 1) {
			mqi_flow.mqi_improvement_ball(S, partition_config.fix_seed_node);
			updated_motif_conductance = qm.local_conductance(S, 1, partition_config.triangle_count);
			if(current_min_motif_conductance > updated_motif_conductance){
				current_min_motif_conductance = updated_motif_conductance;
				motif_conductance = updated_motif_conductance;
			}
			else{
				numTries++;
			}
			if (partition_config.print_cut) {
				std::cout << "cut \t\t"         << qm.edge_cut(S)                 << std::endl;
			}
		}
		
		std::cout << "Motif Conductance: " << motif_conductance << std::endl;

		if(motif_conductance < min_motif_conductance) { 
				min_motif_conductance = motif_conductance;
				opt_depth = bfsDepths[alpha];
				opt_imbalance = current_imbalance;
				NodeID community_size = 0;
				best_community.clear();
				forall_nodes(S, n) {
					if (S.getPartitionIndex(n) != 1) { 
						community_size++;
						best_community.push_back(map_model_original[n]);
					}
				} endfor
				size_of_final_cluster = community_size;
		}
		std::cout << "---------------------------------" << std::endl;
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

