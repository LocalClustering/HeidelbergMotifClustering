/******************************************************************************
 * random_matching.h 
 * *
 * XXX.
 * XXX
 *****************************************************************************/

#ifndef RANDOM_MATCHING_D5YDSMDW
#define RANDOM_MATCHING_D5YDSMDW

#include "matching.h"

class random_matching : public matching {
        public:
                random_matching();
                virtual ~random_matching();

                void match(const PartitionConfig & config, 
                           graph_access & G, 
                           Matching & _matching, 
                           CoarseMapping & coarse_mapping, 
                           NodeID & no_of_coarse_vertices,
                           NodePermutationMap & permutation);
};

#endif /* end of include guard: RANDOM_MATCHING_D5YDSMDW */
