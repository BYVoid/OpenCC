#ifndef MARISA_GRIMOIRE_VECTOR_VECTOR_H_
#define MARISA_GRIMOIRE_VECTOR_VECTOR_H_

#include <new>

#include "marisa/grimoire/io.h"

namespace marisa {
namespace grimoire {
namespace vector {

template <typename T>
class Vector {
 public:
  Vector()
      : buf_(), objs_(NULL), const_objs_(NULL),
        size_(0), capacity_(0), fixed_(false) {}
  ~Vector() {
    if (objs_ != NULL) {
      for (std::size_t i = 0; i < size_; ++i) {
        objs_[i].~T();
      }
    }
  }

  void map(Mapper &mapper) {
    Vector temp;
    temp.map_(mapper);
    swap(temp);
  }

  void read(Reader &reader) {
    Vector temp;
    temp.read_(reader);
    swap(temp);
  }

  void write(Writer &writer) const {
    write_(writer);
  }

  void push_back(const T &x) {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    MARISA_DEBUG_IF(size_ == max_size(), MARISA_SIZE_ERROR);
    reserve(size_ + 1);
    new (&objs_[size_]) T(x);
    ++size_;
  }

  void pop_back() {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    MARISA_DEBUG_IF(size_ == 0, MARISA_STATE_ERROR);
    objs_[--size_].~T();
  }

  // resize() assumes that T's placement new does not throw an exception.
  void resize(std::size_t size) {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    reserve(size);
    for (std::size_t i = size_; i < size; ++i) {
      new (&objs_[i]) T;
    }
    for (std::size_t i = size; i < size_; ++i) {
      objs_[i].~T();
    }
    size_ = size;
  }

  // resize() assumes that T's placement new does not throw an exception.
  void resize(std::size_t size, const T &x) {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    reserve(size);
    for (std::size_t i = size_; i < size; ++i) {
      new (&objs_[i]) T(x);
    }
    for (std::size_t i = size; i < size_; ++i) {
      objs_[i].~T();
    }
    size_ = size;
  }

  void reserve(std::size_t capacity) {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    if (capacity <= capacity_) {
      return;
    }
    MARISA_DEBUG_IF(capacity > max_size(), MARISA_SIZE_ERROR);
    std::size_t new_capacity = capacity;
    if (capacity_ > (capacity / 2)) {
      if (capacity_ > (max_size() / 2)) {
        new_capacity = max_size();
      } else {
        new_capacity = capacity_ * 2;
      }
    }
    realloc(new_capacity);
  }

  void shrink() {
    MARISA_THROW_IF(fixed_, MARISA_STATE_ERROR);
    if (size_ != capacity_) {
      realloc(size_);
    }
  }

  void fix() {
    MARISA_THROW_IF(fixed_, MARISA_STATE_ERROR);
    fixed_ = true;
  }

  const T *begin() const {
    return const_objs_;
  }
  const T *end() const {
    return const_objs_ + size_;
  }
  const T &operator[](std::size_t i) const {
    MARISA_DEBUG_IF(i >= size_, MARISA_BOUND_ERROR);
    return const_objs_[i];
  }
  const T &front() const {
    MARISA_DEBUG_IF(size_ == 0, MARISA_STATE_ERROR);
    return const_objs_[0];
  }
  const T &back() const {
    MARISA_DEBUG_IF(size_ == 0, MARISA_STATE_ERROR);
    return const_objs_[size_ - 1];
  }

  T *begin() {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    return objs_;
  }
  T *end() {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    return objs_ + size_;
  }
  T &operator[](std::size_t i) {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    MARISA_DEBUG_IF(i >= size_, MARISA_BOUND_ERROR);
    return objs_[i];
  }
  T &front() {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    MARISA_DEBUG_IF(size_ == 0, MARISA_STATE_ERROR);
    return objs_[0];
  }
  T &back() {
    MARISA_DEBUG_IF(fixed_, MARISA_STATE_ERROR);
    MARISA_DEBUG_IF(size_ == 0, MARISA_STATE_ERROR);
    return objs_[size_ - 1];
  }

  std::size_t size() const {
    return size_;
  }
  std::size_t capacity() const {
    return capacity_;
  }
  bool fixed() const {
    return fixed_;
  }

  bool empty() const {
    return size_ == 0;
  }
  std::size_t total_size() const {
    return sizeof(T) * size_;
  }
  std::size_t io_size() const {
    return sizeof(UInt64) + ((total_size() + 7) & ~(std::size_t)0x07);
  }

  void clear() {
    Vector().swap(*this);
  }
  void swap(Vector &rhs) {
    buf_.swap(rhs.buf_);
    marisa::swap(objs_, rhs.objs_);
    marisa::swap(const_objs_, rhs.const_objs_);
    marisa::swap(size_, rhs.size_);
    marisa::swap(capacity_, rhs.capacity_);
    marisa::swap(fixed_, rhs.fixed_);
  }

  static std::size_t max_size() {
    return MARISA_SIZE_MAX / sizeof(T);
  }

 private:
  scoped_array<char> buf_;
  T *objs_;
  const T *const_objs_;
  std::size_t size_;
  std::size_t capacity_;
  bool fixed_;

  void map_(Mapper &mapper) {
    UInt64 total_size;
    mapper.map(&total_size);
    MARISA_THROW_IF(total_size > MARISA_SIZE_MAX, MARISA_SIZE_ERROR);
    MARISA_THROW_IF((total_size % sizeof(T)) != 0, MARISA_FORMAT_ERROR);
    const std::size_t size = (std::size_t)(total_size / sizeof(T));
    mapper.map(&const_objs_, size);
    mapper.seek((std::size_t)((8 - (total_size % 8)) % 8));
    size_ = size;
    fix();
  }
  void read_(Reader &reader) {
    UInt64 total_size;
    reader.read(&total_size);
    MARISA_THROW_IF(total_size > MARISA_SIZE_MAX, MARISA_SIZE_ERROR);
    MARISA_THROW_IF((total_size % sizeof(T)) != 0, MARISA_FORMAT_ERROR);
    const std::size_t size = (std::size_t)(total_size / sizeof(T));
    resize(size);
    reader.read(objs_, size);
    reader.seek((std::size_t)((8 - (total_size % 8)) % 8));
  }
  void write_(Writer &writer) const {
    writer.write((UInt64)total_size());
    writer.write(const_objs_, size_);
    writer.seek((8 - (total_size() % 8)) % 8);
  }

  // realloc() assumes that T's placement new does not throw an exception.
  void realloc(std::size_t new_capacity) {
    MARISA_DEBUG_IF(new_capacity > max_size(), MARISA_SIZE_ERROR);

    scoped_array<char> new_buf(
        new (std::nothrow) char[sizeof(T) * new_capacity]);
    MARISA_DEBUG_IF(new_buf.get() == NULL, MARISA_MEMORY_ERROR);
    T *new_objs = reinterpret_cast<T *>(new_buf.get());

    for (std::size_t i = 0; i < size_; ++i) {
      new (&new_objs[i]) T(objs_[i]);
    }
    for (std::size_t i = 0; i < size_; ++i) {
      objs_[i].~T();
    }

    buf_.swap(new_buf);
    objs_ = new_objs;
    const_objs_ = new_objs;
    capacity_ = new_capacity;
  }

  // Disallows copy and assignment.
  Vector(const Vector &);
  Vector &operator=(const Vector &);
};

}  // namespace vector
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_VECTOR_VECTOR_H_
