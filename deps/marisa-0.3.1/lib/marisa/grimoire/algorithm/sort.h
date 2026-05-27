#ifndef MARISA_GRIMOIRE_ALGORITHM_SORT_H_
#define MARISA_GRIMOIRE_ALGORITHM_SORT_H_

#include <cassert>

#include "marisa/base.h"

namespace marisa::grimoire::algorithm {
namespace details {

enum {
  MARISA_INSERTION_SORT_THRESHOLD = 10
};

template <typename T>
int get_label(const T &unit, std::size_t depth) {
  assert(depth <= unit.length());

  return (depth < unit.length()) ? int{static_cast<uint8_t>(unit[depth])} : -1;
}

template <typename T>
int median(const T &a, const T &b, const T &c, std::size_t depth) {
  const int x = get_label(a, depth);
  const int y = get_label(b, depth);
  const int z = get_label(c, depth);
  if (x < y) {
    if (y < z) {
      return y;
    } else if (x < z) {
      return z;
    }
    return x;
  } else if (x < z) {
    return x;
  } else if (y < z) {
    return z;
  }
  return y;
}

template <typename T>
int compare(const T &lhs, const T &rhs, std::size_t depth) {
  for (std::size_t i = depth; i < lhs.length(); ++i) {
    if (i == rhs.length()) {
      return 1;
    }
    if (lhs[i] != rhs[i]) {
      return static_cast<uint8_t>(lhs[i]) - static_cast<uint8_t>(rhs[i]);
    }
  }
  if (lhs.length() == rhs.length()) {
    return 0;
  }
  return (lhs.length() < rhs.length()) ? -1 : 1;
}

template <typename Iterator>
std::size_t insertion_sort(Iterator l, Iterator r, std::size_t depth) {
  assert(l <= r);

  std::size_t count = 1;
  for (Iterator i = l + 1; i < r; ++i) {
    int result = 0;
    for (Iterator j = i; j > l; --j) {
      result = compare(*(j - 1), *j, depth);
      if (result <= 0) {
        break;
      }
      std::swap(*(j - 1), *j);
    }
    if (result != 0) {
      ++count;
    }
  }
  return count;
}

template <typename Iterator>
std::size_t sort(Iterator l, Iterator r, std::size_t depth) {
  assert(l <= r);

  std::size_t count = 0;
  while ((r - l) > MARISA_INSERTION_SORT_THRESHOLD) {
    Iterator pl = l;
    Iterator pr = r;
    Iterator pivot_l = l;
    Iterator pivot_r = r;

    const int pivot = median(*l, *(l + (r - l) / 2), *(r - 1), depth);
    for (;;) {
      while (pl < pr) {
        const int label = get_label(*pl, depth);
        if (label > pivot) {
          break;
        } else if (label == pivot) {
          std::swap(*pl, *pivot_l);
          ++pivot_l;
        }
        ++pl;
      }
      while (pl < pr) {
        const int label = get_label(*--pr, depth);
        if (label < pivot) {
          break;
        } else if (label == pivot) {
          std::swap(*pr, *--pivot_r);
        }
      }
      if (pl >= pr) {
        break;
      }
      std::swap(*pl, *pr);
      ++pl;
    }
    while (pivot_l > l) {
      std::swap(*--pivot_l, *--pl);
    }
    while (pivot_r < r) {
      std::swap(*pivot_r, *pr);
      ++pivot_r;
      ++pr;
    }

    if (((pl - l) > (pr - pl)) || ((r - pr) > (pr - pl))) {
      if ((pr - pl) == 1) {
        ++count;
      } else if ((pr - pl) > 1) {
        if (pivot == -1) {
          ++count;
        } else {
          count += sort(pl, pr, depth + 1);
        }
      }

      if ((pl - l) < (r - pr)) {
        if ((pl - l) == 1) {
          ++count;
        } else if ((pl - l) > 1) {
          count += sort(l, pl, depth);
        }
        l = pr;
      } else {
        if ((r - pr) == 1) {
          ++count;
        } else if ((r - pr) > 1) {
          count += sort(pr, r, depth);
        }
        r = pl;
      }
    } else {
      if ((pl - l) == 1) {
        ++count;
      } else if ((pl - l) > 1) {
        count += sort(l, pl, depth);
      }

      if ((r - pr) == 1) {
        ++count;
      } else if ((r - pr) > 1) {
        count += sort(pr, r, depth);
      }

      l = pl, r = pr;
      if ((pr - pl) == 1) {
        ++count;
      } else if ((pr - pl) > 1) {
        if (pivot == -1) {
          l = r;
          ++count;
        } else {
          ++depth;
        }
      }
    }
  }

  if ((r - l) > 1) {
    count += insertion_sort(l, r, depth);
  }
  return count;
}

}  // namespace details

template <typename Iterator>
std::size_t sort(Iterator begin, Iterator end) {
  assert(begin <= end);
  return details::sort(begin, end, 0);
}

}  // namespace marisa::grimoire::algorithm

#endif  // MARISA_GRIMOIRE_ALGORITHM_SORT_H_
