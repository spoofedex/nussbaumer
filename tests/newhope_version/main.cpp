#include <iostream>
#include <fstream>

#include "Polynomial.h"
#include "compat/Poly.h"
#include "NegaConvo.h"


static void nttmul(poly *r, const poly *x, const poly *y)
{
  poly a,b;
  a = *x;
  b = *y;

  opCountIntWrapper.reset();
  poly_bitrev(&a);
  std::cout << "Bitrev 1: " << opCountIntWrapper.reset() << std::endl;
  poly_bitrev(&b);
  std::cout << "Bitrev 2: " << opCountIntWrapper.reset() << std::endl;
  poly_ntt(&a);
  std::cout << "NTT 1: " << opCountIntWrapper.reset() << std::endl;
  poly_ntt(&b);
  std::cout << "NTT 2: " << opCountIntWrapper.reset() << std::endl;

  poly_pointwise(r,&a,&b);
  std::cout << "Pointwise: " << opCountIntWrapper.reset() << std::endl;

  poly_bitrev(r);
  std::cout << "Bitrev 3: " << opCountIntWrapper.reset() << std::endl;
  poly_invntt(r);
  std::cout << "Inv NTT: " << opCountIntWrapper.reset() << std::endl;
}


int main()
{
  poly a, b;
  poly_create_random(&a);
  poly_create_random(&b);

  poly r1, r2;
  nttmul(&r1,&a,&b);

  // Get the answer, modulo PARAM_Q
  Polynomial<IntWrapper<std::uint64_t>> res1 = r1.toPolynomial();
  for(std::size_t i = 0; i < res1.getSize(); ++i)
    res1[i] %= PARAM_Q;

  // Calculate the expected value.
  // Fix the case where the result was actually negative before calculating the modulo.
  // The addition makes sure that any underflow is corrected. The result will always be
  // positive then, and the same answer modulo PARAM_Q.
  // For this not to overflow again, make sure that the calculations happen in std::uint64_t.
  // This is easily seen not to overflow for N = 1024
  Polynomial<IntWrapper<std::uint64_t>> aExt(a.toPolynomial());
  Polynomial<IntWrapper<std::uint64_t>> bExt(b.toPolynomial());
  auto res2 = naivemult_negacyclic(PARAM_N, aExt, bExt);
  for(std::size_t i = 0; i < res2.getSize(); ++i)
    res2[i] = (res2[i] + PARAM_Q*PARAM_Q*2ull*PARAM_N) % PARAM_Q;

  if(res1 != res2) {
    std::cerr << "TEST FAILED: results not equal!" << std::endl;
    return 1;
  }
}
