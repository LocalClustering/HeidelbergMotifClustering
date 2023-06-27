/******************************************************************************
 * gal_combine.h 
 * *
 * 
 * XXX <XXX.phone@gmail.com>
 *****************************************************************************/

#ifndef GAL_COMBINE_XDMU5YB7
#define GAL_COMBINE_XDMU5YB7

#include "partition_config.h"
#include "data_structure/graph_access.h"

class gal_combine {
public:
        gal_combine();
        virtual ~gal_combine();

        void perform_gal_combine( PartitionConfig & config, graph_access & G);
};


#endif /* end of include guard: GAL_COMBINE_XDMU5YB7 */
