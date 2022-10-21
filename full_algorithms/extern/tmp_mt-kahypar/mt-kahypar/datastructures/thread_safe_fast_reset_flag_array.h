/*******************************************************************************
 * This file is part of KaHyPar.
 *
 * Copyright (C) 2015-2016 Sebastian Schlag <sebastian.schlag@kit.edu>
 * Copyright (C) 2020 Tobias Heuer <tobias.heuer@kit.edu>
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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

#include "mt-kahypar/macros.h"
#include "mt-kahypar/parallel/atomic_wrapper.h"

// based on http://upcoder.com/9/fast-resettable-flag-vector/

namespace mt_kahypar {
namespace ds {
template <typename Type = std::uint16_t>
class ThreadSafeFastResetFlagArray {

public:
  explicit ThreadSafeFastResetFlagArray(const size_t size) :
    _v(std::make_unique<Type[]>(size)),
    _threshold(1),
    _size(size) {
    initialize();
  }

  ThreadSafeFastResetFlagArray() :
    _v(nullptr),
    _threshold(1),
    _size(0) { }

  ThreadSafeFastResetFlagArray(const ThreadSafeFastResetFlagArray&) = delete;
  ThreadSafeFastResetFlagArray& operator= (const ThreadSafeFastResetFlagArray&) = delete;

  ThreadSafeFastResetFlagArray(ThreadSafeFastResetFlagArray&&) = default;
  ThreadSafeFastResetFlagArray& operator= (ThreadSafeFastResetFlagArray&&) = default;

  ~ThreadSafeFastResetFlagArray() = default;

  void swap(ThreadSafeFastResetFlagArray& other) {
    using std::swap;
    swap(_v, other._v);
    swap(_threshold, other._threshold);
  }

  bool operator[] (const size_t i) const {
    return isSet(i);
  }

  // ! Changes value of entry i from false to true and returns true, if the value
  // ! hold on position i was false and was successfully set to true
  bool compare_and_set_to_true(const size_t i) {
    Type expected = __atomic_load_n(&_v[i], __ATOMIC_RELAXED);
    Type desired = _threshold;
    if ( expected != _threshold &&
        __atomic_compare_exchange(&_v[i], &expected, &desired,
          false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED) ) {
      // Value was successfully set from false to true
      return true;
    } else {
      // Either expected == _threshold or compare_exchange_strong failed, which means that
      // an other thread set _v[i] to true before.
      return false;
    }
  }

  void set(const size_t i, const bool value) {
    __atomic_store_n(&_v[i], value ? _threshold : 0, __ATOMIC_RELAXED);
  }

  void setUnsafe(const size_t i, const bool value) {
   _v[i] = value ? _threshold : 0;
  }

  void reset() {
    if (_threshold == std::numeric_limits<Type>::max()) {
      initialize();
      _threshold = 0;
    }
    ++_threshold;
  }

  void setSize(const size_t size, const bool init = false) {
    ASSERT(_v == nullptr, "Error");
    _v = std::make_unique<Type[]>(size);
    _size = size;
    initialize(init);
  }

  void resize(const size_t size, const bool init = false) {
    if ( size > _size ) {
      std::unique_ptr<Type[]> tmp_v =
        std::make_unique<Type[]>(size);
      std::swap(_v, tmp_v);
      _size = size;
      initialize(init);
    } else {
      _size = size;
    }
  }

 private:
  bool isSet(size_t i) const {
    return __atomic_load_n(&_v[i], __ATOMIC_RELAXED) == _threshold;
  }

  void initialize(const bool init = false) {
    const Type init_value = init ? _threshold : 0;
    for ( size_t i = 0; i < _size; ++i ) {
      __atomic_store_n(&_v[i], init_value, __ATOMIC_RELAXED);
    }
  }

  std::unique_ptr<Type[]> _v;
  Type _threshold;
  size_t _size;
};

template <typename Type>
void swap(ThreadSafeFastResetFlagArray<Type>& a,
          ThreadSafeFastResetFlagArray<Type>& b) {
  a.swap(b);
}
}  // namespace ds
}  // namespace mt_kahypar
