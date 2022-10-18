/******************************************************************************
 * motif_clustering.cpp 
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

#include <memory>
#include <vector>

#include "libkahypar.h"

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

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))

using namespace std;

/*
* This program detects motif clusterings in the input graph.
* Output - Nodes in the motif cluster + motif conductance score
*/

void write_motif_clustering(std::vector<NodeID> &partition, std::string filename){
        std::ofstream f(filename.c_str());
        std::cout << "Writing motif clustering to " << filename << " ... " << std::endl;

        for(const auto&j : partition){
                f << j << std::endl;
        }

        f.close();
}

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

        NodeID seed_node = 0;
        int no_threads = 1;
        bool keepParallelNets = false;
        bool fixNodes = false;
        std::vector<short int> bfsDepths = partition_config.bfsDepths; //choice of buffer size depths
	seed_node = (NodeID)partition_config.seed_node - 1;
        no_threads = partition_config.no_threads;
        keepParallelNets = partition_config.keep_parallel_nets;
        fixNodes = partition_config.fix_nodes;

        std::cout<< "Seed Node:" << seed_node << endl;
        std::cout<< "Number of Threads:" << no_threads << endl;

        graph_access G;     

        kahypar_context_t* context = kahypar_context_new();
        kahypar_configure_context_from_file(context, "./extern/kahypar/config/cut_rKaHyPar_sea20.ini"); 

        double enum_time = 0;
        timer t2;
        timer t;
	
	// Load graph
        kahip::mmap_io::graph_from_metis_file_light(G, graph_filename);
        std::cout << "io time: " << t.elapsed()  << std::endl;

        std::cout <<  "Graph has " <<  G.number_of_nodes() <<  " nodes and " <<  G.number_of_edges() <<  " edges"  << std::endl;

        std::cout << "********************** perform Motif Clustering **********************" << std::endl;

        t.restart();

        //execute the operation alpha times, where alpha is the number of buffer sizes
        bfs_depth bfs;
        triangle_listing triangle;

        int size_of_final_cluster = 0;
	bool feasible_solution_found = false;
        double min_motif_conductance = 100;
        short int opt_depth = 0;
        double opt_imbalance = 0;
	bool was_over_100 = false;
        std::vector<kahypar_partition_id_t> opt_partition;
        std::vector<NodeID> opt_mapping; //stores optimal subgraph -> graph mapping
        std::vector <NodeID> *opt_mapping2 = new std::vector <NodeID>(); //stores optimal hypergraph -> graph mapping
        std::vector<std::vector<NodeID>> *triangles; //stores list of triangles
        std::vector <NodeID> *mapping2; //stores hypergraph -> graph mapping
        std::vector<NodeID> mapping(G.number_of_nodes(), -1); //stores subgraph -> graph mapping
        std::vector<NodeID> *hypergraph_map = NULL;
        std::vector<bool> lastLayerNodes; //true at index [i] if node i is in the last layer - subgraph size
        graph_access *G_temp;
	bool del_mapping2 = false;

	// Vectors for BFS
        std::vector<short int> level(G.number_of_nodes(), -1);
        std::vector<bool> visited(G.number_of_nodes(), false);
        std::vector<bool> touched(G.number_of_nodes(), false);

        bool noMotif = false; //to test whether seed node belongs to any motif

        for(int alpha = 0; alpha < bfsDepths.size() && (alpha==0 || t.elapsed() < partition_config.repetition_timelimit); alpha++){
		if (hypergraph_map != NULL) {
			for(const auto & j: *hypergraph_map) {
                                lastLayerNodes[mapping[j]] = false;
				mapping[j]= -1;
			}
			delete hypergraph_map;
		}
                if(del_mapping2) delete mapping2;
                
		bool force_over_100 = (alpha==bfsDepths.size()-1 && !was_over_100);
                G_temp = bfs.runBFS(G, seed_node, bfsDepths[alpha], mapping, hypergraph_map, lastLayerNodes,
					level, visited, touched, force_over_100); // creating a ball up to chosen depths
                std::cout << "Depth = " << bfsDepths[alpha] << std::endl;
                //std::cout <<  "Subgraph has " <<  G_temp->number_of_nodes() <<  " nodes and " <<  G_temp->number_of_edges() <<  " edges"  << std::endl;
                std::cout << "Nodes in Enumeration = " << G_temp->number_of_nodes() << std::endl; 
                
                NodeID hypergraph_node_count;
                
                t2.restart();
                //std::cout << "here" << std::endl;
                triangles = triangle.triangle_run(*G_temp, seed_node, bfsDepths[alpha], lastLayerNodes, hypergraph_node_count, mapping2); // create hypergraph of motif hyperedges from subgraphs S
                enum_time += t2.elapsed();

                del_mapping2 = true;

		if (hypergraph_node_count > 100) was_over_100 = true;

                std::cout << "Nodes in Hypergraph Model = " << hypergraph_node_count << std::endl;
                /* std::cout << "Hypergraph has " << hypergraph_node_count << " nodes and " << triangles->size() << " edges." << std::endl; */

                //building hypergraph as defined in hmetis manual page 14
                std::vector<size_t> eptr;
                std::vector<kahypar_hyperedge_id_t> eind;
                std::vector<kahypar_hyperedge_weight_t> contracted_hyperedge_weights;
                int hyperedge_position = 0;
                eptr.push_back(hyperedge_position);

		unsigned triangle_size = triangles->size();            

		if (hypergraph_node_count <= 1 || triangle_size == 0) {
			std::cout << "The ball contains no motif occurrences." << std::endl;
			std::cout << "---------------------------------" << std::endl;
			continue;
		} else {
			std::cout << "Enumerated Motifs: " << triangle_size << std::endl;
		}
                
                if(!keepParallelNets){
                        std::sort(triangles->begin(), triangles->end());
                }

                int hyperedgeIndex = 0;
                std::vector<int> hypernode_degrees(hypergraph_node_count,0);
                //add first hyperedge to eind
                for(auto &j : (*triangles)[0]) {
                        eind.push_back(j);
                        hyperedge_position++;
                        hypernode_degrees[j]++;
                }
                eptr.push_back(hyperedge_position);
                int baseWeight = 1;
                contracted_hyperedge_weights.push_back(baseWeight);
                //add remaining hyperedges to eind if they are different from previous
                //if same as previous, add +1 to hyperedge weight
                for(int i = 1; i < triangle_size; i++) {
                        if(!keepParallelNets){ //contracting parallel nets
                                if((*triangles)[i]==(*triangles)[i-1]){
                                baseWeight++;
                                contracted_hyperedge_weights[hyperedgeIndex] = baseWeight;
                                }
                                else{
                                        for(auto&j : (*triangles)[i]){
                                                eind.push_back(j);
                                                hyperedge_position++;
                                        }
                                        eptr.push_back(hyperedge_position);
                                        baseWeight = 1;
                                        contracted_hyperedge_weights.push_back(baseWeight);
                                        hyperedgeIndex++;
                                }
                        }
                        else{ //retaining all nets
                                for(auto&j : (*triangles)[i]){
                                        eind.push_back(j);
                                        hyperedge_position++;
                                }
                                eptr.push_back(hyperedge_position);
                                baseWeight = 1;
                                contracted_hyperedge_weights.push_back(baseWeight);
                                hyperedgeIndex++;
                        }

                        for(auto&j : (*triangles)[i]){
                                hypernode_degrees[j]++;
                        }
                }

                //eptr[eptr.size()-1] = eptr[eptr.size()-1] - 1;

                if(alpha == 0) {
                        bool triangleFound = false;
                        for(auto &j : eind) {
                                if(j == (*mapping2)[mapping[seed_node]]){
                                        triangleFound = true;
                                        break;
                                }
                        }
                        if(triangleFound == false){
                                noMotif = true;
                                std::cout << "The seed node was not part of the motif." << std::endl;
				if (hypergraph_node_count <= 3) {
					std::cout << "The hypergraph model is too small." << std::endl;
					continue;
				}
                        }
                }

                delete G_temp;
                delete triangles;    

                const kahypar_hypernode_id_t num_vertices = hypergraph_node_count;
                //const kahypar_hyperedge_id_t num_hyperedges = triangles->size(); OLD VERSION
                const kahypar_hyperedge_id_t num_hyperedges = contracted_hyperedge_weights.size();
		std::vector<kahypar_partition_id_t> partition(num_vertices, -1);
		std::cout << "Nets in Model: " << num_hyperedges << std::endl;

                //run kahypar with chosen imbalance values
                for(int beta = 0; beta < partition_config.beta && (beta==0 || t.elapsed() < partition_config.repetition_timelimit); beta++) {

                        const kahypar_partition_id_t k = 2;
                        kahypar_hyperedge_weight_t objective = 0;
                        
                        double motif_conductance = 0;
        
                        NodeID min_volume; 
                        const double current_imbalance = random_functions::nextDouble(0.03,0.97);
                        
                        // Partition Hypergraph
                        
                        kahypar_partition_MC(num_vertices, num_hyperedges,
                                current_imbalance, k,
                                /*vertex_weights */ nullptr, const_cast<const kahypar_hyperedge_weight_t*>(&(contracted_hyperedge_weights[0])),
                                const_cast<const size_t*>(&(eptr[0])), const_cast<const kahypar_hyperedge_id_t*>(&(eind[0])),
                                &objective, context, partition.data(),
                                noMotif, fixNodes);
                        
                        // compute motif conductance

                        int lastNode = partition.size()-1;
                        int oppositeBlock = partition[lastNode]; //partID corresponding to contracted node
                        int clusterBlock = 1 - oppositeBlock; //partID corresponding to detected cluster

                        std::vector<NodeID> volumes(2,0);
                        for(int i = 0; i < partition.size(); i++){
                                volumes[partition[i]] += hypernode_degrees[i];
                        }
                        min_volume = volumes[clusterBlock];

                        int hypergraph_cut = 0;
                        std::vector<bool> blockFound(2);
                        
                        int hyperedgeCount = 0;
                        for(int i = 0; i < eind.size(); i++) {
                                if(i == eptr[hyperedgeCount+1]){
                                        //std::cout<<std::endl;
                                        if(blockFound[0] && blockFound[1]) {
                                                hypergraph_cut += contracted_hyperedge_weights[hyperedgeCount]; 
                                        }
                                        blockFound[0] = false;
                                        blockFound[1] = false;
                                        hyperedgeCount++;
                                }
                                //std::cout << eind[i] << " ";
                                blockFound[partition[eind[i]]] = true;
                        }
                        // for last hyperedge
                        if(blockFound[0] && blockFound[1]) {
                                        hypergraph_cut += contracted_hyperedge_weights[hyperedgeCount]; 
                        }
                        blockFound[0] = false;
                        blockFound[1] = false;
                        hyperedgeCount++;

                        //std::cout << "Final hyperedge count: " << hyperedgeCount << " Actual hyperedgecount " << num_hyperedges << std::endl;
                        //std::cout << "Cut computed: " << hypergraph_cut << " Cut Obtained " << objective << std::endl;
                        if(hypergraph_cut!=objective){
                                std::cout << "Cut computing error detected." << std::endl;
                        }

			if (partition_config.triangle_count != UNDEFINED_TRIANGLE_COUNT) min_volume = MIN(min_volume,3*partition_config.triangle_count-min_volume);
                        motif_conductance = (double)hypergraph_cut/(double)min_volume;

                        std::cout << "Imbalance: " << current_imbalance << " " << "Motif Conductance: " << motif_conductance << std::endl;
			if (partition_config.print_cut) {
				std::cout << "cut \t\t"         << hypergraph_cut                 << std::endl;
			}

                        if(motif_conductance < min_motif_conductance) {
				feasible_solution_found = true;
                                min_motif_conductance = motif_conductance;
                                opt_depth = bfsDepths[alpha];
                                opt_imbalance = current_imbalance;
                                opt_partition = partition;
                                if (partition_config.write_cluster) {
                                        opt_mapping = mapping;
                                }
				if(del_mapping2) delete opt_mapping2;
                                opt_mapping2 = mapping2;
                                del_mapping2 = false;
                        }
                        
                }

                std::cout << "---------------------------------" << std::endl;
                
        }

        kahypar_context_free(context);

	if (!feasible_solution_found) {
		opt_depth = -1;
		opt_imbalance = -1;
		min_motif_conductance = -1;
		size_of_final_cluster = -1;
	}

        std::cout << "Total Clustering Time: " << t.elapsed()  << std::endl;
        std::cout <<"Triangle Enumeration Time = " << enum_time << std::endl;
        std::cout << "Best Result - Depth: " << opt_depth << " Imbalance: " << opt_imbalance << " Motif Conductance: " << min_motif_conductance << std::endl;

        NodeID oppositeID;
        NodeID clusterID;
	if (feasible_solution_found) {
		oppositeID = opt_partition[opt_partition.size()-1];
		clusterID = 1 - oppositeID;
		for(int i = 0; i < opt_partition.size(); i++) {
			if(opt_partition[i]==clusterID) {
				size_of_final_cluster++;
			}
		}

                if(noMotif == true){
                        size_of_final_cluster++;
                }

		if (partition_config.write_cluster) {
			std::vector<NodeID> reverse_mapping2(G.number_of_nodes(), -1);
			for(int i = 0; i < opt_mapping2->size(); i++){
				if((*opt_mapping2)[i]!=-1){
					reverse_mapping2[(*opt_mapping2)[i]]=i;
				}
			}

			std::vector<NodeID> reverse_mapping1(G.number_of_nodes(), -1);
			for(int i = 0; i < opt_mapping.size(); i++){
				if(opt_mapping[i]!=-1){
					reverse_mapping1[opt_mapping[i]]=i;
				}
			}

                        std::vector<NodeID> best_community; //to store output community
			/* std::vector<NodeID> final_partition(G.number_of_nodes(), oppositeID); */


			for(int i = 0; i < opt_partition.size(); i++) {
				if(opt_partition[i]==clusterID) {
					//std::cout << "Vertex: " << i << " = " << reverse_mapping2[i] << " = " << reverse_mapping1[reverse_mapping2[i]] << std::endl;
					//final_partition[reverse_mapping1[reverse_mapping2[i]]] = clusterID;
                                        best_community.push_back(reverse_mapping1[reverse_mapping2[i]]);
				}
			}

			if(noMotif == true){
				//final_partition[seed_node] = clusterID;
                                best_community.insert(best_community.begin(), seed_node);
			}

                        /*
			if(final_partition[seed_node] == opt_partition[opt_partition.size()-1]){
				std::cout << "Error in code!" << std::endl;
			}
                        */

			//std::cout << "Seed node " << seed_node << " was mapped to " << opt_mapping[seed_node] << " which was then mapped to " << opt_mapping2[opt_mapping[seed_node]] << std::endl;

			std::stringstream filename;
			//filename << seed_node+1 << "_" << "motifclustering";
			//write_motif_clustering(final_partition, filename.str());
                        if(!partition_config.filename_output.compare("")) {
                                filename << "tmppartition" << partition_config.k;
                        } else {
                                filename << partition_config.filename_output;
                        }
                        graph_io::writeCommunity(best_community, filename.str());
		}
	}
        std::cout << "Size of cluster: " << size_of_final_cluster << std::endl;

	if (feasible_solution_found) delete opt_mapping2;

	return 0;
}
