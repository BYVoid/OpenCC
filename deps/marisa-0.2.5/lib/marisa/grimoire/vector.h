#ifndef MARISA_GRIMOIRE_VECTOR_H_
#define MARISA_GRIMOIRE_VECTOR_H_

#include "marisa/grimoire/vector/vector.h"
#include "marisa/grimoire/vector/flat-vector.h"
#include "marisa/grimoire/vector/bit-vector.h"

namespace marisa {
namespace grimoire {

using vector::Vector;
typedef vector::FlatVector FlatVector;
typedef vector::BitVector BitVector;

}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_VECTOR_H_
