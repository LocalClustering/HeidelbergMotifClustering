/******************************************************************************
 * random_functions.cpp 
 * *
 * 
 * XXX <XXX.phone@gmail.com>
 *****************************************************************************/

#include "random_functions.h"

MersenneTwister random_functions::m_mt;
int random_functions::m_seed = 0;

random_functions::random_functions()  {
}

random_functions::~random_functions() {
}
