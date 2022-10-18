/******************************************************************************
 * kway_graph_refinement_commons.cpp 
 * *
 * 
 * XXX <XXX.phone@gmail.com>
 *****************************************************************************/

#include <omp.h>

#include "kway_graph_refinement_commons.h"

kway_graph_refinement_commons::kway_graph_refinement_commons( PartitionConfig & config ) {
        init(config);
}

kway_graph_refinement_commons::~kway_graph_refinement_commons() {
}

