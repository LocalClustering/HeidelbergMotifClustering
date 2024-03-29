/******************************************************************************
 * initial_partitioning.h 
 * *
 * XXX.
 * XXX
 *****************************************************************************/

#ifndef INITIAL_PARTITIONING_D7VA0XO9
#define INITIAL_PARTITIONING_D7VA0XO9

#include "data_structure/graph_hierarchy.h"
#include "partition_config.h"

class initial_partitioning {
public:
        initial_partitioning( );
        virtual ~initial_partitioning();
        void perform_initial_partitioning(const PartitionConfig & config, graph_hierarchy & hierarchy);
        void perform_initial_partitioning(const PartitionConfig & config, graph_access &  G);
        void perform_initial_partitioning_separator(const PartitionConfig & config, graph_access &  G);
};


#endif /* end of include guard: INITIAL_PARTITIONING_D7VA0XO9 */
