/******************************************************************************
 * path.cpp 
 * *
 * 
 * XXX <XXX.phone@gmail.com>
 *****************************************************************************/

#include "path.h"

path::path() : head(UNDEFINED_NODE), tail(UNDEFINED_NODE), length(0), active(false) {
                
}

path::path(const NodeID & v) : head(v), tail(v), length(0), active(true) {
                
}

path::~path() {
                
}

