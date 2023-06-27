/******************************************************************************
 * triangle_listing.cpp 
 * *
 * XXX.
 * XXX
 *****************************************************************************/

#include <algorithm>
#include <argtable3.h>
#include <iostream>
#include <math.h>
#include <regex.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <tuple>
#include <sys/stat.h>
#include <fstream>

#include "random_functions.h"
#include "triangle_listing.h"

triangle_listing::triangle_listing() {
                
}

triangle_listing::~triangle_listing() {
                
}

void triangle_listing::write_triangles_weighted(NodeID n, std::vector<std::vector<NodeID>> *triangles, std::string filename, NodeID totalSize, int contractedIndex) {
        std::ofstream f(filename.c_str());
        std::cout << "Writing triangles to " << filename << " ... " << std::endl;

        // first line - number of edges of hypergraph, number of nodes in hypergraph
        f << triangles->size() << " " << n << std::endl;
        // store all triangles as edges of a hypergraph
        NodeID first,second,third;
        for (int i = 0; i < triangles->size(); i++) {
                first = (*triangles)[i][0];
                second = (*triangles)[i][1];
                third = (*triangles)[i][2];
                if(first == totalSize) {
                        first = contractedIndex;
                }

                if(second == totalSize) {
                        second = contractedIndex;
                }

                if(third == totalSize) {
                        third = contractedIndex;
                }

                if(first != -1) {
                        f << first << " ";
                }
                if(second != -1) {
                        f << second << " ";
                }
                if(third != -1) {
                        f << third << " ";
                }
                f << std::endl;
        }
        f.close();
}

void triangle_listing::write_triangles(NodeID n, std::vector<std::vector<NodeID>> triangles, std::string filename) {
        std::ofstream f(filename.c_str());
        std::cout << "Writing triangles to " << filename << " ... " << std::endl;

        // first line - number of edges of hypergraph, number of nodes in hypergraph
        f << triangles.size() << " " << n << std::endl;
        // store all triangles as edges of a hypergraph
        NodeID first,second,third;
        for (int i = 0; i < triangles.size(); i++) {
                first = triangles[i][0];
                second = triangles[i][1];
                third = triangles[i][2];
        
                f << first << " ";
                f << second << " ";
                f << third << " ";
                f << std::endl;
        }
        f.close();
}

std::vector <std::vector<NodeID>>* triangle_listing::triangle_run(graph_access &G, NodeID seed_node, float epsilon_depth, std::vector<bool> &lastLayerNodes, NodeID &hypergraph_node_count, std::vector <NodeID> *&mapping2){
        // ***************************** perform triangle detection ***************************************       

        //std::vector <std::vector<NodeID>> triangle_set;
        NodeID n = G.number_of_nodes();
        std::vector <bool> marked(n, false);
        NodeID contracted_node = n+1;
        int pushIndex = 0;

        int lastNodeCount = 0;
        for(int i = 0; i < lastLayerNodes.size(); i++) {
                if(lastLayerNodes[i]==true){
                        lastNodeCount++;
                }
        }
        
        NodeID first,second,third;
        std::vector <std::vector<NodeID>> *contracted_triangle_set = new std::vector <std::vector<NodeID>>();
        int lastcount = 0;

        std::vector <bool> isMapped(n+2, false);
        std::vector <NodeID> *newMapping = new std::vector <NodeID>(n+2, -1);
        (*newMapping)[contracted_node] = n-lastNodeCount;
        isMapped[contracted_node] = true;
        int mapIndex = 0;
        pushIndex = 0;

        forall_nodes(G, node) {
                forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                        marked[target] = true;
                }endfor

                forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                            forall_out_edges(G,e2,target) {
                                NodeID target2 = G.getEdgeTarget(e2);
                                if(marked[target2]==true) {
                                        if(node < target && target < target2) {
                                                first = node;
                                                second = target;
                                                third = target2;
                                                //detect last layer nodes
                                                if(lastLayerNodes[first]==true){
                                                        lastcount++;
                                                        first = contracted_node;
                                                }
                                                if(lastLayerNodes[second]==true){
                                                        lastcount++;
                                                        second = contracted_node;
                                                }
                                                if(lastLayerNodes[third]==true){
                                                        lastcount++;
                                                        third = contracted_node;
                                                }
                                                //remap nodes
                                                if(isMapped[first]==false) {
                                                        (*newMapping)[first] = mapIndex;
                                                        isMapped[first] = true;
                                                        mapIndex++;
                                                }

                                                if(isMapped[second]==false) {
                                                        (*newMapping)[second] = mapIndex;
                                                        isMapped[second] = true;
                                                        mapIndex++;
                                                }

                                                if(isMapped[third]==false) {
                                                        (*newMapping)[third] = mapIndex;
                                                        isMapped[third] = true;
                                                        mapIndex++;
                                                }

                                                //if multiple last layer nodes in triangle, contract one of them
                                                if(lastcount == 3) {
                                                        lastcount = 0;
                                                        continue;
                                                }
                                                //add triangle as vector to contracted triangle set
                                                contracted_triangle_set->push_back(std::vector<NodeID>());

                                                if(lastcount == 0 || lastcount == 1) {
                                                        (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[first]);                            
                                                        (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[second]);                            
                                                        (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[third]);  
                                                }
                                                if(lastcount == 2) {
                                                        if(first!=contracted_node){
                                                                (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[first]);                            
                                                                (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[second]);
                                                        }
                                                        else if(second!=contracted_node){
                                                                (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[first]);                            
                                                                (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[second]);
                                                        }
                                                        else if(third!=contracted_node){
                                                                (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[first]);                            
                                                                (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[third]);
                                                        }
                                                }
                                                                                                         
                                                pushIndex++;                                                                                  
                                                lastcount = 0;                                                
                                        }
                                }
                            }endfor
                }endfor 

                forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                        marked[target] = false;
                }endfor
        }endfor

        //update mapping of contracted node                                                                         
        (*newMapping)[contracted_node]=mapIndex;
        for(int i = 0; i < contracted_triangle_set->size(); i++){
                for(int j = 0; j < (*contracted_triangle_set)[i].size(); j++){
                        if((*contracted_triangle_set)[i][j]==n-lastNodeCount){
                                (*contracted_triangle_set)[i][j]=mapIndex;
                        }
                }
        }

        hypergraph_node_count = mapIndex+1;
        mapping2 = newMapping;
        return contracted_triangle_set;

}

long long triangle_listing::triangle_run_graph(graph_access &G, std::vector<bool> &lastLayerNodes) {
        // ***************************** perform triangle detection ***************************************       

        //std::vector <std::vector<NodeID>> triangle_set;
        NodeID n = G.number_of_nodes();
        std::vector <bool> marked(n, false);
	long long unsigned num_triangles=0;
        /*
        std::vector <std::vector<NodeID>> triangle_set; //test
        int pushIndex = 0; //test
        */

        forall_nodes(G, node) {
                forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                        marked[target] = true;
                }endfor

                forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                            forall_out_edges(G,e2,target) {
                                NodeID target2 = G.getEdgeTarget(e2);
                                if(marked[target2]==true) {
                                        //std::cout << "Edge weight for " << node << " to " << target << " is " << G.getEdgeWeight(e) << std::endl;
                                        //std::cout << "Edge weight for " << target << " to " << target2 << " is " << G.getEdgeWeight(e) << std::endl;
                                        //G.setEdgeWeight(e, G.getEdgeWeight(e)+1);
                                        //G.setEdgeWeight(e2, G.getEdgeWeight(e2)+1);
                                        if(node < target && target < target2 && (!lastLayerNodes[node] || !lastLayerNodes[target] || !lastLayerNodes[target2])) { 
						num_triangles++;
                                                G.setEdgeWeight(e, G.getEdgeWeight(e)+1); //node to target
                                                G.setEdgeWeight(e2, G.getEdgeWeight(e2)+1); //target to target2
                                                //target2 to target and target2 to node
                                                forall_out_edges(G,e3,target2) {
                                                        if(G.getEdgeTarget(e3) == node || G.getEdgeTarget(e3) == target){
                                                                G.setEdgeWeight(e3, G.getEdgeWeight(e3)+1);
                                                        }
                                                }endfor
                                                //target to node
                                                
                                                forall_out_edges(G,e4,target) {
                                                        if(G.getEdgeTarget(e4) == node){
                                                                G.setEdgeWeight(e4, G.getEdgeWeight(e4)+1);
								break;
                                                        }
                                                }endfor
                                                //node to target2
                                                forall_out_edges(G,e5,node) {
                                                        if(G.getEdgeTarget(e5) == target2){
                                                                G.setEdgeWeight(e5, G.getEdgeWeight(e5)+1);
								break;
                                                        }
                                                }endfor
                                                
                                                /*
                                                triangle_set.push_back(std::vector<NodeID>());
                                                triangle_set[pushIndex].push_back(node);
                                                triangle_set[pushIndex].push_back(target);
                                                triangle_set[pushIndex].push_back(target2);
                                                pushIndex++;
                                                */
                                                
                                        }
                                }
                            }endfor
                }endfor 

                forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                        marked[target] = false;
                }endfor
        }endfor

        /*
        NodeID first,second,third;
        for (int i = 0; i < triangle_set.size(); i++) {
                        first = triangle_set[i][0];
                        second = triangle_set[i][1];
                        third = triangle_set[i][2];
                        std::cout << first << " " << second << " " << third << std::endl;
        }
        forall_nodes(G, node) {
            forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                        std::cout << "Edge weight for " << node << " to " << target << " is " << G.getEdgeWeight(e) << std::endl; 
            }endfor
        }endfor
        */
	return num_triangles;
}

std::vector <std::vector<NodeID>>* triangle_listing::triangle_run_uncontracted(graph_access &G, NodeID seed_node, float epsilon_depth, std::vector<bool> &lastLayerNodes, NodeID &hypergraph_node_count, std::vector<NodeID> &prelim_partition, std::vector <NodeID> *&mapping2){
        // ***************************** perform triangle detection without contraction ***************************************       

        //std::vector <std::vector<NodeID>> triangle_set;
        NodeID n = G.number_of_nodes();
        std::vector <bool> marked(n, false);
        int pushIndex = 0;

        int lastNodeCount = 0;
        for(int i = 0; i < lastLayerNodes.size(); i++) {
                if(lastLayerNodes[i]==true){
                        lastNodeCount++;
                }
        }
        
        NodeID first,second,third;
        std::vector <std::vector<NodeID>> *contracted_triangle_set = new std::vector <std::vector<NodeID>>();
        int lastcount = 0;

        std::vector <bool> isMapped(n+2, false);
        std::vector <NodeID> *newMapping = new std::vector <NodeID>(n+2, -1);
        int mapIndex = 0;
        pushIndex = 0;

        forall_nodes(G, node) {
                forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                        marked[target] = true;
                }endfor

                forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                            forall_out_edges(G,e2,target) {
                                NodeID target2 = G.getEdgeTarget(e2);
                                if(marked[target2]==true) {
                                        if(node < target && target < target2) {
                                                first = node;
                                                second = target;
                                                third = target2;
                                                //detect last layer nodes
                                                if(lastLayerNodes[first]==true){
                                                        lastcount++;
                                                }
                                                if(lastLayerNodes[second]==true){
                                                        lastcount++;
                                                }
                                                if(lastLayerNodes[third]==true){
                                                        lastcount++;
                                                }
                                                //remap nodes
                                                if(isMapped[first]==false) {
                                                        (*newMapping)[first] = mapIndex;
                                                        //std::cout << first << " is mapped to " << mapIndex << " with part ID ";
                                                        isMapped[first] = true;
                                                        if(lastLayerNodes[first]==true){
                                                                prelim_partition.push_back(1);
                                                                //std::cout << "1" << std::endl;
                                                        }
                                                        else {
                                                                prelim_partition.push_back(0);
                                                                //std::cout << "0" << std::endl;
                                                        }
                                                        mapIndex++;
                                                }

                                                if(isMapped[second]==false) {
                                                        (*newMapping)[second] = mapIndex;
                                                        //std::cout << second << " is mapped to " << mapIndex << " with part ID ";
                                                        isMapped[second] = true;
                                                        if(lastLayerNodes[second]==true){
                                                                prelim_partition.push_back(1);
                                                                //std::cout << "1" << std::endl;
                                                        }
                                                        else {
                                                                prelim_partition.push_back(0);
                                                                //std::cout << "0" << std::endl;
                                                        }
                                                        mapIndex++;
                                                }

                                                if(isMapped[third]==false) {
                                                        (*newMapping)[third] = mapIndex;
                                                        //std::cout << third << " is mapped to " << mapIndex << " with part ID ";
                                                        isMapped[third] = true;
                                                        if(lastLayerNodes[third]==true){
                                                                prelim_partition.push_back(1);
                                                                //std::cout << "1" << std::endl;
                                                        }
                                                        else {
                                                                prelim_partition.push_back(0);
                                                                //std::cout << "0" << std::endl;
                                                        }
                                                        mapIndex++;
                                                }
                                                if(lastcount == 3) {
                                                        lastcount = 0;
                                                        continue;
                                                }
                                                //add triangle as vector to contracted triangle set
                                                contracted_triangle_set->push_back(std::vector<NodeID>());
                                                
                                                
                                                (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[first]);                            
                                                (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[second]);                            
                                                (*contracted_triangle_set)[pushIndex].push_back((*newMapping)[third]);  
                                                                                
                                                pushIndex++;                                                                                  
                                                lastcount = 0;                                                
                                        }
                                }
                            }endfor
                }endfor 

                forall_out_edges(G, e, node) {
                        NodeID target = G.getEdgeTarget(e);
                        marked[target] = false;
                }endfor
        }endfor

        hypergraph_node_count = mapIndex;
        mapping2 = newMapping;

        return contracted_triangle_set;

}
