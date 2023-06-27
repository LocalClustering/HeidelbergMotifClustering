/******************************************************************************
 * label_propagation_refinement.cpp 
 * *
 * 
 * XXX <XXX.phone@gmail.com>
 *****************************************************************************/


#include "label_propagation_refinement.h"
#include "partition/coarsening/clustering/node_ordering.h"
#include "tools/random_functions.h"

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))

label_propagation_refinement::label_propagation_refinement() {
                
}

label_propagation_refinement::~label_propagation_refinement() {
                
}

EdgeWeight label_propagation_refinement::perform_refinement(PartitionConfig & partition_config, 
                                                            graph_access & G, 
                                                            complete_boundary & boundary) {
        NodeWeight block_upperbound = partition_config.upper_bound_partition;

        // in this case the _matching paramter is not used 
        // coarse_mappng stores cluster id and the mapping (it is identical)
        std::vector<PartitionID> hash_map(partition_config.k,0);
        std::vector<NodeID> permutation(G.number_of_nodes());
        std::vector<NodeWeight> cluster_sizes(partition_config.k, 0);

        node_ordering n_ordering;
        n_ordering.order_nodes(partition_config, G, permutation);

        std::queue< NodeID > * Q             = new std::queue< NodeID >();
        std::queue< NodeID > * next_Q        = new std::queue< NodeID >();
        std::vector<bool> * Q_contained      = new std::vector<bool>(G.number_of_nodes(), false);
        std::vector<bool> * next_Q_contained = new std::vector<bool> (G.number_of_nodes(), false);
        forall_nodes(G, node) {
                cluster_sizes[G.getPartitionIndex(node)] += G.getNodeWeight(node);
                Q->push(permutation[node]);
        } endfor

        for( int j = 0; j < partition_config.label_iterations_refinement; j++) {
                unsigned int change_counter = 0;
                while( !Q->empty() ) {
                        NodeID node = Q->front();
                        Q->pop();
                        (*Q_contained)[node] = false;

                        //now move the node to the cluster that is most common in the neighborhood
                        forall_out_edges(G, e, node) {
                                NodeID target = G.getEdgeTarget(e);
                                hash_map[G.getPartitionIndex(target)]+=G.getEdgeWeight(e);
                                //std::cout <<  "curblock " <<  G.getPartitionIndex(target)  << std::endl;
                        } endfor

                        //second sweep for finding max and resetting array
                        PartitionID max_block = G.getPartitionIndex(node);
                        PartitionID my_block  = G.getPartitionIndex(node);

                        PartitionID max_value = 0;
                        forall_out_edges(G, e, node) {
                                NodeID target             = G.getEdgeTarget(e);
                                PartitionID cur_block     = G.getPartitionIndex(target);
                                PartitionID cur_value     = hash_map[cur_block];
                                if((cur_value > max_value  || (cur_value == max_value && random_functions::nextBool())) 
                                && (cluster_sizes[cur_block] + G.getNodeWeight(node) < block_upperbound || (cur_block == my_block && cluster_sizes[my_block] <= partition_config.upper_bound_partition)))
                                {
                                        max_value = cur_value;
                                        max_block = cur_block;
                                }

                                hash_map[cur_block] = 0;
                        } endfor

                        cluster_sizes[G.getPartitionIndex(node)]  -= G.getNodeWeight(node);
                        cluster_sizes[max_block]         += G.getNodeWeight(node);
                        bool changed_label                = G.getPartitionIndex(node) != max_block; 
                        change_counter                   += changed_label;
                        G.setPartitionIndex(node, max_block);
                        //std::cout <<  "maxblock " <<  max_block  << std::endl;

                        if(changed_label) {
                                forall_out_edges(G, e, node) {
                                        NodeID target = G.getEdgeTarget(e);
                                        if(!(*next_Q_contained)[target]) {
                                                next_Q->push(target);
                                                (*next_Q_contained)[target] = true;
                                        } 
                                } endfor
                        }
                } 

                std::swap( Q, next_Q);
                std::swap( Q_contained, next_Q_contained);

        }
        

        delete Q;
        delete next_Q;
        delete Q_contained;
        delete next_Q_contained;


        // in this case the _matching paramter is not used 
        // coarse_mappng stores cluster id and the mapping (it is identical)
        //std::vector<PartitionID> hash_map(G.number_of_nodes(),0);
        //std::vector<NodeID> permutation(G.number_of_nodes());
        //std::vector<NodeWeight> cluster_sizes(partition_config.k,0);

        //forall_nodes(G, node) {
                //cluster_sizes[G.getPartitionIndex(node)] += G.getNodeWeight(node);
        //} endfor
        
        //random_functions::permutate_vector_fast(permutation, true);
        //NodeWeight block_upperbound = partition_config.upper_bound_partition;

        //for( int j = 0; j < partition_config.label_iterations; j++) {
                //forall_nodes(G, i) {
                        //NodeID node = permutation[i];
                        ////move the node to the cluster that is most common in the neighborhood

                        //forall_out_edges(G, e, node) {
                                //NodeID target = G.getEdgeTarget(e);
                                //hash_map[G.getPartitionIndex(target)]+=G.getEdgeWeight(e);
                        //} endfor

                        ////second sweep for finding max and resetting array
                        //PartitionID max_block = G.getPartitionIndex(node);
                        //PartitionID my_block  = G.getPartitionIndex(node);

                        //PartitionID max_value = 0;
                        //forall_out_edges(G, e, node) {
                                //NodeID target             = G.getEdgeTarget(e);
                                //PartitionID cur_block     = G.getPartitionIndex(target);
                                //PartitionID cur_value     = hash_map[cur_block];
                                //if((cur_value > max_value  || (cur_value == max_value && random_functions::nextBool())) 
                                //&& (cluster_sizes[cur_block] + G.getNodeWeight(node) < block_upperbound || cur_block == my_block))
                                //{
                                        //max_value = cur_value;
                                        //max_block = cur_block;
                                //}

                                //hash_map[cur_block] = 0;
                        //} endfor
                        //cluster_sizes[G.getPartitionIndex(node)] -= G.getNodeWeight(node);
                        //cluster_sizes[max_block] += G.getNodeWeight(node);
                        //G.setPartitionIndex(node,max_block);
                //} endfor
        //}
        
        return 0;

}

EdgeWeight label_propagation_refinement::perform_refinement_conductance(PartitionConfig & partition_config, 
				std::vector<NodeID>& fixed_nodes, graph_access & G, PartitionID block_artif_node) {
        /* NodeWeight block_upperbound = partition_config.upper_bound_partition; */

        // in this case the _matching paramter is not used 
        // coarse_mappng stores cluster id and the mapping (it is identical)
        /* std::vector<PartitionID> hash_map(partition_config.k,0); */
        std::vector<NodeID> permutation(G.number_of_nodes());
        /* std::vector<NodeWeight> cluster_sizes(partition_config.k, 0); */
        /* NodeWeight cluster_size = 0; */
        EdgeWeight edgeCut = 0;
	long long volume = 0;

        node_ordering n_ordering;
        n_ordering.order_nodes(partition_config, G, permutation);

        std::queue< NodeID > * Q             = new std::queue< NodeID >();
        std::queue< NodeID > * next_Q        = new std::queue< NodeID >();
        std::vector<bool> * Q_contained      = new std::vector<bool>(G.number_of_nodes(), false);
        std::vector<bool> * next_Q_contained = new std::vector<bool> (G.number_of_nodes(), false);
        forall_nodes(G, node) {
                PartitionID partitionIDSource = G.getPartitionIndex(node);
                /* cluster_sizes[G.getPartitionIndex(node)] += G.getNodeWeight(node); */
		/* if (partitionIDSource != block_artif_node) cluster_size += G.getNodeWeight(node); */
                Q->push(permutation[node]);

                forall_out_edges(G, e, node) {
                        NodeID targetNode = G.getEdgeTarget(e);
                        PartitionID partitionIDTarget = G.getPartitionIndex(targetNode);

                        if (partitionIDSource != partitionIDTarget) edgeCut += G.getEdgeWeight(e);
			if (partitionIDSource != block_artif_node) volume += (long long)G.getEdgeWeight(e);
                } endfor 
        } endfor
	edgeCut /= 2;

        for( int j = 0; j < partition_config.label_iterations_refinement; j++) {
                /* unsigned int change_counter = 0; */
                while( !Q->empty() ) {
                        NodeID node = Q->front();
                        Q->pop();
                        (*Q_contained)[node] = false;
			bool skip_node = false;

			for (auto &fixed : fixed_nodes) {
				if (node == fixed) {
					skip_node = true;
					break;
				}
			}
			if (skip_node) continue;

                        /* PartitionID max_block = G.getPartitionIndex(node); */
                        PartitionID partitionIDSource = G.getPartitionIndex(node);
                        PartitionID partitionIDTarget = 1-partitionIDSource;

                        //now move the node to the cluster that is most common in the neighborhood
			long long delta_degree = 0;
			EdgeWeight delta_cut = 0;
                        forall_out_edges(G, e, node) {
                                NodeID target = G.getEdgeTarget(e);
				if (partitionIDSource == G.getPartitionIndex(target)) delta_cut += G.getEdgeWeight(e);
				else delta_cut -= G.getEdgeWeight(e);
				if (partitionIDSource == block_artif_node) delta_degree += (long long) G.getEdgeWeight(e);
				else delta_degree -= (long long) G.getEdgeWeight(e);
				/* delta_degree += (long long) G.getEdgeWeight(e); */
                                /* hash_map[G.getPartitionIndex(target)]+=G.getEdgeWeight(e); */
                        } endfor

			if (volume+delta_degree == 0) continue; // moving node leads to infeasible conductance 

			double new_conductance = (edgeCut+delta_cut)/(double)(volume+delta_degree);
			double best_conductance = (edgeCut)/(double)(volume);
			if (partition_config.triangle_count != UNDEFINED_TRIANGLE_COUNT) {
				new_conductance = (edgeCut+delta_cut)/(double)MIN(volume+delta_degree,6*partition_config.triangle_count-volume-delta_degree);
				best_conductance = (edgeCut)/(double)MIN(volume,6*partition_config.triangle_count-volume);
			}
                        bool changed_label = new_conductance < best_conductance || (new_conductance==best_conductance&&random_functions::nextBool()); 

                        //second sweep for finding max and resetting array

                        /* PartitionID max_value = 1; */
                        /* forall_out_edges(G, e, node) { */
                        /*         NodeID target             = G.getEdgeTarget(e); */
                        /*         PartitionID cur_block     = G.getPartitionIndex(target); */
                        /*         /1* PartitionID cur_cut = (partitionIDSource==block_artif_node)? hash_map[cur_block]; *1/ */
                        /*         /1* PartitionID cur_value     = hash_map[cur_block]; *1/ */
                        /*         if((cur_value > max_value  || (cur_value == max_value && random_functions::nextBool())) ) */
                        /*         { */
                        /*                 max_value = cur_value; */
                        /*                 max_block = cur_block; */
                        /*         } */

                        /*         /1* hash_map[cur_block] = 0; *1/ */
                        /* } endfor */

			/* if (G.getPartitionIndex(node) != block_artif_node) cluster_size -= G.getNodeWeight(node); */
			/* if (max_block != block_artif_node) cluster_size -= G.getNodeWeight(node); */

                        /* cluster_sizes[G.getPartitionIndex(node)]  -= G.getNodeWeight(node); */
                        /* cluster_sizes[max_block]         += G.getNodeWeight(node); */



                        /* bool changed_label                = G.getPartitionIndex(node) != max_block; */ 
                        /* change_counter                   += changed_label; */
                        /* G.setPartitionIndex(node, max_block); */

                        if(changed_label) {
				edgeCut = edgeCut+delta_cut;
				volume = volume+delta_degree;
				G.setPartitionIndex(node, partitionIDTarget);
                                forall_out_edges(G, e, node) {
                                        NodeID target = G.getEdgeTarget(e);
                                        if(!(*next_Q_contained)[target]) {
                                                next_Q->push(target);
                                                (*next_Q_contained)[target] = true;
                                        } 
                                } endfor
                        }
                } 

                std::swap( Q, next_Q);
                std::swap( Q_contained, next_Q_contained);

        }
        

        delete Q;
        delete next_Q;
        delete Q_contained;
        delete next_Q_contained;
       
        return 0;

}

