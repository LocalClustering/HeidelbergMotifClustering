/******************************************************************************
 * mixed_refinement.h 
 * *
 * XXX.
 * XXX
 *****************************************************************************/

#ifndef MIXED_REFINEMENT_XJC6COP3
#define MIXED_REFINEMENT_XJC6COP3

#include "definitions.h"
#include "refinement.h"

class mixed_refinement : public refinement {
public:
        mixed_refinement( );
        virtual ~mixed_refinement();

        virtual EdgeWeight perform_refinement(PartitionConfig & config, 
                                              graph_access & G, 
                                              complete_boundary & boundary); 
};


#endif /* end of include guard: MIXED_REFINEMENT_XJC6COP3 */
