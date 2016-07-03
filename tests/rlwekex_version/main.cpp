#include <iostream>
#include <fstream>

#include "Polynomial.h"
#include "RingModElt.h"
#include "compat/Poly.h"
#include "NegaConvo.h"
#include "rlwekex/fft.h"


int main()
{
  poly a, b;
  poly_create_random(&a);
  poly_create_random(&b);
  Polynomial<FftRingType> p1(a.toPolynomial());
  Polynomial<FftRingType> p2(b.toPolynomial());
  Polynomial<FftRingType> result(1024);

  FFT_CTX ctx;
  if(!FFT_CTX_init(&ctx)) {
    std::cerr << "Failed to initialize FFT CTX" << std::endl;
    return 1;
  }

  // Polynomials are stored in vectors, so the memory is guaranteed to be contiguous.
  FFT_mul(&result[0], &p1[0], &p2[0], &ctx);
  FFT_CTX_free(&ctx);

  std::cout << "RLWEKEX: " << FftRingType::getOpCount() << std::endl;

  // Calculate the expected value.
  if(result != naivemult_negacyclic(PARAM_N, p1, p2)) {
    std::cerr << "TEST FAILED: results not equal!" << std::endl;
    return 1;
  }
}
