/******************************************************************************
 * triangle_counter.cpp 
 * *
 * XXX.
 * XXX
 *****************************************************************************/

#include <argtable3.h>
#include <regex.h>
#include <string.h> 

#include "data_structure/graph_access.h"
#include "graph_io.h"
#include "macros_assertions.h"
#include "parse_parameters.h"
#include "partition/partition_config.h"
#include "quality_metrics.h"
#include "algorithms/bfs_depth.h"
#include "algorithms/triangle_listing.h"
#include "uncoarsening/refinement/quotient_graph_refinement/complete_boundary.h"

int main(int argn, char **argv) {

        PartitionConfig partition_config;
        std::string graph_filename;
	std::vector<NodeID> community;
	bfs_depth bfs;
	triangle_listing triangle;
	double motif_conductance;
	double current_imbalance;

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

        graph_access G;     
        graph_io::readGraphWeighted(G, graph_filename);

        G.set_partition_count(partition_config.k); 
 
        /* std::vector<PartitionID> input_partition; */
        /* if(partition_config.input_partition != "") { */
        /*         std::cout <<  "reading input partition" << std::endl; */
        /*         /1* graph_io::readPartition(G, partition_config.input_partition); *1/ */
        /*         graph_io::readCommunity(G, partition_config.input_partition, community, 1); */
        /* } else { */
        /*         std::cout <<  "Please specify an input partition using the --input_partition flag."  << std::endl; */
        /*         exit(0); */
        /* } */

	/* std::vector<NodeID> mapping(G.number_of_nodes(), -1); //stores subgraph -> graph mapping */
	/* std::vector<NodeID> *subgraph_map = NULL; */
	std::vector<bool> lastLayerNodes(G.number_of_nodes(), false); //true at index [i] if node i is in the last layer - subgraph size
	graph_access *G_temp;
	// Vectors for BFS
	/* std::vector<short int> level(G.number_of_nodes(), -1); */
	/* std::vector<bool> visited(G.number_of_nodes(), false); */
	/* std::vector<bool> touched(G.number_of_nodes(), false); */


        /* G_temp = bfs.runBFS_graph(G, community, 0, mapping, subgraph_map, lastLayerNodes, */
	    		/* level, visited, touched, false); // creating a ball up to chosen depths */

	/* long long num_triangles = triangle.triangle_run_graph(*G_temp, lastLayerNodes); // set edge weight to number of triangles that edge belongs to */
	std::cout << triangle.triangle_run_graph(G, lastLayerNodes) << "\n"; 

        /* NodeID subgraph_node_count = G_temp->number_of_nodes(); */

        //contract nodes 
        //set partition index 0 for seed node, 1 for last layer nodes, 2 onwards for rest
        /* int index = 2; */
        /* int lastLayerNodeCount = 0; */
        /* forall_nodes((*G_temp), node) { */
        /*         if(node == 0) { */
        /*             continue; */    
        /*         } */
        /*         if(lastLayerNodes[node] == true) { */
        /*                 G_temp->setPartitionIndex(node, 1); */
        /*                 lastLayerNodeCount++; */
        /*         } */
        /*         else{ */
        /*                 G_temp->setPartitionIndex(node, index); */
        /*                 index++; */
        /*         } */
        /* } endfor */
	/* G_temp->set_partition_count(index); */ 

        /* graph_access S; //contracted subgraph */
        /* complete_boundary boundary(G_temp); */
        /* boundary.fastComputeQuotientGraphRemoveZeroEdges(S,index); */
        /* /1* boundary.build(); *1/ */
        /* /1* boundary.getUnderlyingQuotientGraph(S); *1/ */
	/* S.setNodeWeight(1,1); */

        /* forall_nodes(S, node) { */
		/* S.setPartitionIndex(node, 0); */
        /* } endfor */
	/* S.setPartitionIndex(1, 1); */

        /* std::cout <<  "graph has " <<  G.number_of_nodes() <<  " nodes and " <<  G.number_of_edges() <<  " edges"  << std::endl; */
        /* quality_metrics qm; */

	/* motif_conductance = qm.local_conductance(S, S.getPartitionIndex(1), partition_config.triangle_count); */
        /* std::cout << "Best Result - Depth: XX Imbalance: XX Motif Conductance: " << motif_conductance << std::endl; */
        /* std::cout << "Size of cluster: " << community.size() << std::endl; */

        /* std::cout << "cut \t\t"         << qm.edge_cut(G)                 << std::endl; */
        /* std::cout << "no boundary vertices \t\t" << qm.boundary_nodes(G)           << std::endl; */
        /* std::cout << "balance \t"       << qm.balance(G)                  << std::endl; */
        /* std::cout << "balance based on edges \t"       << qm.balance_edges(G)                  << std::endl; */
        /* std::cout << "max comm vol \t"  << qm.max_communication_volume(G) << std::endl; */
        /* std::cout << "min comm vol \t"  << qm.min_communication_volume(G) << std::endl; */
        /* std::cout << "total comm vol \t"  << qm.total_communication_volume(G) << std::endl; */
}

