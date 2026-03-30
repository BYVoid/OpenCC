#ifndef MARISA_GRIMOIRE_VECTOR_VECTOR_H_
#define MARISA_GRIMOIRE_VECTOR_VECTOR_H_

#include <cassert>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "marisa/grimoire/io.h"

namespace marisa::grimoire::vector {

template <typename T>
class Vector {
 public:
  // These assertions are repeated for clarity/robustness where the property
  // is used.
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(std::is_trivially_destructible_v<T>);

  Vector() = default;
  // `T` is trivially destructible, so default destructor is ok.
  ~Vector() = default;

  Vector(const Vector<T> &other) : fixed_(other.fixed_) {
    if (other.buf_ == nullptr) {
      objs_ = other.objs_;
      const_objs_ = other.const_objs_;
      size_ = other.size_;
      capacity_ = other.capacity_;
    } else {
      copyInit(other.const_objs_, other.size_, other.capacity_);
    }
  }

  Vector &operator=(const Vector<T> &other) {
    clear();
    fixed_ = other.fixed_;
    if (other.buf_ == nullptr) {
      objs_ = other.objs_;
      const_objs_ = other.const_objs_;
      size_ = other.size_;
      capacity_ = other.capacity_;
    } else {
      copyInit(other.const_objs_, other.size_, other.capacity_);
    }
    return *this;
  }

  Vector(Vector &&) noexcept = default;
  Vector &operator=(Vector<T> &&) noexcept = default;

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
    assert(!fixed_);
    assert(size_ < max_size());
    reserve(size_ + 1);
    new (&objs_[size_]) T(x);
    ++size_;
  }

  void pop_back() {
    assert(!fixed_);
    assert(size_ != 0);
    --size_;
    static_assert(std::is_trivially_destructible_v<T>);
  }

  // resize() assumes that T's placement new does not throw an exception.
  void resize(std::size_t size) {
    assert(!fixed_);
    reserve(size);
    for (std::size_t i = size_; i < size; ++i) {
      new (&objs_[i]) T;
    }
    static_assert(std::is_trivially_destructible_v<T>);
    size_ = size;
  }

  // resize() assumes that T's placement new does not throw an exception.
  void resize(std::size_t size, const T &x) {
    assert(!fixed_);
    reserve(size);
    if (size > size_) {
      std::fill_n(&objs_[size_], size - size_, x);
    }
    // No need to destroy old elements.
    static_assert(std::is_trivially_destructible_v<T>);
    size_ = size;
  }

  void reserve(std::size_t capacity) {
    assert(!fixed_);
    if (capacity <= capacity_) {
      return;
    }
    assert(capacity <= max_size());
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
    MARISA_THROW_IF(fixed_, std::logic_error);
    if (size_ != capacity_) {
      realloc(size_);
    }
  }

  void fix() {
    MARISA_THROW_IF(fixed_, std::logic_error);
    fixed_ = true;
  }

  const T *begin() const {
    return const_objs_;
  }
  const T *end() const {
    return const_objs_ + size_;
  }
  const T &operator[](std::size_t i) const {
    assert(i < size_);
    return const_objs_[i];
  }
  const T &front() const {
    assert(size_ != 0);
    return const_objs_[0];
  }
  const T &back() const {
    assert(size_ != 0);
    return const_objs_[size_ - 1];
  }

  T *begin() {
    assert(!fixed_);
    return objs_;
  }
  T *end() {
    assert(!fixed_);
    return objs_ + size_;
  }
  T &operator[](std::size_t i) {
    assert(!fixed_);
    assert(i < size_);
    return objs_[i];
  }
  T &front() {
    assert(!fixed_);
    assert(size_ != 0);
    return objs_[0];
  }
  T &back() {
    assert(!fixed_);
    assert(size_ != 0);
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
    return sizeof(uint64_t) + ((total_size() + 7) & ~0x07U);
  }

  void clear() noexcept {
    Vector().swap(*this);
  }
  void swap(Vector &rhs) noexcept {
    buf_.swap(rhs.buf_);
    std::swap(objs_, rhs.objs_);
    std::swap(const_objs_, rhs.const_objs_);
    std::swap(size_, rhs.size_);
    std::swap(capacity_, rhs.capacity_);
    std::swap(fixed_, rhs.fixed_);
  }

  static std::size_t max_size() {
    return SIZE_MAX / sizeof(T);
  }

 private:
  std::unique_ptr<char[]> buf_;
  T *objs_ = nullptr;
  const T *const_objs_ = nullptr;
  std::size_t size_ = 0;
  std::size_t capacity_ = 0;
  bool fixed_ = false;

  void map_(Mapper &mapper) {
    uint64_t total_size;
    mapper.map(&total_size);
    MARISA_THROW_IF(total_size > SIZE_MAX, std::runtime_error);
    MARISA_THROW_IF((total_size % sizeof(T)) != 0, std::runtime_error);
    const std::size_t size = static_cast<std::size_t>(total_size / sizeof(T));
    mapper.map(&const_objs_, size);
    mapper.seek(static_cast<std::size_t>((8 - (total_size % 8)) % 8));
    size_ = size;
    fix();
  }
  void read_(Reader &reader) {
    uint64_t total_size;
    reader.read(&total_size);
    MARISA_THROW_IF(total_size > SIZE_MAX, std::runtime_error);
    MARISA_THROW_IF((total_size % sizeof(T)) != 0, std::runtime_error);
    const std::size_t size = static_cast<std::size_t>(total_size / sizeof(T));
    resize(size);
    reader.read(objs_, size);
    reader.seek(static_cast<std::size_t>((8 - (total_size % 8)) % 8));
  }
  void write_(Writer &writer) const {
    writer.write(static_cast<uint64_t>(total_size()));
    writer.write(const_objs_, size_);
    writer.seek((8 - (total_size() % 8)) % 8);
  }

  // Copies current elements to new buffer of size `new_capacity`.
  // Requires `new_capacity >= size_`.
  void realloc(std::size_t new_capacity) {
    assert(new_capacity >= size_);
    assert(new_capacity <= max_size());

    std::unique_ptr<char[]> new_buf(new char[sizeof(T) * new_capacity]);
    T *new_objs = reinterpret_cast<T *>(new_buf.get());

    static_assert(std::is_trivially_copyable_v<T>);
    std::memcpy(new_objs, objs_, sizeof(T) * size_);

    buf_ = std::move(new_buf);
    objs_ = new_objs;
    const_objs_ = new_objs;
    capacity_ = new_capacity;
  }

  // copyInit() assumes that T's placement new does not throw an exception.
  // Requires the vector to be empty.
  void copyInit(const T *src, std::size_t size, std::size_t capacity) {
    assert(size_ == 0);

    std::unique_ptr<char[]> new_buf(new char[sizeof(T) * capacity]);
    T *new_objs = reinterpret_cast<T *>(new_buf.get());

    static_assert(std::is_trivially_copyable_v<T>);
    std::memcpy(new_objs, src, sizeof(T) * size);

    buf_ = std::move(new_buf);
    objs_ = new_objs;
    const_objs_ = new_objs;
    size_ = size;
    capacity_ = capacity;
  }
};

}  // namespace marisa::grimoire::vector

#endif  // MARISA_GRIMOIRE_VECTOR_VECTOR_H_
