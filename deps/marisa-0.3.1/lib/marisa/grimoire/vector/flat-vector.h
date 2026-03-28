#ifndef MARISA_GRIMOIRE_VECTOR_FLAT_VECTOR_H_
#define MARISA_GRIMOIRE_VECTOR_FLAT_VECTOR_H_

#include <cassert>
#include <stdexcept>

#include "marisa/grimoire/vector/vector.h"

namespace marisa::grimoire::vector {

class FlatVector {
 public:
#if MARISA_WORD_SIZE == 64
  using Unit = uint64_t;
#else   // MARISA_WORD_SIZE == 64
  using Unit = uint32_t;
#endif  // MARISA_WORD_SIZE == 64

  FlatVector() = default;

  FlatVector(const FlatVector &) = delete;
  FlatVector &operator=(const FlatVector &) = delete;

  void build(const Vector<uint32_t> &values) {
    FlatVector temp;
    temp.build_(values);
    swap(temp);
  }

  void map(Mapper &mapper) {
    FlatVector temp;
    temp.map_(mapper);
    swap(temp);
  }
  void read(Reader &reader) {
    FlatVector temp;
    temp.read_(reader);
    swap(temp);
  }
  void write(Writer &writer) const {
    write_(writer);
  }

  uint32_t operator[](std::size_t i) const {
    assert(i < size_);

    const std::size_t pos = i * value_size_;
    const std::size_t unit_id = pos / MARISA_WORD_SIZE;
    const std::size_t unit_offset = pos % MARISA_WORD_SIZE;

    if ((unit_offset + value_size_) <= MARISA_WORD_SIZE) {
      return static_cast<uint32_t>(units_[unit_id] >> unit_offset) & mask_;
    } else {
      return static_cast<uint32_t>(
                 (units_[unit_id] >> unit_offset) |
                 (units_[unit_id + 1] << (MARISA_WORD_SIZE - unit_offset))) &
             mask_;
    }
  }

  std::size_t value_size() const {
    return value_size_;
  }
  uint32_t mask() const {
    return mask_;
  }

  bool empty() const {
    return size_ == 0;
  }
  std::size_t size() const {
    return size_;
  }
  std::size_t total_size() const {
    return units_.total_size();
  }
  std::size_t io_size() const {
    return units_.io_size() + (sizeof(uint32_t) * 2) + sizeof(uint64_t);
  }

  void clear() noexcept {
    FlatVector().swap(*this);
  }
  void swap(FlatVector &rhs) noexcept {
    units_.swap(rhs.units_);
    std::swap(value_size_, rhs.value_size_);
    std::swap(mask_, rhs.mask_);
    std::swap(size_, rhs.size_);
  }

 private:
  Vector<Unit> units_;
  std::size_t value_size_ = 0;
  uint32_t mask_ = 0;
  std::size_t size_ = 0;

  void build_(const Vector<uint32_t> &values) {
    uint32_t max_value = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
      if (values[i] > max_value) {
        max_value = values[i];
      }
    }

    std::size_t value_size = 0;
    while (max_value != 0) {
      ++value_size;
      max_value >>= 1;
    }

    std::size_t num_units = values.empty() ? 0 : (64 / MARISA_WORD_SIZE);
    if (value_size != 0) {
      num_units = static_cast<std::size_t>(
          ((static_cast<uint64_t>(value_size) * values.size()) +
           (MARISA_WORD_SIZE - 1)) /
          MARISA_WORD_SIZE);
      num_units += num_units % (64 / MARISA_WORD_SIZE);
    }

    units_.resize(num_units);
    if (num_units > 0) {
      units_.back() = 0;
    }

    value_size_ = value_size;
    if (value_size != 0) {
      mask_ = UINT32_MAX >> (32 - value_size);
    }
    size_ = values.size();

    for (std::size_t i = 0; i < values.size(); ++i) {
      set(i, values[i]);
    }
  }

  void map_(Mapper &mapper) {
    units_.map(mapper);
    {
      uint32_t temp_value_size;
      mapper.map(&temp_value_size);
      MARISA_THROW_IF(temp_value_size > 32, std::runtime_error);
      value_size_ = temp_value_size;
    }
    {
      uint32_t temp_mask;
      mapper.map(&temp_mask);
      mask_ = temp_mask;
    }
    {
      uint64_t temp_size;
      mapper.map(&temp_size);
      MARISA_THROW_IF(temp_size > SIZE_MAX, std::runtime_error);
      size_ = static_cast<std::size_t>(temp_size);
    }
  }

  void read_(Reader &reader) {
    units_.read(reader);
    {
      uint32_t temp_value_size;
      reader.read(&temp_value_size);
      MARISA_THROW_IF(temp_value_size > 32, std::runtime_error);
      value_size_ = temp_value_size;
    }
    {
      uint32_t temp_mask;
      reader.read(&temp_mask);
      mask_ = temp_mask;
    }
    {
      uint64_t temp_size;
      reader.read(&temp_size);
      MARISA_THROW_IF(temp_size > SIZE_MAX, std::runtime_error);
      size_ = static_cast<std::size_t>(temp_size);
    }
  }

  void write_(Writer &writer) const {
    units_.write(writer);
    writer.write(static_cast<uint32_t>(value_size_));
    writer.write(static_cast<uint32_t>(mask_));
    writer.write(static_cast<uint64_t>(size_));
  }

  void set(std::size_t i, uint32_t value) {
    assert(i < size_);
    assert(value <= mask_);

    const std::size_t pos = i * value_size_;
    const std::size_t unit_id = pos / MARISA_WORD_SIZE;
    const std::size_t unit_offset = pos % MARISA_WORD_SIZE;

    units_[unit_id] &= ~(static_cast<Unit>(mask_) << unit_offset);
    units_[unit_id] |= static_cast<Unit>(value & mask_) << unit_offset;
    if ((unit_offset + value_size_) > MARISA_WORD_SIZE) {
      units_[unit_id + 1] &=
          ~(static_cast<Unit>(mask_) >> (MARISA_WORD_SIZE - unit_offset));
      units_[unit_id + 1] |=
          static_cast<Unit>(value & mask_) >> (MARISA_WORD_SIZE - unit_offset);
    }
  }
};

}  // namespace marisa::grimoire::vector

#endif  // MARISA_GRIMOIRE_VECTOR_FLAT_VECTOR_H_
