#include <new>

#include "marisa/keyset.h"

namespace marisa {

Keyset::Keyset()
    : base_blocks_(), base_blocks_size_(0), base_blocks_capacity_(0),
      extra_blocks_(), extra_blocks_size_(0), extra_blocks_capacity_(0),
      key_blocks_(), key_blocks_size_(0), key_blocks_capacity_(0),
      ptr_(NULL), avail_(0), size_(0), total_length_(0) {}

void Keyset::push_back(const Key &key) {
  MARISA_DEBUG_IF(size_ == MARISA_SIZE_MAX, MARISA_SIZE_ERROR);

  char * const key_ptr = reserve(key.length());
  for (std::size_t i = 0; i < key.length(); ++i) {
    key_ptr[i] = key[i];
  }

  Key &new_key = key_blocks_[size_ / KEY_BLOCK_SIZE][size_ % KEY_BLOCK_SIZE];
  new_key.set_str(key_ptr, key.length());
  new_key.set_id(key.id());
  ++size_;
  total_length_ += new_key.length();
}

void Keyset::push_back(const Key &key, char end_marker) {
  MARISA_DEBUG_IF(size_ == MARISA_SIZE_MAX, MARISA_SIZE_ERROR);

  if ((size_ / KEY_BLOCK_SIZE) == key_blocks_size_) {
    append_key_block();
  }

  char * const key_ptr = reserve(key.length() + 1);
  for (std::size_t i = 0; i < key.length(); ++i) {
    key_ptr[i] = key[i];
  }
  key_ptr[key.length()] = end_marker;

  Key &new_key = key_blocks_[size_ / KEY_BLOCK_SIZE][size_ % KEY_BLOCK_SIZE];
  new_key.set_str(key_ptr, key.length());
  new_key.set_id(key.id());
  ++size_;
  total_length_ += new_key.length();
}

void Keyset::push_back(const char *str) {
  MARISA_DEBUG_IF(size_ == MARISA_SIZE_MAX, MARISA_SIZE_ERROR);
  MARISA_THROW_IF(str == NULL, MARISA_NULL_ERROR);

  std::size_t length = 0;
  while (str[length] != '\0') {
    ++length;
  }
  push_back(str, length);
}

void Keyset::push_back(const char *ptr, std::size_t length, float weight) {
  MARISA_DEBUG_IF(size_ == MARISA_SIZE_MAX, MARISA_SIZE_ERROR);
  MARISA_THROW_IF((ptr == NULL) && (length != 0), MARISA_NULL_ERROR);
  MARISA_THROW_IF(length > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);

  char * const key_ptr = reserve(length);
  for (std::size_t i = 0; i < length; ++i) {
    key_ptr[i] = ptr[i];
  }

  Key &key = key_blocks_[size_ / KEY_BLOCK_SIZE][size_ % KEY_BLOCK_SIZE];
  key.set_str(key_ptr, length);
  key.set_weight(weight);
  ++size_;
  total_length_ += length;
}

void Keyset::reset() {
  base_blocks_size_ = 0;
  extra_blocks_size_ = 0;
  ptr_ = NULL;
  avail_ = 0;
  size_ = 0;
  total_length_ = 0;
}

void Keyset::clear() {
  Keyset().swap(*this);
}

void Keyset::swap(Keyset &rhs) {
  base_blocks_.swap(rhs.base_blocks_);
  marisa::swap(base_blocks_size_, rhs.base_blocks_size_);
  marisa::swap(base_blocks_capacity_, rhs.base_blocks_capacity_);
  extra_blocks_.swap(rhs.extra_blocks_);
  marisa::swap(extra_blocks_size_, rhs.extra_blocks_size_);
  marisa::swap(extra_blocks_capacity_, rhs.extra_blocks_capacity_);
  key_blocks_.swap(rhs.key_blocks_);
  marisa::swap(key_blocks_size_, rhs.key_blocks_size_);
  marisa::swap(key_blocks_capacity_, rhs.key_blocks_capacity_);
  marisa::swap(ptr_, rhs.ptr_);
  marisa::swap(avail_, rhs.avail_);
  marisa::swap(size_, rhs.size_);
  marisa::swap(total_length_, rhs.total_length_);
}

char *Keyset::reserve(std::size_t size) {
  if ((size_ / KEY_BLOCK_SIZE) == key_blocks_size_) {
    append_key_block();
  }

  if (size > EXTRA_BLOCK_SIZE) {
    append_extra_block(size);
    return extra_blocks_[extra_blocks_size_ - 1].get();
  } else {
    if (size > avail_) {
      append_base_block();
    }
    ptr_ += size;
    avail_ -= size;
    return ptr_ - size;
  }
}

void Keyset::append_base_block() {
  if (base_blocks_size_ == base_blocks_capacity_) {
    const std::size_t new_capacity =
        (base_blocks_size_ != 0) ? (base_blocks_size_ * 2) : 1;
    scoped_array<scoped_array<char> > new_blocks(
        new (std::nothrow) scoped_array<char>[new_capacity]);
    MARISA_THROW_IF(new_blocks.get() == NULL, MARISA_MEMORY_ERROR);
    for (std::size_t i = 0; i < base_blocks_size_; ++i) {
      base_blocks_[i].swap(new_blocks[i]);
    }
    base_blocks_.swap(new_blocks);
    base_blocks_capacity_ = new_capacity;
  }
  if (base_blocks_[base_blocks_size_].get() == NULL) {
    scoped_array<char> new_block(new (std::nothrow) char[BASE_BLOCK_SIZE]);
    MARISA_THROW_IF(new_block.get() == NULL, MARISA_MEMORY_ERROR);
    base_blocks_[base_blocks_size_].swap(new_block);
  }
  ptr_ = base_blocks_[base_blocks_size_++].get();
  avail_ = BASE_BLOCK_SIZE;
}

void Keyset::append_extra_block(std::size_t size) {
  if (extra_blocks_size_ == extra_blocks_capacity_) {
    const std::size_t new_capacity =
        (extra_blocks_size_ != 0) ? (extra_blocks_size_ * 2) : 1;
    scoped_array<scoped_array<char> > new_blocks(
        new (std::nothrow) scoped_array<char>[new_capacity]);
    MARISA_THROW_IF(new_blocks.get() == NULL, MARISA_MEMORY_ERROR);
    for (std::size_t i = 0; i < extra_blocks_size_; ++i) {
      extra_blocks_[i].swap(new_blocks[i]);
    }
    extra_blocks_.swap(new_blocks);
    extra_blocks_capacity_ = new_capacity;
  }
  scoped_array<char> new_block(new (std::nothrow) char[size]);
  MARISA_THROW_IF(new_block.get() == NULL, MARISA_MEMORY_ERROR);
  extra_blocks_[extra_blocks_size_++].swap(new_block);
}

void Keyset::append_key_block() {
  if (key_blocks_size_ == key_blocks_capacity_) {
    const std::size_t new_capacity =
        (key_blocks_size_ != 0) ? (key_blocks_size_ * 2) : 1;
    scoped_array<scoped_array<Key> > new_blocks(
        new (std::nothrow) scoped_array<Key>[new_capacity]);
    MARISA_THROW_IF(new_blocks.get() == NULL, MARISA_MEMORY_ERROR);
    for (std::size_t i = 0; i < key_blocks_size_; ++i) {
      key_blocks_[i].swap(new_blocks[i]);
    }
    key_blocks_.swap(new_blocks);
    key_blocks_capacity_ = new_capacity;
  }
  scoped_array<Key> new_block(new (std::nothrow) Key[KEY_BLOCK_SIZE]);
  MARISA_THROW_IF(new_block.get() == NULL, MARISA_MEMORY_ERROR);
  key_blocks_[key_blocks_size_++].swap(new_block);
}

}  // namespace marisa
