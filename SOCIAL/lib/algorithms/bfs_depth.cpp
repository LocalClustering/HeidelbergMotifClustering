/******************************************************************************
 * bfs_depth.cpp 
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
#include "bfs_depth.h"
#include "data_structure/graph_access.h"

bfs_depth::bfs_depth() {
                
}

bfs_depth::~bfs_depth() {
                
}

graph_access* bfs_depth::runBFS(graph_access & G, NodeID seed_node, short int & epsilon_depth, std::vector<NodeID> &mapping, std::vector<NodeID> *&hypergraph_map, std::vector<bool> &lastLayerNodes, std::vector<short int>& level, std::vector<bool>& visited, std::vector<bool>& touched, bool force_over_100){

        short int maxdepth = epsilon_depth;
        //std::cout << "Performing BFS from " << seed_node << " upto depth " << maxdepth << std::endl;
        maxdepth++; // this is for the unused outer layer       

	NodeID ball_counter = 0;
        std::vector<NodeID> *visitedNodes = new std::vector<NodeID>();
        std::queue<NodeID> bfsQueue;
        bfsQueue.push(seed_node);
        level[seed_node] = 0;
        visited[seed_node] = true;
        NodeID currentNode;

        while(!bfsQueue.empty()){
                currentNode = bfsQueue.front();
                if(level[currentNode] == maxdepth + 1) {
			if (force_over_100 && ball_counter < 100) {
				maxdepth++;
				ball_counter = visitedNodes->size();
			} else {
				break;
			}
                }
                bfsQueue.pop();
                visitedNodes->push_back(currentNode);
		if(level[currentNode] < maxdepth) ball_counter++;
                if(level[currentNode] < maxdepth || force_over_100) {
			forall_out_edges(G, e, currentNode) {
				NodeID target = G.getEdgeTarget(e);
				if(visited[target]){
					continue;
				}
				bfsQueue.push(target);
				level[target] = level[currentNode]+1;
				visited[target]=true;
			}endfor
		}
        }

	if (force_over_100) epsilon_depth = maxdepth - 1;

        NodeID mapindex = 0;
        
        for(const auto & j: *visitedNodes){
                touched[j]=true;
                mapping[j]=mapindex;
                mapindex++;
        }

        EdgeID edgeCount = 0;

        std::vector<std::vector<NodeID>> subgraph_edges;
        subgraph_edges.resize(visitedNodes->size());
        for(const auto &j : *visitedNodes) {
                forall_out_edges(G, e, j) {
                        NodeID target = G.getEdgeTarget(e);
                        if(touched[target]){
                                edgeCount++;
                                subgraph_edges[mapping[j]].push_back(mapping[target]);
                        }
                }endfor
        }

        lastLayerNodes.resize(visitedNodes->size());
        for(const auto&j : *visitedNodes) {
                touched[j] = true;
                if(level[j]==maxdepth){
                        lastLayerNodes[mapping[j]]=true;
                }
        }

	// Cleanup global vectors
        for(const auto&j : *visitedNodes) {
		level[j] = -1;
		visited[j] = false;
		touched[j] = false;
        }
        while(!bfsQueue.empty()){
                auto j = bfsQueue.front();
                bfsQueue.pop();
		visited[j] = false;
	}
   
        graph_access *G_temp = new graph_access();
        //graph_access &G_temp = *G_tempone;
        G_temp->start_construction_light(visitedNodes->size(), edgeCount);
        for (NodeID u = 0; u < visitedNodes->size(); ++u) {
                G_temp->new_node();
                //G_temp->setPartitionIndex(u, 0);
		G_temp->setNodeWeight(u, 1);

                for(auto &j : subgraph_edges[u]) {
                        const EdgeID e = G_temp->new_edge(u, j);
			G_temp->setEdgeWeight(e, 1);
                }

        }
        G_temp->finish_construction_light();
        
        hypergraph_map = visitedNodes;
        return G_temp;
}

graph_access* bfs_depth::runBFS_graph(graph_access & G, std::vector<NodeID> & seed_nodes, short int & epsilon_depth, std::vector<NodeID> &mapping, std::vector<NodeID> *&hypergraph_map, std::vector<bool> &lastLayerNodes, std::vector<short int>& level, std::vector<bool>& visited, std::vector<bool>& touched, bool force_over_100){

        short int maxdepth = epsilon_depth;
        //std::cout << "Performing BFS from " << seed_node << " upto depth " << maxdepth << std::endl;
        maxdepth++; // this is for the unused outer layer       

/* std::cout << "Point 4\n"; */
	NodeID ball_counter = 0;
        std::vector<NodeID> *visitedNodes = new std::vector<NodeID>();
        std::queue<NodeID> bfsQueue;
	for (const auto& seed_node: seed_nodes) {
		bfsQueue.push(seed_node);
		level[seed_node] = 0;
		visited[seed_node] = true;
	}
        NodeID currentNode;
/* std::cout << "Point 5\n"; */

        while(!bfsQueue.empty()){
                currentNode = bfsQueue.front();
                if(level[currentNode] == maxdepth + 1) {
			if (force_over_100 && ball_counter < 100) {
				maxdepth++;
				ball_counter = visitedNodes->size();
			} else {
				break;
			}
                }
                bfsQueue.pop();
                visitedNodes->push_back(currentNode);
		if(level[currentNode] < maxdepth) ball_counter++;
                if(level[currentNode] < maxdepth || force_over_100) {
			forall_out_edges(G, e, currentNode) {
				NodeID target = G.getEdgeTarget(e);
				if(visited[target]){
					continue;
				}
				bfsQueue.push(target);
				level[target] = level[currentNode]+1;
				visited[target]=true;
			}endfor
		}
        }
/* std::cout << "Point 6\n"; */

	if (force_over_100) epsilon_depth = maxdepth - 1;

        NodeID mapindex = 0;
        
        for(const auto & j: *visitedNodes){
                touched[j]=true;
                mapping[j]=mapindex;
                mapindex++;
        }

        EdgeID edgeCount = 0;
/* std::cout << "Point 7\n"; */

        std::vector<std::vector<NodeID>> subgraph_edges;
        subgraph_edges.resize(visitedNodes->size());
        for(const auto &j : *visitedNodes) {
                forall_out_edges(G, e, j) {
                        NodeID target = G.getEdgeTarget(e);
                        if(touched[target]){
                                edgeCount++;
                                subgraph_edges[mapping[j]].push_back(mapping[target]);
                        }
                }endfor
        }
/* std::cout << "Point 8\n"; */

        lastLayerNodes.resize(visitedNodes->size());
        for(const auto&j : *visitedNodes) {
                touched[j] = true;
                if(level[j]==maxdepth){
                        lastLayerNodes[mapping[j]]=true;
                }
        }
/* std::cout << "Point 9\n"; */

	// Cleanup global vectors
        for(const auto&j : *visitedNodes) {
		level[j] = -1;
		visited[j] = false;
		touched[j] = false;
        }
/* std::cout << "Point 10\n"; */
        while(!bfsQueue.empty()){
                auto j = bfsQueue.front();
                bfsQueue.pop();
		visited[j] = false;
	}
/* std::cout << "Point 11\n"; */
   
        graph_access *G_temp = new graph_access();
        //graph_access &G_temp = *G_tempone;
        G_temp->start_construction(visitedNodes->size(), edgeCount);
        for (NodeID u = 0; u < visitedNodes->size(); ++u) {
                G_temp->new_node();
                G_temp->setPartitionIndex(u, 0);
		G_temp->setNodeWeight(u, 1);

                for(auto &j : subgraph_edges[u]) {
                        const EdgeID e = G_temp->new_edge(u, j);
			G_temp->setEdgeWeight(e, 0);
                }

        }
/* std::cout << "Point 12\n"; */
        G_temp->finish_construction();
        
        hypergraph_map = visitedNodes;
/* std::cout << "Point 13\n"; */
        return G_temp;
}

graph_access* bfs_depth::runBFS_evaluator(graph_access & G, std::vector<NodeID> & seed_nodes, short int epsilon_depth, std::vector<NodeID> &mapping, std::vector<NodeID> *&hypergraph_map, std::vector<bool> &lastLayerNodes, std::vector<short int>& level, std::vector<bool>& visited, std::vector<bool>& touched, bool force_over_100){

        short int maxdepth = epsilon_depth;
        //std::cout << "Performing BFS from " << seed_node << " upto depth " << maxdepth << std::endl;
        maxdepth++; // this is for the unused outer layer       

/* std::cout << "Point 4\n"; */
	NodeID ball_counter = 0;
        std::vector<NodeID> *visitedNodes = new std::vector<NodeID>();
        std::queue<NodeID> bfsQueue;
	for (const auto& seed_node: seed_nodes) {
		bfsQueue.push(seed_node);
		level[seed_node] = 0;
		visited[seed_node] = true;
	}
        NodeID currentNode;
/* std::cout << "Point 5\n"; */

        while(!bfsQueue.empty()){
                currentNode = bfsQueue.front();
                if(level[currentNode] == maxdepth + 1) {
			if (force_over_100 && ball_counter < 100) {
				maxdepth++;
				ball_counter = visitedNodes->size();
			} else {
				break;
			}
                }
                bfsQueue.pop();
                visitedNodes->push_back(currentNode);
		if(level[currentNode] < maxdepth) ball_counter++;
                if(level[currentNode] < maxdepth || force_over_100) {
			forall_out_edges(G, e, currentNode) {
				NodeID target = G.getEdgeTarget(e);
				if(visited[target]){
					continue;
				}
				bfsQueue.push(target);
				level[target] = level[currentNode]+1;
				visited[target]=true;
			}endfor
		}
        }
/* std::cout << "Point 6\n"; */

	if (force_over_100) epsilon_depth = maxdepth - 1;

        NodeID mapindex = 0;
        
        for(const auto & j: *visitedNodes){
                touched[j]=true;
                mapping[j]=mapindex;
                mapindex++;
        }

        EdgeID edgeCount = 0;
/* std::cout << "Point 7\n"; */

        std::vector<std::vector<NodeID>> subgraph_edges;
        subgraph_edges.resize(visitedNodes->size());
        for(const auto &j : *visitedNodes) {
                forall_out_edges(G, e, j) {
                        NodeID target = G.getEdgeTarget(e);
                        if(touched[target]){
                                edgeCount++;
                                subgraph_edges[mapping[j]].push_back(mapping[target]);
                        }
                }endfor
        }
/* std::cout << "Point 8\n"; */

        lastLayerNodes.resize(visitedNodes->size());
        for(const auto&j : *visitedNodes) {
                touched[j] = true;
                if(level[j]==maxdepth){
                        lastLayerNodes[mapping[j]]=true;
                }
        }
/* std::cout << "Point 9\n"; */

	// Cleanup global vectors
        for(const auto&j : *visitedNodes) {
		level[j] = -1;
		visited[j] = false;
		touched[j] = false;
        }
/* std::cout << "Point 10\n"; */
        while(!bfsQueue.empty()){
                auto j = bfsQueue.front();
                bfsQueue.pop();
		visited[j] = false;
	}
/* std::cout << "Point 11\n"; */
   
        graph_access *G_temp = new graph_access();
        //graph_access &G_temp = *G_tempone;
        G_temp->start_construction(visitedNodes->size(), edgeCount);
        for (NodeID u = 0; u < visitedNodes->size(); ++u) {
                G_temp->new_node();
                G_temp->setPartitionIndex(u, 0);
		G_temp->setNodeWeight(u, 1);

                for(auto &j : subgraph_edges[u]) {
                        const EdgeID e = G_temp->new_edge(u, j);
			G_temp->setEdgeWeight(e, 0);
                }

        }
/* std::cout << "Point 12\n"; */
        G_temp->finish_construction();
        
        hypergraph_map = visitedNodes;
/* std::cout << "Point 13\n"; */
        return G_temp;
}
