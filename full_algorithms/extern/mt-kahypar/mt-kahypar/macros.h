/*******************************************************************************
 * This file is part of Mt-KaHyPar.
 *
 * Copyright (C) 2019 Tobias Heuer <tobias.heuer@kit.edu>
 *
 * Mt-KaHyPar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mt-KaHyPar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mt-KaHyPar.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#pragma once

#include <type_traits>

#include "kahypar/macros.h"

#define SPECIALIZATION(EXPR, TYPE)          \
  template<bool T = EXPR>                   \
  std::enable_if_t<T, TYPE>

#define TRUE_SPECIALIZATION(EXPR, TYPE)     \
  template<bool T = EXPR>                   \
  std::enable_if_t<T, TYPE>

#define FALSE_SPECIALIZATION(EXPR, TYPE)    \
  template<bool T = EXPR>                   \
  std::enable_if_t<!T, TYPE>

#if defined(__GNUC__) || defined(__clang__)
#define MT_KAHYPAR_ATTRIBUTE_ALWAYS_INLINE __attribute__ ((always_inline)) inline
#else
#define MT_KAHYPAR_ATTRIBUTE_ALWAYS_INLINE
#endif

#define HEAVY_ASSERT0(cond) \
  !(enable_heavy_assert) ? (void)0 : [&]() { ASSERT(cond); } ()
#define HEAVY_ASSERT1(cond, msg) \
  !(enable_heavy_assert) ? (void)0 : [&]() { ASSERT(cond, msg); } ()

#ifdef KAHYPAR_ENABLE_HEAVY_PREPROCESSING_ASSERTIONS
  #define HEAVY_PREPROCESSING_ASSERT_1(cond) ASSERT(cond)
  #define HEAVY_PREPROCESSING_ASSERT_2(cond, msg) ASSERT(cond, msg)
#else
  #define HEAVY_PREPROCESSING_ASSERT_1(cond) HEAVY_ASSERT0(cond)
  #define HEAVY_PREPROCESSING_ASSERT_2(cond, msg) HEAVY_ASSERT1(cond, msg)
#endif

#ifdef KAHYPAR_ENABLE_HEAVY_COARSENING_ASSERTIONS
  #define HEAVY_COARSENING_ASSERT_1(cond) ASSERT(cond)
  #define HEAVY_COARSENING_ASSERT_2(cond, msg) ASSERT(cond, msg)
#else
  #define HEAVY_COARSENING_ASSERT_1(cond) HEAVY_ASSERT0(cond)
  #define HEAVY_COARSENING_ASSERT_2(cond, msg) HEAVY_ASSERT1(cond, msg)
#endif

#ifdef KAHYPAR_ENABLE_HEAVY_INITIAL_PARTITIONING_ASSERTIONS
  #define HEAVY_INITIAL_PARTITIONING_ASSERT_1(cond) ASSERT(cond)
  #define HEAVY_INITIAL_PARTITIONING_ASSERT_2(cond, msg) ASSERT(cond, msg)
#else
  #define HEAVY_INITIAL_PARTITIONING_ASSERT_1(cond) HEAVY_ASSERT0(cond)
  #define HEAVY_INITIAL_PARTITIONING_ASSERT_2(cond, msg) HEAVY_ASSERT1(cond, msg)
#endif

#ifdef KAHYPAR_ENABLE_HEAVY_REFINEMENT_ASSERTIONS
  #define HEAVY_REFINEMENT_ASSERT_1(cond) ASSERT(cond)
  #define HEAVY_REFINEMENT_ASSERT_2(cond, msg) ASSERT(cond, msg)
#else
  #define HEAVY_REFINEMENT_ASSERT_1(cond) HEAVY_ASSERT0(cond)
  #define HEAVY_REFINEMENT_ASSERT_2(cond, msg) HEAVY_ASSERT1(cond, msg)
#endif

#define HEAVY_ASSERT_(TYPE, N) HEAVY_ ## TYPE ## _ASSERT_ ## N
#define HEAVY_ASSERT_EVAL(TYPE, N) HEAVY_ASSERT_(TYPE, N)

// Heavy assertions are assertions which increase the complexity of the scope
// which they are executed in by an polynomial factor. In debug mode you are often only
// interested in certain phase of the multilevel paradigm. However, when enabling all assertions
// it can take a while to reach the point which you are really interested in, because heavy assertions
// radicaly downgrade the performance of the application. Therefore such assertions should be packed
// in a heavy assertion macro. Heavy assertions can be enabled via cmake flag for specific phase or for
// specific scope by adding
// static constexpr bool enable_heavy_assert = false;
// to the corresponding scope.
#define HEAVY_PREPROCESSING_ASSERT(...) EXPAND(HEAVY_ASSERT_EVAL(PREPROCESSING, EXPAND(NARG(__VA_ARGS__)))(__VA_ARGS__))
#define HEAVY_COARSENING_ASSERT(...) EXPAND(HEAVY_ASSERT_EVAL(COARSENING, EXPAND(NARG(__VA_ARGS__)))(__VA_ARGS__))
#define HEAVY_INITIAL_PARTITIONING_ASSERT(...) EXPAND(HEAVY_ASSERT_EVAL(INITIAL_PARTITIONING, EXPAND(NARG(__VA_ARGS__)))(__VA_ARGS__))
#define HEAVY_REFINEMENT_ASSERT(...) EXPAND(HEAVY_ASSERT_EVAL(REFINEMENT, EXPAND(NARG(__VA_ARGS__)))(__VA_ARGS__))

// An assertion that triggers for the hypergraph partitioners but not for the graph partitioner
#ifdef USE_GRAPH_PARTITIONER
  #define ASSERT_FOR_HG_ONLY(cond);
#else
  #define ASSERT_FOR_HG_ONLY(cond) ASSERT(cond);
#endif

// Info, Warning and Error Output Macros
#define GREEN "\033[1;92m"
#define CYAN "\033[1;96m"
#define YELLOW "\033[1;93m"
#define RED "\033[1;91m"
#define WHITE "\033[1;97m"
#define BOLD "\033[1m"
#define END "\033[0m"
#define INFO(msg) LOG << CYAN << "[INFO]" << END << msg
#define WARNING(msg) LOG << YELLOW << "[WARNING]" << END << msg
#define ERROR(msg) LOG << RED << "[ERROR]" << END << msg; std::exit(-1)

#ifdef MT_KAHYPAR_LIBRARY_MODE
#define ALGO_SWITCH(warning_msg, error_msg, context_variable, alternative_value) \
  ERROR(error_msg);
#else
#define ALGO_SWITCH(warning_msg, error_msg, context_variable, alternative_value) \
  WARNING(warning_msg);                                                          \
  char answer = 'N';                                                             \
  std::cin >> answer;                                                            \
  answer = std::toupper(answer);                                                 \
  if (answer == 'Y') {                                                           \
    context_variable = alternative_value;                                        \
  } else {                                                                       \
    ERROR(error_msg);                                                            \
  }
#endif

