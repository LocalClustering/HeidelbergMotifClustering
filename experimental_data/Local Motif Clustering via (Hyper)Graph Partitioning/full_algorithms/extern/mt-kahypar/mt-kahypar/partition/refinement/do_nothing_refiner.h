/*******************************************************************************
 * This file is part of KaHyPar.
 *
 * Copyright (C) 2015 Sebastian Schlag <sebastian.schlag@kit.edu>
 *
 * KaHyPar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KaHyPar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KaHyPar.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "mt-kahypar/partition/refinement/i_refiner.h"

namespace mt_kahypar {
class DoNothingRefiner final : public IRefiner {
 public:
  template <typename ... Args>
  explicit DoNothingRefiner(Args&& ...) noexcept { }
  DoNothingRefiner(const DoNothingRefiner&) = delete;
  DoNothingRefiner(DoNothingRefiner&&) = delete;
  DoNothingRefiner & operator= (const DoNothingRefiner &) = delete;
  DoNothingRefiner & operator= (DoNothingRefiner &&) = delete;

 private:
  void initializeImpl(PartitionedHypergraph&) override final { }

  bool refineImpl(PartitionedHypergraph&,
                  const parallel::scalable_vector<HypernodeID>&,
                  Metrics &,
                  const double) override final {
    return false;
  }
};
}  // namespace kahypar
