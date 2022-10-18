/******************************************************************************
 * kway_motif_cond_refinement_commons.cpp 
 * *
 * 
 * XXX <XXX.phone@gmail.com>
 *****************************************************************************/

#include <omp.h>

#include "kway_motif_cond_refinement_commons.h"

kway_motif_cond_refinement_commons::kway_motif_cond_refinement_commons( PartitionConfig & config ) {
        init(config);
}

kway_motif_cond_refinement_commons::~kway_motif_cond_refinement_commons() {
}

