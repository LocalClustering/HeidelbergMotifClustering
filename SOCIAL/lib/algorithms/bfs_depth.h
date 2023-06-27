/******************************************************************************
 * bfs_depth.h 
 * *
 * XXX.
 * XXX
 *****************************************************************************/

#ifndef BFS_DEPTH_H_
#define BFS_DEPTH_H_

#include <stack>
#include <vector>

#include "data_structure/graph_access.h"
#include "definitions.h"

class bfs_depth {
public:
        bfs_depth();
        virtual ~bfs_depth();

        void write_subgraph(std::vector<std::vector<NodeID>> subgraph_edges, int nodeCount, int edgeCount, std::string filename);
        void write_lastlayer(std::vector<NodeID> mapping, std::vector<NodeID> last, int NodeCount, std::string filename);
	graph_access* runBFS(graph_access & G, NodeID seed_node, short int & epsilon_depth, std::vector<NodeID> &mapping, std::vector<NodeID> *&hypergraph_map, std::vector<bool> &lastLayerNodes, std::vector<short int>& level, std::vector<bool>& visited, std::vector<bool>& touched, bool force_over_100);
	graph_access* runBFS_graph(graph_access & G, std::vector<NodeID>& seed_nodes, short int & epsilon_depth, std::vector<NodeID> &mapping, std::vector<NodeID> *&hypergraph_map, std::vector<bool> &lastLayerNodes, std::vector<short int>& level, std::vector<bool>& visited, std::vector<bool>& touched, bool force_over_100);
        graph_access* runBFS_evaluator(graph_access & G, std::vector<NodeID>& seed_nodes, short int epsilon_depth, std::vector<NodeID> &mapping, std::vector<NodeID> *&hypergraph_map, std::vector<bool> &lastLayerNodes, std::vector<short int>& level, std::vector<bool>& visited, std::vector<bool>& touched, bool force_over_100);
};


#endif /* end of include guard: BFS_DEPTH_H_ */
