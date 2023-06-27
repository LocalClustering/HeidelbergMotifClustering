/******************************************************************************
 * triangle_listing.h 
 * *
 *****************************************************************************/

#ifndef TRIANGLE_LISTING_H
#define TRIANGLE_LISTING_H

#include <stack>
#include <vector>

#include "data_structure/graph_access.h"
#include "definitions.h"


class triangle_listing {
public:
        triangle_listing();
        virtual ~triangle_listing();

        void write_triangles(NodeID n, std::vector<std::vector<NodeID>> triangles, std::string filename);
        void write_triangles_weighted(NodeID n, std::vector<std::vector<NodeID>> *triangles, std::string filename, NodeID totalSize, int contractedIndex);
        std::vector <std::vector<NodeID>>* triangle_run(graph_access &G, NodeID seed_node, float epsilon_depth, std::vector<bool> &lastLayerNodes, NodeID &hypergraph_node_count, std::vector <NodeID> *&mapping2);
	long long triangle_run_graph(graph_access &G, std::vector<bool> &lastLayerNodes);
};


#endif /* end of include guard: TRIANGLE_LISTING_H */
