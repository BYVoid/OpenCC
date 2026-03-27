#include "marisa/keyset.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>

namespace marisa {

Keyset::Keyset() = default;

void Keyset::push_back(const Key &key) {
  assert(size_ < SIZE_MAX);

  char *const key_ptr = reserve(key.length());
  std::memcpy(key_ptr, key.ptr(), key.length());

  Key &new_key = key_blocks_[size_ / KEY_BLOCK_SIZE][size_ % KEY_BLOCK_SIZE];
  new_key.set_str(key_ptr, key.length());
  new_key.set_id(key.id());
  ++size_;
  total_length_ += new_key.length();
}

void Keyset::push_back(const Key &key, char end_marker) {
  assert(size_ < SIZE_MAX);

  if ((size_ / KEY_BLOCK_SIZE) == key_blocks_size_) {
    append_key_block();
  }

  char *const key_ptr = reserve(key.length() + 1);
  std::memcpy(key_ptr, key.ptr(), key.length());
  key_ptr[key.length()] = end_marker;

  Key &new_key = key_blocks_[size_ / KEY_BLOCK_SIZE][size_ % KEY_BLOCK_SIZE];
  new_key.set_str(key_ptr, key.length());
  new_key.set_id(key.id());
  ++size_;
  total_length_ += new_key.length();
}

void Keyset::push_back(const char *str) {
  assert(size_ < SIZE_MAX);
  MARISA_THROW_IF(str == nullptr, std::invalid_argument);

  std::size_t length = 0;
  while (str[length] != '\0') {
    ++length;
  }
  push_back(str, length);
}

void Keyset::push_back(const char *ptr, std::size_t length, float weight) {
  assert(size_ < SIZE_MAX);
  MARISA_THROW_IF((ptr == nullptr) && (length != 0), std::invalid_argument);
  MARISA_THROW_IF(length > UINT32_MAX, std::invalid_argument);

  char *const key_ptr = reserve(length);
  std::memcpy(key_ptr, ptr, length);

  Key &key = key_blocks_[size_ / KEY_BLOCK_SIZE][size_ % KEY_BLOCK_SIZE];
  key.set_str(key_ptr, length);
  key.set_weight(weight);
  ++size_;
  total_length_ += length;
}

void Keyset::reset() {
  base_blocks_size_ = 0;
  extra_blocks_size_ = 0;
  ptr_ = nullptr;
  avail_ = 0;
  size_ = 0;
  total_length_ = 0;
}

void Keyset::clear() noexcept {
  Keyset().swap(*this);
}

void Keyset::swap(Keyset &rhs) noexcept {
  base_blocks_.swap(rhs.base_blocks_);
  std::swap(base_blocks_size_, rhs.base_blocks_size_);
  std::swap(base_blocks_capacity_, rhs.base_blocks_capacity_);
  extra_blocks_.swap(rhs.extra_blocks_);
  std::swap(extra_blocks_size_, rhs.extra_blocks_size_);
  std::swap(extra_blocks_capacity_, rhs.extra_blocks_capacity_);
  key_blocks_.swap(rhs.key_blocks_);
  std::swap(key_blocks_size_, rhs.key_blocks_size_);
  std::swap(key_blocks_capacity_, rhs.key_blocks_capacity_);
  std::swap(ptr_, rhs.ptr_);
  std::swap(avail_, rhs.avail_);
  std::swap(size_, rhs.size_);
  std::swap(total_length_, rhs.total_length_);
}

char *Keyset::reserve(std::size_t size) {
  if ((size_ / KEY_BLOCK_SIZE) == key_blocks_size_) {
    append_key_block();
  }

  if (size > EXTRA_BLOCK_SIZE) {
    append_extra_block(size);
    return extra_blocks_[extra_blocks_size_ - 1].get();
  }
  if (size > avail_) {
    append_base_block();
  }
  ptr_ += size;
  avail_ -= size;
  return ptr_ - size;
}

void Keyset::append_base_block() {
  if (base_blocks_size_ == base_blocks_capacity_) {
    const std::size_t new_capacity =
        (base_blocks_size_ != 0) ? (base_blocks_size_ * 2) : 1;
    std::unique_ptr<std::unique_ptr<char[]>[]> new_blocks(
        new std::unique_ptr<char[]>[new_capacity]);
    for (std::size_t i = 0; i < base_blocks_size_; ++i) {
      base_blocks_[i].swap(new_blocks[i]);
    }
    base_blocks_.swap(new_blocks);
    base_blocks_capacity_ = new_capacity;
  }
  if (base_blocks_[base_blocks_size_] == nullptr) {
    std::unique_ptr<char[]> new_block(new char[BASE_BLOCK_SIZE]);
    base_blocks_[base_blocks_size_].swap(new_block);
  }
  ptr_ = base_blocks_[base_blocks_size_++].get();
  avail_ = BASE_BLOCK_SIZE;
}

void Keyset::append_extra_block(std::size_t size) {
  if (extra_blocks_size_ == extra_blocks_capacity_) {
    const std::size_t new_capacity =
        (extra_blocks_size_ != 0) ? (extra_blocks_size_ * 2) : 1;
    std::unique_ptr<std::unique_ptr<char[]>[]> new_blocks(
        new std::unique_ptr<char[]>[new_capacity]);
    for (std::size_t i = 0; i < extra_blocks_size_; ++i) {
      extra_blocks_[i].swap(new_blocks[i]);
    }
    extra_blocks_.swap(new_blocks);
    extra_blocks_capacity_ = new_capacity;
  }
  std::unique_ptr<char[]> new_block(new char[size]);
  extra_blocks_[extra_blocks_size_++].swap(new_block);
}

void Keyset::append_key_block() {
  if (key_blocks_size_ == key_blocks_capacity_) {
    const std::size_t new_capacity =
        (key_blocks_size_ != 0) ? (key_blocks_size_ * 2) : 1;
    std::unique_ptr<std::unique_ptr<Key[]>[]> new_blocks(
        new std::unique_ptr<Key[]>[new_capacity]);
    for (std::size_t i = 0; i < key_blocks_size_; ++i) {
      key_blocks_[i].swap(new_blocks[i]);
    }
    key_blocks_.swap(new_blocks);
    key_blocks_capacity_ = new_capacity;
  }
  std::unique_ptr<Key[]> new_block(new Key[KEY_BLOCK_SIZE]);
  key_blocks_[key_blocks_size_++].swap(new_block);
}

}  // namespace marisa
