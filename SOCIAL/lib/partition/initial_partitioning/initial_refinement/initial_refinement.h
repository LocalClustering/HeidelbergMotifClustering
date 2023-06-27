/******************************************************************************
 * initial_refinement.h 
 * *
 * XXX.
 * XXX
 *****************************************************************************/

#ifndef INITIAL_REFINEMENT_LDIIF5CG
#define INITIAL_REFINEMENT_LDIIF5CG

#include "data_structure/graph_access.h"
#include "partition_config.h"

class initial_refinement {
public:
        initial_refinement( );
        virtual ~initial_refinement();

        int optimize( const PartitionConfig & config, 
                      graph_access & G, 
                      EdgeWeight & initial_cut); 
};


#endif /* end of include guard: INITIAL_REFINEMENT_LDIIF5CG */
