#ifndef AVX2_COMPONENTWISE_H_
#define AVX2_COMPONENTWISE_H_

/**
 * Prepare for componentwise multiplication of 64 polynomials with 32 coefficients.
 * @param[in,out] data   The data representing the polynomials, starting with the 64 first coefficients, etc.
 */
extern "C" void componentwise32_64_prepare(std::uint16_t* data);

/**
 * Actually perform the multiplication of the two prepared polynomials.
 * @param[out] dest      Stores the result.
 * @param[in]  prep1     The first polynomials, prepared.
 * @param[in]  prep2     The second polynomials, prepared.
 */
extern "C" void componentwise32_64_run(std::uint16_t* dest, std::uint16_t* prep1, std::uint16_t* prep2);

#endif
