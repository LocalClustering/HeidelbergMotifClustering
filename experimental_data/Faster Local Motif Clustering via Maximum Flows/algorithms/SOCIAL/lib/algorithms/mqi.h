/******************************************************************************
 * mqi.h 
 * *
 * XXX.
 * XXX
 *****************************************************************************/

#ifndef MQI_H_
#define MQI_H_

#include <stack>
#include <vector>

#include "data_structure/graph_access.h"
#include "definitions.h"
#include "quality_metrics.h"

class mqi {
public:
        mqi();
        virtual ~mqi();
        
        void mqi_improvement(graph_access & S, bool fix_seed_node);
        
        void mqi_improvement_ball(graph_access & S, bool fix_seed_node);

        double mqi_improvement_hypergraph_ball_lawler(std::vector<NodeID> & eind, std::vector<size_t> & eptr, 
                                        NodeID num_vertices, NodeID num_hyperedges, 
                                        int clusterBlock, std::vector<NodeID> & partition, std::vector<bool> & isCutHyperedge, 
                                        std::vector<bool> & isTriangleHyperedge, int & graphNodeCount, int & graphEdgeCount, std::vector<bool> & isIncludedHyperedge, bool noMotif, std::vector<int> & hypernode_degrees,
                                        std::vector<NodeID> & contracted_hyperedge_weights, int &hypergraph_cut, NodeID &min_volume, bool fix_seed_node);

        double mqi_improvement_hypergraph_ball_star(std::vector<NodeID> & eind, std::vector<size_t> & eptr, NodeID num_vertices, NodeID num_hyperedges,
                                        int clusterBlock, std::vector<NodeID> & partition, std::vector<bool> & isCutHyperedge, 
                                        std::vector<bool> & isTriangleHyperedge, int &graphNodeCount, int &graphEdgeCount, std::vector<bool> & isIncludedHyperedge, bool noMotif, std::vector<int> & hypernode_degrees,
                                        std::vector<NodeID> & contracted_hyperedge_weights, int &hypergraph_cut, NodeID &min_volume,std::vector<NodeID> & hyperEdgeBlock, bool fix_seed_node);
};


#endif /* end of include guard: MQI_H_ */
