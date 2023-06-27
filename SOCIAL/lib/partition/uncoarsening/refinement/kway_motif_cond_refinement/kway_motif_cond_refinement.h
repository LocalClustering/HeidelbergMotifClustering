/******************************************************************************
 * kway_motif_cond_refinement.h 
 *
 * XXX.
 * XXX
 *****************************************************************************/

#ifndef KWAY_GRAPH_REFINEMENT_PVGY97EW
#define KWAY_GRAPH_REFINEMENT_PVGY97EW

#include <vector>

#include "data_structure/priority_queues/priority_queue_interface.h"
#include "definitions.h"
#include "kway_motif_cond_refinement_commons.h"
#include "random_functions.h"
#include "uncoarsening/refinement/quotient_graph_refinement/2way_fm_refinement/vertex_moved_hashtable.h"
#include "uncoarsening/refinement/refinement.h"

class kway_motif_cond_refinement : public refinement {
        public:
                kway_motif_cond_refinement( );
                virtual ~kway_motif_cond_refinement();

                EdgeWeight perform_refinement(PartitionConfig & config, 
					      std::vector<NodeID>& fixed_nodes, 
                                              graph_access & G, 
					      PartitionID block_artif_node,
                                              complete_boundary & boundary);

                void setup_start_nodes(PartitionConfig & config, 
                                       graph_access & G, 
                                       complete_boundary & boundary,  
                                       boundary_starting_nodes & start_nodes);
};

#endif /* end of include guard: KWAY_GRAPH_REFINEMENT_PVGY97EW */

