#ifndef MARISA_GRIMOIRE_VECTOR_FLAT_VECTOR_H_
#define MARISA_GRIMOIRE_VECTOR_FLAT_VECTOR_H_

#include "marisa/grimoire/vector/vector.h"

namespace marisa {
namespace grimoire {
namespace vector {

class FlatVector {
 public:
#if MARISA_WORD_SIZE == 64
  typedef UInt64 Unit;
#else  // MARISA_WORD_SIZE == 64
  typedef UInt32 Unit;
#endif  // MARISA_WORD_SIZE == 64

  FlatVector() : units_(), value_size_(0), mask_(0), size_(0) {}

  void build(const Vector<UInt32> &values) {
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

  UInt32 operator[](std::size_t i) const {
    MARISA_DEBUG_IF(i >= size_, MARISA_BOUND_ERROR);

    const std::size_t pos = i * value_size_;
    const std::size_t unit_id = pos / MARISA_WORD_SIZE;
    const std::size_t unit_offset = pos % MARISA_WORD_SIZE;

    if ((unit_offset + value_size_) <= MARISA_WORD_SIZE) {
      return (UInt32)(units_[unit_id] >> unit_offset) & mask_;
    } else {
      return (UInt32)((units_[unit_id] >> unit_offset)
          | (units_[unit_id + 1] << (MARISA_WORD_SIZE - unit_offset))) & mask_;
    }
  }

  std::size_t value_size() const {
    return value_size_;
  }
  UInt32 mask() const {
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
    return units_.io_size() + (sizeof(UInt32) * 2) + sizeof(UInt64);
  }

  void clear() {
    FlatVector().swap(*this);
  }
  void swap(FlatVector &rhs) {
    units_.swap(rhs.units_);
    marisa::swap(value_size_, rhs.value_size_);
    marisa::swap(mask_, rhs.mask_);
    marisa::swap(size_, rhs.size_);
  }

 private:
  Vector<Unit> units_;
  std::size_t value_size_;
  UInt32 mask_;
  std::size_t size_;

  void build_(const Vector<UInt32> &values) {
    UInt32 max_value = 0;
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
      num_units = (std::size_t)(
          (((UInt64)value_size * values.size()) + (MARISA_WORD_SIZE - 1))
          / MARISA_WORD_SIZE);
      num_units += num_units % (64 / MARISA_WORD_SIZE);
    }

    units_.resize(num_units);
    if (num_units > 0) {
      units_.back() = 0;
    }

    value_size_ = value_size;
    if (value_size != 0) {
      mask_ = MARISA_UINT32_MAX >> (32 - value_size);
    }
    size_ = values.size();

    for (std::size_t i = 0; i < values.size(); ++i) {
      set(i, values[i]);
    }
  }

  void map_(Mapper &mapper) {
    units_.map(mapper);
    {
      UInt32 temp_value_size;
      mapper.map(&temp_value_size);
      MARISA_THROW_IF(temp_value_size > 32, MARISA_FORMAT_ERROR);
      value_size_ = temp_value_size;
    }
    {
      UInt32 temp_mask;
      mapper.map(&temp_mask);
      mask_ = temp_mask;
    }
    {
      UInt64 temp_size;
      mapper.map(&temp_size);
      MARISA_THROW_IF(temp_size > MARISA_SIZE_MAX, MARISA_SIZE_ERROR);
      size_ = (std::size_t)temp_size;
    }
  }

  void read_(Reader &reader) {
    units_.read(reader);
    {
      UInt32 temp_value_size;
      reader.read(&temp_value_size);
      MARISA_THROW_IF(temp_value_size > 32, MARISA_FORMAT_ERROR);
      value_size_ = temp_value_size;
    }
    {
      UInt32 temp_mask;
      reader.read(&temp_mask);
      mask_ = temp_mask;
    }
    {
      UInt64 temp_size;
      reader.read(&temp_size);
      MARISA_THROW_IF(temp_size > MARISA_SIZE_MAX, MARISA_SIZE_ERROR);
      size_ = (std::size_t)temp_size;
    }
  }

  void write_(Writer &writer) const {
    units_.write(writer);
    writer.write((UInt32)value_size_);
    writer.write((UInt32)mask_);
    writer.write((UInt64)size_);
  }

  void set(std::size_t i, UInt32 value) {
    MARISA_DEBUG_IF(i >= size_, MARISA_BOUND_ERROR);
    MARISA_DEBUG_IF(value > mask_, MARISA_RANGE_ERROR);

    const std::size_t pos = i * value_size_;
    const std::size_t unit_id = pos / MARISA_WORD_SIZE;
    const std::size_t unit_offset = pos % MARISA_WORD_SIZE;

    units_[unit_id] &= ~((Unit)mask_ << unit_offset);
    units_[unit_id] |= (Unit)(value & mask_) << unit_offset;
    if ((unit_offset + value_size_) > MARISA_WORD_SIZE) {
      units_[unit_id + 1] &=
          ~((Unit)mask_ >> (MARISA_WORD_SIZE - unit_offset));
      units_[unit_id + 1] |=
          (Unit)(value & mask_) >> (MARISA_WORD_SIZE - unit_offset);
    }
  }

  // Disallows copy and assignment.
  FlatVector(const FlatVector &);
  FlatVector &operator=(const FlatVector &);
};

}  // namespace vector
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_VECTOR_FLAT_VECTOR_H_
