#ifndef MARISA_GRIMOIRE_VECTOR_BIT_VECTOR_H_
#define MARISA_GRIMOIRE_VECTOR_BIT_VECTOR_H_

#include <cassert>
#include <stdexcept>

#include "marisa/grimoire/vector/rank-index.h"
#include "marisa/grimoire/vector/vector.h"

namespace marisa::grimoire::vector {

class BitVector {
 public:
#if MARISA_WORD_SIZE == 64
  using Unit = uint64_t;
#else   // MARISA_WORD_SIZE == 64
  using Unit = uint32_t;
#endif  // MARISA_WORD_SIZE == 64

  BitVector() = default;

  BitVector(const BitVector &) = delete;
  BitVector &operator=(const BitVector &) = delete;

  void build(bool enables_select0, bool enables_select1) {
    BitVector temp;
    temp.build_index(*this, enables_select0, enables_select1);
    units_.shrink();
    temp.units_.swap(units_);
    swap(temp);
  }

  void map(Mapper &mapper) {
    BitVector temp;
    temp.map_(mapper);
    swap(temp);
  }
  void read(Reader &reader) {
    BitVector temp;
    temp.read_(reader);
    swap(temp);
  }
  void write(Writer &writer) const {
    write_(writer);
  }

  void disable_select0() {
    select0s_.clear();
  }
  void disable_select1() {
    select1s_.clear();
  }

  void push_back(bool bit) {
    MARISA_THROW_IF(size_ == UINT32_MAX, std::length_error);
    if (size_ == (MARISA_WORD_SIZE * units_.size())) {
      units_.resize(units_.size() + (64 / MARISA_WORD_SIZE), 0);
    }
    if (bit) {
      units_[size_ / MARISA_WORD_SIZE] |= Unit{1} << (size_ % MARISA_WORD_SIZE);
      ++num_1s_;
    }
    ++size_;
  }

  bool operator[](std::size_t i) const {
    assert(i < size_);
    return (units_[i / MARISA_WORD_SIZE] &
            (Unit{1} << (i % MARISA_WORD_SIZE))) != 0;
  }

  std::size_t rank0(std::size_t i) const {
    assert(!ranks_.empty());
    assert(i <= size_);
    return i - rank1(i);
  }
  std::size_t rank1(std::size_t i) const;

  std::size_t select0(std::size_t i) const;
  std::size_t select1(std::size_t i) const;

  std::size_t num_0s() const {
    return size_ - num_1s_;
  }
  std::size_t num_1s() const {
    return num_1s_;
  }

  bool empty() const {
    return size_ == 0;
  }
  std::size_t size() const {
    return size_;
  }
  std::size_t total_size() const {
    return units_.total_size() + ranks_.total_size() + select0s_.total_size() +
           select1s_.total_size();
  }
  std::size_t io_size() const {
    return units_.io_size() + (sizeof(uint32_t) * 2) + ranks_.io_size() +
           select0s_.io_size() + select1s_.io_size();
  }

  void clear() noexcept {
    BitVector().swap(*this);
  }
  void swap(BitVector &rhs) noexcept {
    units_.swap(rhs.units_);
    std::swap(size_, rhs.size_);
    std::swap(num_1s_, rhs.num_1s_);
    ranks_.swap(rhs.ranks_);
    select0s_.swap(rhs.select0s_);
    select1s_.swap(rhs.select1s_);
  }

 private:
  Vector<Unit> units_;
  std::size_t size_ = 0;
  std::size_t num_1s_ = 0;
  Vector<RankIndex> ranks_;
  Vector<uint32_t> select0s_;
  Vector<uint32_t> select1s_;

  void build_index(const BitVector &bv, bool enables_select0,
                   bool enables_select1);

  void map_(Mapper &mapper) {
    units_.map(mapper);
    {
      uint32_t temp_size;
      mapper.map(&temp_size);
      size_ = temp_size;
    }
    {
      uint32_t temp_num_1s;
      mapper.map(&temp_num_1s);
      MARISA_THROW_IF(temp_num_1s > size_, std::runtime_error);
      num_1s_ = temp_num_1s;
    }
    ranks_.map(mapper);
    select0s_.map(mapper);
    select1s_.map(mapper);
  }

  void read_(Reader &reader) {
    units_.read(reader);
    {
      uint32_t temp_size;
      reader.read(&temp_size);
      size_ = temp_size;
    }
    {
      uint32_t temp_num_1s;
      reader.read(&temp_num_1s);
      MARISA_THROW_IF(temp_num_1s > size_, std::runtime_error);
      num_1s_ = temp_num_1s;
    }
    ranks_.read(reader);
    select0s_.read(reader);
    select1s_.read(reader);
  }

  void write_(Writer &writer) const {
    units_.write(writer);
    writer.write(static_cast<uint32_t>(size_));
    writer.write(static_cast<uint32_t>(num_1s_));
    ranks_.write(writer);
    select0s_.write(writer);
    select1s_.write(writer);
  }
};

}  // namespace marisa::grimoire::vector

#endif  // MARISA_GRIMOIRE_VECTOR_BIT_VECTOR_H_
