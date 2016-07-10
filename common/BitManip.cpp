#include <cstddef>

#include "BitManip.h"

/**
 * Calculates the integer with the last "n" least significant bits set to the same bits in "value",
 * in reversed order. The other bits will be set to 0.
 * @param[in] n        The number of bits to reverse.
 * @param[in] value    The value whose bits to reverse (bits past n are ignored)
 * @return    The n least significant bits in reversed order.
 */
std::size_t bitrev(unsigned long n, size_t value) {
  std::size_t ret = 0;
  for(unsigned int i = 0; i < n; ++i) {
    ret = (ret << 1) | (value & 1);
    value >>= 1;
  }

  return ret;
}
