#ifndef AVX2_NUSSBAUMER_H_
#define AVX2_NUSSBAUMER_H_

#include <cstdint>

/**
 * Type for storing transformed polynomials; big enough to run the in-place preparation for
 * componentwise multiplication.
 */
typedef std::uint16_t Transformed[6912] __attribute__((aligned(32)));

/**
 * Transformed result; not enough space for componentwise multiplication, but enough for a
 * result.
 */
typedef std::uint16_t TransformedResult[6912] __attribute__((aligned(32)));

/**
 * Performs the forward Nussbaumer transform.
 * @param[out] dest    The memory to store the transformed input.
 * @param[in]  src     The coefficients of the polynomial.
 */
extern "C" void nussbaumer1024_forward(Transformed dest, std::uint16_t* src);

/**
 * Calculate the inverse Nussbaumer transform.
 * @param[out] dest    The memory region to store the result.
 * @param[in] src      The transformed value.
 */
extern "C" void nussbaumer1024_inverse(TransformedResult dest, Transformed src);

#endif
