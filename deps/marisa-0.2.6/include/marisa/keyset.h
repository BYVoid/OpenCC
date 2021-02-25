#ifndef MARISA_KEYSET_H_
#define MARISA_KEYSET_H_

#include "marisa/key.h"

namespace marisa {

class Keyset {
 public:
  enum {
    BASE_BLOCK_SIZE  = 4096,
    EXTRA_BLOCK_SIZE = 1024,
    KEY_BLOCK_SIZE   = 256
  };

  Keyset();

  void push_back(const Key &key);
  void push_back(const Key &key, char end_marker);

  void push_back(const char *str);
  void push_back(const char *ptr, std::size_t length, float weight = 1.0);

  const Key &operator[](std::size_t i) const {
    MARISA_DEBUG_IF(i >= size_, MARISA_BOUND_ERROR);
    return key_blocks_[i / KEY_BLOCK_SIZE][i % KEY_BLOCK_SIZE];
  }
  Key &operator[](std::size_t i) {
    MARISA_DEBUG_IF(i >= size_, MARISA_BOUND_ERROR);
    return key_blocks_[i / KEY_BLOCK_SIZE][i % KEY_BLOCK_SIZE];
  }

  std::size_t num_keys() const {
    return size_;
  }

  bool empty() const {
    return size_ == 0;
  }
  std::size_t size() const {
    return size_;
  }
  std::size_t total_length() const {
    return total_length_;
  }

  void reset();

  void clear();
  void swap(Keyset &rhs);

 private:
  scoped_array<scoped_array<char> > base_blocks_;
  std::size_t base_blocks_size_;
  std::size_t base_blocks_capacity_;
  scoped_array<scoped_array<char> > extra_blocks_;
  std::size_t extra_blocks_size_;
  std::size_t extra_blocks_capacity_;
  scoped_array<scoped_array<Key> > key_blocks_;
  std::size_t key_blocks_size_;
  std::size_t key_blocks_capacity_;
  char *ptr_;
  std::size_t avail_;
  std::size_t size_;
  std::size_t total_length_;

  char *reserve(std::size_t size);

  void append_base_block();
  void append_extra_block(std::size_t size);
  void append_key_block();

  // Disallows copy and assignment.
  Keyset(const Keyset &);
  Keyset &operator=(const Keyset &);
};

}  // namespace marisa

#endif  // MARISA_KEYSET_H_
