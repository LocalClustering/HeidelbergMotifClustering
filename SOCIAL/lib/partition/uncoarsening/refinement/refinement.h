/******************************************************************************
 * refinement.h 
 * *
 * XXX.
 * XXX
 *****************************************************************************/

#ifndef REFINEMENT_UJN9IBHM
#define REFINEMENT_UJN9IBHM

#include "data_structure/graph_access.h"
#include "partition_config.h"
#include "quotient_graph_refinement/complete_boundary.h"

class refinement {
public:
        refinement( );
        virtual ~refinement();
        
        virtual EdgeWeight perform_refinement(PartitionConfig & config, 
                                              graph_access & G, 
                                              complete_boundary & boundary) = 0;
};


#endif /* end of include guard: REFINEMENT_UJN9IBHM */
