/******************************************************************************
 * mqi.cpp 
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
#include "mqi.h"
#include "push_relabel.h"
#include "data_structure/graph_access.h"
#include "data_structure/flow_graph.h"
#include "io/graph_io.h"
#include "quality_metrics.h"

mqi::mqi() {
                
}

mqi::~mqi() {
                
}

void mqi::mqi_improvement(graph_access & S, bool fix_seed_node){
    /*--------------------------------------------------------------------------- 
                        perform mqi improvement on graph
    -----------------------------------------------------------------------------*/ 
    flow_graph fG; 
    NodeID n = 0; // flow graph size
    NodeID blockVolume = 0; //volume of current block
    std::vector<bool> isIncluded(S.number_of_nodes(), false); //boolean vector to test if node from subgraph is in cluster
    std::vector<NodeID> nodeDegrees(S.number_of_nodes(), 0); //weighted node degrees of all nodes in S

    forall_nodes(S, node){
        forall_out_edges(S, e, node) {
                nodeDegrees[node] += S.getEdgeWeight(e);
        }endfor
        if (S.getPartitionIndex(node) != S.getPartitionIndex(1)) {
            isIncluded[node] = true;
            n++;
            blockVolume += nodeDegrees[node];
		}
    }endfor

    n += 2; //size of directed graph for model including source and sink artificial nodes

    fG.start_construction(n); //initialize directed flow graph

    NodeID source = n-2;
    NodeID sink   = n-1;
    int idx = 0;
    NodeID sourceID, targetID;
    NodeID internalEdgeCapacity, sourceEdgeCapacity, sinkEdgeCapacity;
    
    int c = 0; //number of cut edges
    std::vector<NodeID> nodeMapping(S.number_of_nodes(), -1); // node remapping in flow graph

    // add nodes in cluster to directed graph, as well as directed edge from source to cluster nodes
    forall_nodes(S, node) {
        if(isIncluded[node]){
            if(nodeMapping[node]!=-1){
                sourceID = nodeMapping[node];
            }
            else{
                nodeMapping[node] = idx;
                idx++;
                sourceID = nodeMapping[node];
            }
            forall_out_edges(S, e, node) {
                NodeID target = S.getEdgeTarget(e);
                if(isIncluded[target]){ 
                    // if both nodes of edge in cluster, add directed edge
                    if(nodeMapping[target]!=-1){
                        targetID = nodeMapping[target];
                    }
                    else{
                        nodeMapping[target] = idx;
                        idx++;
                        targetID = nodeMapping[target];
                    }
                    internalEdgeCapacity = S.getEdgeWeight(e)*blockVolume;
                    fG.new_edge(sourceID, targetID, internalEdgeCapacity);
                }
                else{
                    // add edge from source to cluster node (contracting target to source node if target outside cluster)
                    sourceEdgeCapacity = S.getEdgeWeight(e);
                    fG.new_edge(source, sourceID, sourceEdgeCapacity*blockVolume); 
                    c += sourceEdgeCapacity; // compute cut
                }
            }endfor
        }        
    }endfor

    NodeID infinite_capacity = c * blockVolume;

    // add edges from cluster nodes to sink with capacity c
    forall_nodes(S, node) {
        if(isIncluded[node]){
            sourceID = nodeMapping[node];
            sinkEdgeCapacity = c*nodeDegrees[node];
            if(node == 0) { // seed node
                if(fix_seed_node) { // edge from seed node to sink = infinite capacity
                    fG.new_edge(sourceID, sink, infinite_capacity);
                }
                else {
                    fG.new_edge(sourceID, sink, sinkEdgeCapacity);
                }
            }
            else {
                fG.new_edge(sourceID, sink, sinkEdgeCapacity);
            }
        }
    }endfor

    // compute max flow on constructed flow graph using push relabel
    push_relabel pr;
    std::vector< NodeID > source_set;
    FlowType flowvalue = pr.solve_max_flow_min_cut(fG, source, sink, true, source_set);

    // reverse map nodes from flow graph to original graph
    std::vector<NodeID> revMapping(idx);
    for(int i = 0; i < nodeMapping.size(); i++) {
        if(nodeMapping[i]!=-1) {
            revMapping[nodeMapping[i]] = i;
        }
    }

    // remove nodes reachable from source from cluster
    for(int i = 0; i < source_set.size(); i++) {
        if(source_set[i]==source){
            continue;
        }
        S.setPartitionIndex(revMapping[source_set[i]], S.getPartitionIndex(1));
    }

    // if seed node removed from cluster, send it back
    if(!fix_seed_node) {
        if(S.getPartitionIndex(0) == S.getPartitionIndex(1)){
            S.setPartitionIndex(0, 1-S.getPartitionIndex(1));
        }
    }
}


void mqi::mqi_improvement_ball(graph_access & S, bool fix_seed_node){
    /*--------------------------------------------------------------------------- 
                        perform mqi improvement on graph ball
    -----------------------------------------------------------------------------*/ 
    flow_graph fG; 
    NodeID n = 0; // flow graph size
    NodeID blockVolume = 0; //volume of current block
    std::vector<bool> isIncluded(S.number_of_nodes(), false); //boolean vector to test if node from subgraph is in cluster
    std::vector<NodeID> nodeDegrees(S.number_of_nodes(), 0); //weighted node degrees of all nodes in S

    forall_nodes(S, node){
        forall_out_edges(S, e, node) {
                nodeDegrees[node] += S.getEdgeWeight(e);
        }endfor
        if (S.getPartitionIndex(node) != 1) {
            isIncluded[node] = true;
            n++;
            blockVolume += nodeDegrees[node];
		}
    }endfor

    n += 2; //size of directed graph for model including source and sink artificial nodes
    
    fG.start_construction(n); //initialize directed flow graph

    NodeID source = n-2;
    NodeID sink   = n-1;
    int idx = 0;
    NodeID sourceID, targetID;
    NodeID internalEdgeCapacity, sourceEdgeCapacity, sinkEdgeCapacity;
    
    int c = 0; //number of cut edges
    std::vector<NodeID> nodeMapping(S.number_of_nodes(), -1); // node remapping in flow graph

    // add nodes in cluster to directed graph, as well as directed edge from source to cluster nodes
    forall_nodes(S, node) {
        if(isIncluded[node]){
            if(nodeMapping[node]!=-1){
                sourceID = nodeMapping[node];
            }
            else{
                nodeMapping[node] = idx;
                idx++;
                sourceID = nodeMapping[node];
            }
            forall_out_edges(S, e, node) {
                NodeID target = S.getEdgeTarget(e);
                if(isIncluded[target]){
                    // if both nodes of edge in cluster, add directed edge 
                    if(nodeMapping[target]!=-1){
                        targetID = nodeMapping[target];
                    }
                    else{
                        nodeMapping[target] = idx;
                        idx++;
                        targetID = nodeMapping[target];
                    }
                    internalEdgeCapacity = S.getEdgeWeight(e)*blockVolume;
                    fG.new_edge(sourceID, targetID, internalEdgeCapacity);
                }
                else{
                    // add edge from source to cluster node (contracting target to source node if target outside cluster)
                    sourceEdgeCapacity = S.getEdgeWeight(e);
                    fG.new_edge(source, sourceID, sourceEdgeCapacity*blockVolume);
                    c += sourceEdgeCapacity; // compute cut
                }
            }endfor
        }        
    }endfor


    NodeID infinite_capacity = c * blockVolume;

    // add edges from cluster nodes to sink with capacity c
    forall_nodes(S, node) {
        if(isIncluded[node]){
            sourceID = nodeMapping[node];
            sinkEdgeCapacity = c*nodeDegrees[node];
            if(node == 0) { // seed node
                if(fix_seed_node) { // edge from seed node to sink = infinite capacity
                    fG.new_edge(sourceID, sink, infinite_capacity);
                }
                else {
                    fG.new_edge(sourceID, sink, sinkEdgeCapacity);
                }
            }
            else {
                fG.new_edge(sourceID, sink, sinkEdgeCapacity);
            }
        }
    }endfor

    // compute max flow on constructed flow graph using push relabel
    push_relabel pr;
    std::vector< NodeID > source_set;
    FlowType flowvalue = pr.solve_max_flow_min_cut(fG, source, sink, true, source_set);

    // reverse map nodes from flow graph to original graph
    std::vector<NodeID> revMapping(idx);
    for(int i = 0; i < nodeMapping.size(); i++) {
        if(nodeMapping[i]!=-1) {
            revMapping[nodeMapping[i]] = i;
        }
    }

    // remove nodes reachable from source from cluster
    for(int i = 0; i < source_set.size(); i++) {
        if(source_set[i]==source){
            continue;
        }
        S.setPartitionIndex(revMapping[source_set[i]], 1);
    }

    // if seed node removed from cluster, send it back
    if(!fix_seed_node) {
        if(S.getPartitionIndex(0) == S.getPartitionIndex(1)){
            S.setPartitionIndex(0, 1-S.getPartitionIndex(1));
        }
    }

}

double mqi::mqi_improvement_hypergraph_ball_lawler(std::vector<NodeID> & eind, std::vector<size_t> & eptr, 
                                        NodeID num_vertices, NodeID num_hyperedges, 
                                        int clusterBlock, std::vector<NodeID> & partition, std::vector<bool> & isCutHyperedge, 
                                        std::vector<bool> & isTriangleHyperedge, int & graphNodeCount, int & graphEdgeCount, std::vector<bool> & isIncludedHyperedge, bool noMotif, std::vector<int> & hypernode_degrees,
                                        std::vector<NodeID> & contracted_hyperedge_weights, int &hypergraph_cut, NodeID &min_volume, bool fix_seed_node){
    
    /*--------------------------------------------------------------------------- 
            perform mqi improvement on hypergraph using Lawler expansion
    -----------------------------------------------------------------------------*/ 
                                    
    int hyperedgeCount = 0;
    std::vector<NodeID> mapping(num_vertices, -1);
    std::vector<NodeID> reverse_mapping(num_vertices, -1);
    std::vector<NodeID> graphNodes;
    std::vector<NodeID> updatedPartitionID;
    std::vector<std::vector<NodeID>> graphEdgeSet;
    std::vector<std::vector<NodeID>> graphEdgeWeightSet;

    int mapIndex = 0;
    int hyperedgeNodeCount = 0;
    int includedEdgeCount = 0;
    int numArtificialNodes = 0;

    // remapping nodes of the included hyperedges
    for(int i = 0; i < eind.size(); i++) {
        if(i == eptr[hyperedgeCount+1]){
                hyperedgeCount++;
                hyperedgeNodeCount = 0;
        }
        
        if(isIncludedHyperedge[hyperedgeCount]){
            // how many artificial nodes needed
            if(hyperedgeNodeCount == 0) {
                numArtificialNodes ++; //e1
                numArtificialNodes ++; //e2
            }
            // remapping 
            if(mapping[eind[i]]==-1){
                mapping[eind[i]] = mapIndex;
                reverse_mapping[mapIndex] = eind[i];
                graphNodes.push_back(eind[i]);
                updatedPartitionID.push_back(partition[eind[i]]);
                mapIndex++;
            }
            hyperedgeNodeCount++;
        }
    }

    int lastOriginalPointer = mapIndex; // pointer to last node that is actually in the hypergraph

    hyperedgeCount = 0;
    hyperedgeNodeCount = 0;
    int e1 = 0;
    int e2 = 0;
    graphEdgeSet.resize(numArtificialNodes + graphNodes.size());
    graphEdgeWeightSet.resize(numArtificialNodes + graphNodes.size());
    int numEdges = 0;

    // building graph nodes and edges set for flow network
    for(int i = 0; i < eind.size(); i++) {
        if(i == eptr[hyperedgeCount+1]){
                hyperedgeCount++;
                hyperedgeNodeCount = 0;
        }
        // if hyperedge is included in the cluster
        if(isIncludedHyperedge[hyperedgeCount]){
            if(hyperedgeNodeCount == 0) { // add artificial nodes e1 and e2 for each hyperedge, and an edge from e1 to e2
                graphNodes.push_back(mapIndex); //add e1
                updatedPartitionID.push_back(clusterBlock);
                e1 = mapIndex;
                mapIndex++;
                graphNodes.push_back(mapIndex); //add e2
                updatedPartitionID.push_back(clusterBlock);
                e2 = mapIndex;
                mapIndex++;
                graphEdgeSet[graphNodes[e1]].push_back(graphNodes[e2]); //edge from e1 to e2
                graphEdgeWeightSet[graphNodes[e1]].push_back(contracted_hyperedge_weights[hyperedgeCount]);
                numEdges ++;
            }
            // add edges from node to e1, and from e2 to node to edge set
            graphEdgeSet[mapping[eind[i]]].push_back(graphNodes[e1]);
            graphEdgeWeightSet[mapping[eind[i]]].push_back(1);
            numEdges ++;
            graphEdgeSet[graphNodes[e2]].push_back(mapping[eind[i]]);
            graphEdgeWeightSet[graphNodes[e2]].push_back(1);
            numEdges ++;

            hyperedgeNodeCount++;
        }
    }

    // build graph from lawler network nodes and edges
    graph_access S;
    S.start_construction(graphNodes.size(), numEdges);
    for (NodeID u = 0; u < graphNodes.size(); ++u) {
            S.new_node();
            S.setPartitionIndex(u, updatedPartitionID[u]);
            S.setNodeWeight(u, 1);

        for(int j = 0; j < graphEdgeSet[u].size(); j++) {
            const EdgeID e = S.new_edge(u, graphEdgeSet[u][j]);
            S.setEdgeWeight(e, graphEdgeWeightSet[u][j]);
        }
    }
    S.finish_construction();

    // create flow graph from graph 
    flow_graph fG; 
    NodeID n = 0; // number of nodes in mqi lawler
    std::vector<bool> isIncluded(S.number_of_nodes(), false); //boolean vector to test if node from subgraph is in cluster

    forall_nodes(S, node){
        if (S.getPartitionIndex(node) != 1-clusterBlock) { 
            isIncluded[node] = true;
            n++;
		}
    }endfor

    n += 2; //size of directed graph for model including source and sink nodes
    fG.start_construction(n); // initialize directed flow graph

    NodeID source = n-2;
    NodeID sink   = n-1;
    int idx = 0;
    NodeID sourceID, targetID;
    NodeID internalEdgeCapacity, sourceEdgeCapacity, sinkEdgeCapacity;
    
    int c = 0; //number of cut edges
    std::vector<NodeID> nodeMapping(S.number_of_nodes(), -1);
    std::vector<NodeID> sourceConnectedNodes(S.number_of_nodes(), 0);
    std::vector<NodeID> connectedToSourceNodes(S.number_of_nodes(), 0);

    // add nodes in cluster to flow network, as well as directed edge from source to cluster node

    NodeID current_node_degree = 0;
    NodeID infinite_capacity = hypergraph_cut * min_volume;

    forall_nodes(S, node) {
        if(S.getPartitionIndex(node) != 1-clusterBlock) {
            if(nodeMapping[node]!=-1){
                    sourceID = nodeMapping[node];
                }
            else{
                nodeMapping[node] = idx;
                idx++;
                sourceID = nodeMapping[node];
            }
        }
        forall_out_edges(S, e, node) {
            NodeID target = S.getEdgeTarget(e);
            if(S.getPartitionIndex(target) != 1-clusterBlock) {
                if(nodeMapping[target]!=-1){
                    targetID = nodeMapping[target];
                }
                else{
                    nodeMapping[target] = idx;
                    idx++;
                    targetID = nodeMapping[target];
                }
            }

            if(node >= lastOriginalPointer){ //edge source is artificial
                if(target >= lastOriginalPointer) { //edge target is also artificial
                    fG.new_edge(sourceID, targetID, min_volume*S.getEdgeWeight(e));
                }
                else{
                    if(S.getPartitionIndex(target) != 1-clusterBlock) { // edge source is artificial and target is OG cluster node
                        fG.new_edge(sourceID, targetID, infinite_capacity);
                    }
                    else{ // edge source is artificial and target is out of cluster OG/ source node 
                        connectedToSourceNodes[node] += infinite_capacity;
                    }
                }
            }
            else{ //edge source is original node
                if (S.getPartitionIndex(node) != 1-clusterBlock){ //node is inside cluster, not collapsed into source
                    if(target >= lastOriginalPointer) {
                        fG.new_edge(sourceID, targetID, infinite_capacity);
                    }
                    else if(S.getPartitionIndex(target) == 1-clusterBlock) { //edge source is OG cluster node and edge targete is OG out of cluster/ source node
                        connectedToSourceNodes[node] += infinite_capacity;
                    }
                }
                else { // the edge source node is to be contracted into s
                    sourceConnectedNodes[target] += infinite_capacity;
                }
            }
        }endfor
    }endfor

    // add source and sink edges
    forall_nodes(S, node) {
        sourceID = nodeMapping[node];
        if(node < lastOriginalPointer) {
            if(S.getPartitionIndex(node) != 1-clusterBlock){ // add edges from cluster nodes to sink with capacity c * node degree
                current_node_degree = S.getNodeDegree(node);
                sinkEdgeCapacity = hypergraph_cut * current_node_degree;
                if (node == 0) {
                    if(fix_seed_node && noMotif == false) {
                        fG.new_edge(sourceID, sink, infinite_capacity);
                    }
                    else {
                        fG.new_edge(sourceID, sink, sinkEdgeCapacity);
                    }
                }
                else{
                    fG.new_edge(sourceID, sink, sinkEdgeCapacity);
                }
            }
        }
        // add source to cluster edge
        if(sourceConnectedNodes[node]!=0){
            fG.new_edge(source, sourceID, sourceConnectedNodes[node]);
        }

        // add cluster to source edge
        if(connectedToSourceNodes[node]!=0){
            fG.new_edge(sourceID, source, connectedToSourceNodes[node]);
        }
    }endfor
    
    push_relabel pr;
    std::vector< NodeID > source_set;
    FlowType flowvalue = pr.solve_max_flow_min_cut(fG, source, sink, true, source_set);

    // map back from flow network to created graph S
    std::vector<NodeID> revMapping(idx);
    for(int i = 0; i < nodeMapping.size(); i++) {
        if(nodeMapping[i]!=-1) {
            revMapping[nodeMapping[i]] = i;
        }
    }

    // assign source connected nodes to outside cluster
    for(int i = 0; i < source_set.size(); i++) {
        if(source_set[i]==source){
            continue;
        }
        S.setPartitionIndex(revMapping[source_set[i]], 1-clusterBlock); 
    }

    // if seed node removed from cluster, send it back
    if(!fix_seed_node) {
        if(noMotif == false){ 
            if(S.getPartitionIndex(0) == 1-clusterBlock){
                S.setPartitionIndex(0, clusterBlock);
            }
        }
    }
    
    // compute revised motif conductance for the hypergraph and update hypergraph vectors for next run

    // Step 1 - Revise the partition IDs
    for(int i = 0; i < lastOriginalPointer; i++) {
        partition[reverse_mapping[i]] = S.getPartitionIndex(i);
    }

    // Step 2 - Compute block volumes
    std::vector<NodeID> volumes(2,0);
    for(int i = 0; i < partition.size(); i++){
            volumes[partition[i]] += hypernode_degrees[i];
    }
    NodeID minVolume = volumes[clusterBlock];

    // Step 3 - Recompute hypergraph cut, update isCutHyperedge, isTriangleHyperedge, isIncludedHyperedge
    int hypergraph_cut_new = 0;
    hyperedgeNodeCount = 0;
    hyperedgeCount = 0;
    graphNodeCount = 0;
    graphEdgeCount = 0;

    for(int i = 0; i < contracted_hyperedge_weights.size(); i++) {
        isCutHyperedge[i] = false;
        isTriangleHyperedge[i] = false;
        isIncludedHyperedge[i] = false;
    }

    std::vector<bool> blockFound(2);
    for(int i = 0; i < eind.size(); i++) {
        if(i == eptr[hyperedgeCount+1]){
                if(blockFound[0] && blockFound[1]) {
                        hypergraph_cut_new += contracted_hyperedge_weights[hyperedgeCount];
                        isCutHyperedge[hyperedgeCount] = true;
                }
                if(hyperedgeNodeCount==3) {
                        isTriangleHyperedge[hyperedgeCount] = true;
                }
                if(blockFound[clusterBlock]) {
                        isIncludedHyperedge[hyperedgeCount] = true;
                        if(hyperedgeNodeCount==3){
                                graphNodeCount = graphNodeCount + 4;
                                graphEdgeCount = graphEdgeCount + 3;
                        }
                        else{
                                graphNodeCount = graphNodeCount + 3;
                                graphEdgeCount = graphEdgeCount + 2;
                        }
                }
                hyperedgeNodeCount = 0;
                blockFound[0] = false;
                blockFound[1] = false;
                hyperedgeCount++;
        }
        blockFound[partition[eind[i]]] = true;
        hyperedgeNodeCount ++;
    }

    // repeat for last hyperedge
    if(blockFound[0] && blockFound[1]) {
            hypergraph_cut_new += contracted_hyperedge_weights[hyperedgeCount];
            isCutHyperedge[hyperedgeCount] = true; 
    }
    if(hyperedgeNodeCount==3) {
            isTriangleHyperedge[hyperedgeCount] = true;
    }
    if(blockFound[clusterBlock]) {
            isIncludedHyperedge[hyperedgeCount] = true;
            if(hyperedgeNodeCount==3){
                    graphNodeCount = graphNodeCount + 4;
                    graphEdgeCount = graphEdgeCount + 3;
            }
            else{
                    graphNodeCount = graphNodeCount + 3;
                    graphEdgeCount = graphEdgeCount + 2;
            }
    }
    
    hyperedgeNodeCount = 0;
    blockFound[0] = false;
    blockFound[1] = false;
    hyperedgeCount++;
    
    // Step 4 - Recompute motif conductance, update cut and min volume
    double motif_conductance; 
    motif_conductance = (double)hypergraph_cut_new/(double)minVolume;
    min_volume = min_volume;
    hypergraph_cut = hypergraph_cut_new;

    return motif_conductance;
}

double mqi::mqi_improvement_hypergraph_ball_star(std::vector<NodeID> & eind, std::vector<size_t> & eptr, NodeID num_vertices, NodeID num_hyperedges,
                                        int clusterBlock, std::vector<NodeID> & partition, std::vector<bool> & isCutHyperedge, 
                                        std::vector<bool> & isTriangleHyperedge, int &graphNodeCount, int &graphEdgeCount, std::vector<bool> & isIncludedHyperedge, bool noMotif, std::vector<int> & hypernode_degrees,
                                        std::vector<NodeID> & contracted_hyperedge_weights, int &hypergraph_cut, NodeID &min_volume,std::vector<NodeID> & hyperEdgeBlock, bool fix_seed_node){
                                        
    /*--------------------------------------------------------------------------- 
            perform mqi improvement on hypergraph using Star expansion
    -----------------------------------------------------------------------------*/ 

    int hyperedgeCount = 0;
    std::vector<NodeID> mapping(num_vertices, -1);
    std::vector<NodeID> reverse_mapping(num_vertices, -1);
    std::vector<NodeID> graphNodes;
    std::vector<NodeID> updatedPartitionID;
    std::vector<std::vector<NodeID>> graphEdgeSet;
    std::vector<std::vector<NodeID>> graphEdgeWeightSet;
    
    int mapIndex = 0;
    int hyperedgeNodeCount = 0;
    int includedEdgeCount = 0;
    int numArtificialNodes = 0;

    // remapping nodes of the included hyperedges
    for(int i = 0; i < eind.size(); i++) {
        if(i == eptr[hyperedgeCount+1]){
                hyperedgeCount++;
                hyperedgeNodeCount = 0;
        }
        if(isIncludedHyperedge[hyperedgeCount]){
            // how many artificial nodes needed
            if(hyperedgeNodeCount == 0) {
                numArtificialNodes ++;
            }
            // remapping
            if(mapping[eind[i]]==-1){
                mapping[eind[i]] = mapIndex;
                reverse_mapping[mapIndex] = eind[i];
                graphNodes.push_back(eind[i]);
                updatedPartitionID.push_back(partition[eind[i]]);
                mapIndex++;
            }
            hyperedgeNodeCount++;
        }
    }

    int lastOriginalPointer = mapIndex; // pointer to last node that is actually in the hypergraph

    hyperedgeCount = 0;
    hyperedgeNodeCount = 0;
    int starNodePointer = 0;
    graphEdgeSet.resize(numArtificialNodes + graphNodes.size());
    graphEdgeWeightSet.resize(numArtificialNodes + graphNodes.size());
    int numEdges = 0;

    // building graph nodes and edges set for flow network
    for(int i = 0; i < eind.size(); i++) {
        if(i == eptr[hyperedgeCount+1]){
                hyperedgeCount++;
                hyperedgeNodeCount = 0;
        }
        // if hyperedge is included in the cluster
        if(isIncludedHyperedge[hyperedgeCount]){
            if(hyperedgeNodeCount == 0) {
                graphNodes.push_back(mapIndex);
                if(!isCutHyperedge[hyperedgeCount]){ // if not cut hyperedge, include artificial node in cluster
                    updatedPartitionID.push_back(clusterBlock);
                }
                else{ //if cut hyperedge, put artificial node where more neighbors are
                    updatedPartitionID.push_back(hyperEdgeBlock[hyperedgeCount]);
                }
                starNodePointer = mapIndex;
                mapIndex++;
            }
            // add edges from hypergraph node to star node and back
            graphEdgeSet[mapping[eind[i]]].push_back(graphNodes[starNodePointer]);
            graphEdgeWeightSet[mapping[eind[i]]].push_back(contracted_hyperedge_weights[hyperedgeCount]);
            numEdges ++;
            graphEdgeSet[graphNodes[starNodePointer]].push_back(mapping[eind[i]]);
            graphEdgeWeightSet[graphNodes[starNodePointer]].push_back(contracted_hyperedge_weights[hyperedgeCount]);
            numEdges ++;

            hyperedgeNodeCount++;
        }
    }

    // build graph from star expansion nodes and edges
    graph_access S;
    S.start_construction(graphNodes.size(), numEdges);
    for (NodeID u = 0; u < graphNodes.size(); ++u) {
            S.new_node();
            S.setPartitionIndex(u, updatedPartitionID[u]);
            S.setNodeWeight(u, 1);

        for(int j = 0; j < graphEdgeSet[u].size(); j++) {
            const EdgeID e = S.new_edge(u, graphEdgeSet[u][j]);
            S.setEdgeWeight(e, graphEdgeWeightSet[u][j]);
        }

    }
    S.finish_construction();

    // create flow graph from graph 
    flow_graph fG; 
    NodeID n = 0; // number of nodes in mqi star
    std::vector<bool> isIncluded(S.number_of_nodes(), false); //boolean vector to test if node from subgraph is in cluster

    forall_nodes(S, node){
        if (S.getPartitionIndex(node) != 1-clusterBlock) {
            isIncluded[node] = true;
            n++;
		}
    }endfor

    n += 2; //size of directed graph for model including source and sink nodes
    fG.start_construction(n); // initialize directed flow graph

    NodeID source = n-2;
    NodeID sink   = n-1;
    int idx = 0;
    NodeID sourceID, targetID;
    NodeID internalEdgeCapacity, sourceEdgeCapacity, sinkEdgeCapacity;
    
    int c = 0; //number of cut edges
    std::vector<NodeID> nodeMapping(S.number_of_nodes(), -1);
    std::vector<NodeID> sourceConnectedNodes(graphNodes.size(), 0);
    NodeID current_node_degree = 0;

    // add nodes in cluster to directed graph

    forall_nodes(S, node) {
        if(isIncluded[node]){
            if(nodeMapping[node]!=-1){
                sourceID = nodeMapping[node];
            }
            else{
                nodeMapping[node] = idx;
                idx++;
                sourceID = nodeMapping[node];
            }
            forall_out_edges(S, e, node) {
                NodeID target = S.getEdgeTarget(e);
                if(isIncluded[target]){ // if both nodes of edge in obtained cluster, add directed edge
                    if(nodeMapping[target]!=-1){
                        targetID = nodeMapping[target];
                    }
                    else{
                        nodeMapping[target] = idx;
                        idx++;
                        targetID = nodeMapping[target];
                    }
                    internalEdgeCapacity = S.getEdgeWeight(e)*min_volume;
                    fG.new_edge(sourceID, targetID, internalEdgeCapacity);
                }
                else{
                    sourceEdgeCapacity = S.getEdgeWeight(e);
                    sourceConnectedNodes[sourceID]+=sourceEdgeCapacity;
                    c += sourceEdgeCapacity; 
                }
            }endfor
        }        
    }endfor

    // add source to cluster node edges - allows for contracting such edges
    for(int i = 0; i < sourceConnectedNodes.size(); i++){
        if(sourceConnectedNodes[i]!=0){
            fG.new_edge(source, i, sourceConnectedNodes[i]*min_volume);
        }
    }

    NodeID infinite_capacity = hypergraph_cut * min_volume;

    // add edges from cluster nodes to sink with capacity c
    forall_nodes(S, node) {
        if(isIncluded[node] && node < lastOriginalPointer){
            sourceID = nodeMapping[node];
            current_node_degree = S.getNodeDegree(node);
            sinkEdgeCapacity = hypergraph_cut*current_node_degree;
            if (node == 0) {
                if (fix_seed_node && noMotif == false) {
                    fG.new_edge(sourceID, sink, infinite_capacity);
                }
                else {
                    fG.new_edge(sourceID, sink, sinkEdgeCapacity);
                }
            }
            else {
                fG.new_edge(sourceID, sink, sinkEdgeCapacity);
            }
        }
    }endfor

    push_relabel pr;
    std::vector< NodeID > source_set;
    FlowType flowvalue = pr.solve_max_flow_min_cut(fG, source, sink, true, source_set);

    // map back from flow network to created graph S
    std::vector<NodeID> revMapping(idx);
    for(int i = 0; i < nodeMapping.size(); i++) {
        if(nodeMapping[i]!=-1) {
            revMapping[nodeMapping[i]] = i;
        }
    }

    // assign source connected nodes to outside cluster
    for(int i = 0; i < source_set.size(); i++) {
        if(source_set[i]==source){
            continue;
        }
        S.setPartitionIndex(revMapping[source_set[i]], 1-clusterBlock); 
    }

    // if seed node removed from cluster, send it back
    if(!fix_seed_node) {
        if(noMotif == false){ 
            if(S.getPartitionIndex(0) == 1-clusterBlock){
                S.setPartitionIndex(0, clusterBlock);
            }
        }
    }
    
    // compute revised motif conductance for the hypergraph and update hypergraph vectors for next run

    //Step 1 - Revise the partition IDs
    for(int i = 0; i < lastOriginalPointer; i++) {
        partition[reverse_mapping[i]] = S.getPartitionIndex(i);
    }

    // Step 2 - Compute block volumes
    std::vector<NodeID> volumes(2,0);
    for(int i = 0; i < partition.size(); i++){
            volumes[partition[i]] += hypernode_degrees[i];
    }
    NodeID minVolume = volumes[clusterBlock];

    // Step 3 - Recompute hypergraph cut, update isCutHyperedge, isTriangleHyperedge, isIncludedHyperedge, hyperEdgeBlock
    int hypergraph_cut_new = 0;
    hyperedgeNodeCount = 0;
    hyperedgeCount = 0;
    graphNodeCount = 0;
    graphEdgeCount = 0;
    std::vector<NodeID> blockCount(2,0);

    for(int i = 0; i < contracted_hyperedge_weights.size(); i++) {
        isCutHyperedge[i] = false;
        isTriangleHyperedge[i] = false;
        isIncludedHyperedge[i] = false;
        hyperEdgeBlock[i] = 0;
    }

    std::vector<bool> blockFound(2);
    for(int i = 0; i < eind.size(); i++) {
        if(i == eptr[hyperedgeCount+1]){
                if(blockFound[0] && blockFound[1]) {
                        hypergraph_cut_new += contracted_hyperedge_weights[hyperedgeCount];
                        isCutHyperedge[hyperedgeCount] = true;
                }
                if(hyperedgeNodeCount==3) {
                        isTriangleHyperedge[hyperedgeCount] = true;
                }
                if(blockFound[clusterBlock]) {
                        isIncludedHyperedge[hyperedgeCount] = true;
                        if(hyperedgeNodeCount==3){
                                graphNodeCount = graphNodeCount + 4;
                                graphEdgeCount = graphEdgeCount + 3;
                        }
                        else{
                                graphNodeCount = graphNodeCount + 3;
                                graphEdgeCount = graphEdgeCount + 2;
                        }
                }
                if(blockCount[0]==blockCount[1]){
                        hyperEdgeBlock[hyperedgeCount] = clusterBlock;
                }
                else if(blockCount[0]>blockCount[1]) {
                        hyperEdgeBlock[hyperedgeCount] = 0;
                }
                else {
                        hyperEdgeBlock[hyperedgeCount] = 1;
                }
                hyperedgeNodeCount = 0;
                blockFound[0] = false;
                blockFound[1] = false;
                blockCount[0] = 0;
                blockCount[1] = 0;
                hyperedgeCount++;
        }
        blockFound[partition[eind[i]]] = true;
        blockCount[partition[eind[i]]]++;
        hyperedgeNodeCount ++;
    }

    // repeat for last hyperedge
    if(blockFound[0] && blockFound[1]) {
            hypergraph_cut_new += contracted_hyperedge_weights[hyperedgeCount];
            isCutHyperedge[hyperedgeCount] = true; 
    }
    if(hyperedgeNodeCount==3) {
            isTriangleHyperedge[hyperedgeCount] = true;
    }
    if(blockFound[clusterBlock]) {
            isIncludedHyperedge[hyperedgeCount] = true;
            if(hyperedgeNodeCount==3){
                    graphNodeCount = graphNodeCount + 4;
                    graphEdgeCount = graphEdgeCount + 3;
            }
            else{
                    graphNodeCount = graphNodeCount + 3;
                    graphEdgeCount = graphEdgeCount + 2;
            }
    }
    if(blockCount[0]==blockCount[1]){
            hyperEdgeBlock[hyperedgeCount] = clusterBlock;
    }
    else if(blockCount[0]>blockCount[1]) {
            hyperEdgeBlock[hyperedgeCount] = 0;
    }
    else {
            hyperEdgeBlock[hyperedgeCount] = 1;
    }
    hyperedgeNodeCount = 0;
    blockFound[0] = false;
    blockFound[1] = false;
    blockCount[0] = 0;
    blockCount[1] = 0;
    hyperedgeCount++;
    
    // Step 4 - Recompute motif conductance, update cut and min volume
    double motif_conductance; 
    motif_conductance = (double)hypergraph_cut_new/(double)minVolume;
    min_volume = min_volume;
    hypergraph_cut = hypergraph_cut_new;

    return motif_conductance;
}
