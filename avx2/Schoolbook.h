#ifndef AVX2_SCHOOLBOOK_H_
#define AVX2_SCHOOLBOOK_H_

#include <cstdint>

/**
 * Test the schoolbook4 polynomial multiplication; the data represents 8 blocks of 16 sets of 16-bit integers
 * the first four blocks indicating the coefficients of the 16 polynomials to multiply, followde by four blocks
 * indicating the polynomials to multiply with. The result will be stored in 7 blocks of 16 sets of 16-bit
 * integers.
 * @param[in,out] data    The input/output source.
 */
extern "C" void schoolbook4_test(std::uint16_t* data);

#endif
