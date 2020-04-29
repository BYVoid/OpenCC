#ifndef MARISA_GRIMOIRE_VECTOR_RANK_INDEX_H_
#define MARISA_GRIMOIRE_VECTOR_RANK_INDEX_H_

#include "marisa/base.h"

namespace marisa {
namespace grimoire {
namespace vector {

class RankIndex {
 public:
  RankIndex() : abs_(0), rel_lo_(0), rel_hi_(0) {}

  void set_abs(std::size_t value) {
    MARISA_DEBUG_IF(value > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    abs_ = (UInt32)value;
  }
  void set_rel1(std::size_t value) {
    MARISA_DEBUG_IF(value > 64, MARISA_RANGE_ERROR);
    rel_lo_ = (UInt32)((rel_lo_ & ~0x7FU) | (value & 0x7FU));
  }
  void set_rel2(std::size_t value) {
    MARISA_DEBUG_IF(value > 128, MARISA_RANGE_ERROR);
    rel_lo_ = (UInt32)((rel_lo_ & ~(0xFFU << 7)) | ((value & 0xFFU) << 7));
  }
  void set_rel3(std::size_t value) {
    MARISA_DEBUG_IF(value > 192, MARISA_RANGE_ERROR);
    rel_lo_ = (UInt32)((rel_lo_ & ~(0xFFU << 15)) | ((value & 0xFFU) << 15));
  }
  void set_rel4(std::size_t value) {
    MARISA_DEBUG_IF(value > 256, MARISA_RANGE_ERROR);
    rel_lo_ = (UInt32)((rel_lo_ & ~(0x1FFU << 23)) | ((value & 0x1FFU) << 23));
  }
  void set_rel5(std::size_t value) {
    MARISA_DEBUG_IF(value > 320, MARISA_RANGE_ERROR);
    rel_hi_ = (UInt32)((rel_hi_ & ~0x1FFU) | (value & 0x1FFU));
  }
  void set_rel6(std::size_t value) {
    MARISA_DEBUG_IF(value > 384, MARISA_RANGE_ERROR);
    rel_hi_ = (UInt32)((rel_hi_ & ~(0x1FFU << 9)) | ((value & 0x1FFU) << 9));
  }
  void set_rel7(std::size_t value) {
    MARISA_DEBUG_IF(value > 448, MARISA_RANGE_ERROR);
    rel_hi_ = (UInt32)((rel_hi_ & ~(0x1FFU << 18)) | ((value & 0x1FFU) << 18));
  }

  std::size_t abs() const {
    return abs_;
  }
  std::size_t rel1() const {
    return rel_lo_ & 0x7FU;
  }
  std::size_t rel2() const {
    return (rel_lo_ >> 7) & 0xFFU;
  }
  std::size_t rel3() const {
    return (rel_lo_ >> 15) & 0xFFU;
  }
  std::size_t rel4() const {
    return (rel_lo_ >> 23) & 0x1FFU;
  }
  std::size_t rel5() const {
    return rel_hi_ & 0x1FFU;
  }
  std::size_t rel6() const {
    return (rel_hi_ >> 9) & 0x1FFU;
  }
  std::size_t rel7() const {
    return (rel_hi_ >> 18) & 0x1FFU;
  }

 private:
  UInt32 abs_;
  UInt32 rel_lo_;
  UInt32 rel_hi_;
};

}  // namespace vector
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_VECTOR_RANK_INDEX_H_
