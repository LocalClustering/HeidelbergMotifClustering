/******************************************************************************
 * label_propagation_refinement.h  
 * *
 * 
 * XXX <XXX.phone@gmail.com>
 *****************************************************************************/


#ifndef LABEL_PROPAGATION_REFINEMENT_R4XW141Y
#define LABEL_PROPAGATION_REFINEMENT_R4XW141Y

#include "definitions.h"
#include "../refinement.h"

class label_propagation_refinement : public refinement {
public:
        label_propagation_refinement();
        virtual ~label_propagation_refinement();

        virtual EdgeWeight perform_refinement(PartitionConfig & config, 
                                              graph_access & G, 
                                              complete_boundary & boundary); 
	virtual EdgeWeight perform_refinement_conductance(PartitionConfig & partition_config, 
				std::vector<NodeID>& fixed_nodes, graph_access & G, PartitionID block_artif_node);
};


#endif /* end of include guard: LABEL_PROPAGATION_REFINEMENT_R4XW141Y */
