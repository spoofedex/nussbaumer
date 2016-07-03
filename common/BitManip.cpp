#include <cstddef>

#include "BitManip.h"

std::size_t bitrev(unsigned long n, size_t value) {
  std::size_t ret = 0;
  for(unsigned int i = 0; i < n; ++i) {
    ret = (ret << 1) | (value & 1);
    value >>= 1;
  }

  return ret;
}
