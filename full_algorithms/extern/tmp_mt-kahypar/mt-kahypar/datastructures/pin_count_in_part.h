/*******************************************************************************
 * This file is part of Mt-KaHyPar.
 *
 * Copyright (C) 2019 Lars Gottesbüren <lars.gottesbueren@kit.edu>
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

#include <cmath>


#include "mt-kahypar/macros.h"
#include "mt-kahypar/datastructures/hypergraph_common.h"
#include "mt-kahypar/datastructures/array.h"


namespace mt_kahypar {
namespace ds {

/*!
 * Data structure to store the pin count in a block of the partition
 * per hyperedge in a memory efficient way. The data structure is initialized
 * with the maximum value of a pin count entry (which is the maximum size
 * of a hyperedge). The pin counts are then stored in a compressed form where
 * each entry occupies exactly the number of bits it requires to store the
 * maximum value. To do so, we store several pin count entries in a 64-bit unsigned
 * integer.
 * Note, this data structure is not thread-safe. Updates of a pin count entry
 * of a hyperedge must be done exclusively. Different hyperedges can be updated
 * concurrently.
 */
class PinCountInPart {

  static constexpr bool debug = false;

 public:
  using Value = uint64_t;

  PinCountInPart() :
    _num_hyperedges(0),
    _k(0),
    _max_value(0),
    _bits_per_element(0),
    _entries_per_value(0),
    _values_per_hyperedge(0),
    _extraction_mask(0),
    _pin_count_in_part() { }

  PinCountInPart(const HyperedgeID num_hyperedges,
                 const PartitionID k,
                 const HypernodeID max_value,
                 const bool assign_parallel = true) :
    _num_hyperedges(0),
    _k(0),
    _max_value(0),
    _bits_per_element(0),
    _entries_per_value(0),
    _values_per_hyperedge(0),
    _extraction_mask(0),
    _pin_count_in_part() {
    initialize(num_hyperedges, k, max_value, assign_parallel);
  }

  PinCountInPart(const PinCountInPart&) = delete;
  PinCountInPart & operator= (const PinCountInPart &) = delete;

  PinCountInPart(PinCountInPart&& other) :
    _num_hyperedges(other._num_hyperedges),
    _k(other._k),
    _max_value(other._max_value),
    _bits_per_element(other._bits_per_element),
    _entries_per_value(other._entries_per_value),
    _values_per_hyperedge(other._values_per_hyperedge),
    _extraction_mask(other._extraction_mask),
    _pin_count_in_part(std::move(other._pin_count_in_part)) { }

  PinCountInPart & operator= (PinCountInPart&& other) {
    _num_hyperedges = other._num_hyperedges;
    _k = other._k;
    _max_value = other._max_value;
    _bits_per_element = other._bits_per_element;
    _entries_per_value = other._entries_per_value;
    _values_per_hyperedge = other._values_per_hyperedge;
    _extraction_mask = other._extraction_mask;
    _pin_count_in_part = std::move(other._pin_count_in_part);
    return *this;
  }

  // ! Initializes the data structure
  void initialize(const HyperedgeID num_hyperedges,
                  const PartitionID k,
                  const HypernodeID max_value,
                  const bool assign_parallel = true) {
    ASSERT(_num_hyperedges == 0);
    if ( num_hyperedges > 0 ) {
      _num_hyperedges = num_hyperedges;
      _k = k;
      _max_value = max_value;
      _bits_per_element = num_bits_per_element(max_value);
      _entries_per_value = num_entries_per_value(k, max_value);
      _values_per_hyperedge = num_values_per_hyperedge(k, max_value);
      _extraction_mask = std::pow(2UL, _bits_per_element) - 1UL;
      _pin_count_in_part.resize("Refinement", "pin_count_in_part",
        num_hyperedges * _values_per_hyperedge, true, assign_parallel);
    }
  }

  Array<Value>& data() {
    return _pin_count_in_part;
  }

  // ! Returns the pin count of the hyperedge in the corresponding block
  inline HypernodeID pinCountInPart(const HyperedgeID he,
                                    const PartitionID id) const {
    ASSERT(he < _num_hyperedges);
    ASSERT(id != kInvalidPartition && id < _k);
    const size_t value_pos = he * _values_per_hyperedge + id / _entries_per_value;
    const size_t bit_pos = (id % _entries_per_value) * _bits_per_element;
    const Value mask = _extraction_mask << bit_pos;
    return (_pin_count_in_part[value_pos] & mask) >> bit_pos;
  }

  // ! Sets the pin count of the hyperedge in the corresponding block to value
  inline void setPinCountInPart(const HyperedgeID he,
                                const PartitionID id,
                                const HypernodeID value) {
    ASSERT(he < _num_hyperedges);
    ASSERT(id != kInvalidPartition && id < _k);
    const size_t value_pos = he * _values_per_hyperedge + id / _entries_per_value;
    const size_t bit_pos = (id % _entries_per_value) * _bits_per_element;
    updateEntry(_pin_count_in_part[value_pos], bit_pos, value);
  }

  // ! Increments the pin count of the hyperedge in the corresponding block
  inline HypernodeID incrementPinCountInPart(const HyperedgeID he,
                                             const PartitionID id) {
    ASSERT(he < _num_hyperedges);
    ASSERT(id != kInvalidPartition && id < _k);
    const size_t value_pos = he * _values_per_hyperedge + id / _entries_per_value;
    const size_t bit_pos = (id % _entries_per_value) * _bits_per_element;
    const Value mask = _extraction_mask << bit_pos;
    Value& current_value = _pin_count_in_part[value_pos];
    Value pin_count_in_part = (current_value & mask) >> bit_pos;
    ASSERT(pin_count_in_part + 1 <= _max_value);
    updateEntry(current_value, bit_pos, pin_count_in_part + 1);
    return pin_count_in_part + 1;
  }

  // ! Decrements the pin count of the hyperedge in the corresponding block
  inline HypernodeID decrementPinCountInPart(const HyperedgeID he,
                                             const PartitionID id) {
    ASSERT(he < _num_hyperedges);
    ASSERT(id != kInvalidPartition && id < _k);
    const size_t value_pos = he * _values_per_hyperedge + id / _entries_per_value;
    const size_t bit_pos = (id % _entries_per_value) * _bits_per_element;
    const Value mask = _extraction_mask << bit_pos;
    Value& current_value = _pin_count_in_part[value_pos];
    Value pin_count_in_part = (current_value & mask) >> bit_pos;
    ASSERT(pin_count_in_part > 0UL);
    updateEntry(current_value, bit_pos, pin_count_in_part - 1);
    return pin_count_in_part - 1;
  }

  // ! Returns the size in bytes of this data structure
  size_t size_in_bytes() const {
    return sizeof(Value) * _pin_count_in_part.size();
  }

  static size_t num_elements(const HyperedgeID num_hyperedges,
                             const PartitionID k,
                             const HypernodeID max_value) {
    return num_hyperedges * num_values_per_hyperedge(k, max_value);
  }

 private:
  inline void updateEntry(Value& value,
                          const size_t bit_pos,
                          const Value new_value) {
    ASSERT(new_value <= _max_value);
    const Value zero_mask = ~(_extraction_mask << bit_pos);
    const Value value_mask = new_value << bit_pos;
    value = (value & zero_mask) | value_mask;
  }

  static size_t num_values_per_hyperedge(const PartitionID k,
                                         const HypernodeID max_value) {
    const size_t entries_per_value = num_entries_per_value(k, max_value);
    ASSERT(entries_per_value <= static_cast<size_t>(k));
    return k / entries_per_value + (k % entries_per_value!= 0);
  }

  static size_t num_entries_per_value(const PartitionID k,
                                      const HypernodeID max_value) {
    const size_t bits_per_element = num_bits_per_element(max_value);
    const size_t bits_per_value = sizeof(Value) * 8UL;
    ASSERT(bits_per_element <= bits_per_value);
    return std::min(bits_per_value / bits_per_element, static_cast<size_t>(k));
  }

  static size_t num_bits_per_element(const HypernodeID max_value) {
    return std::ceil(std::log2(static_cast<double>(max_value + 1)));
  }

  HyperedgeID _num_hyperedges;
  PartitionID _k;
  HypernodeID _max_value;
  size_t _bits_per_element;
  size_t _entries_per_value;
  size_t _values_per_hyperedge;
  Value _extraction_mask;
  Array<Value> _pin_count_in_part;
};
}  // namespace ds
}  // namespace mt_kahypar
