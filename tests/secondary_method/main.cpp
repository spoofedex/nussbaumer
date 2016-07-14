#include <iostream>
#include <cassert>

#include "Polynomial.h"
#include "RingModElt.h"
#include "NegaNussbaumer.h"
#include "NegaConvo.h"
#include "Karatsuba.h"
#include "compat/Poly.h"

int main() {
  // Get two random polynomials
  typedef RingModElt<PARAM_Q> RingType;
  poly a, b;
  poly_create_random(&a);
  poly_create_random(&b);
  Polynomial<RingType> p1 = a.toPolynomial();
  Polynomial<RingType> p2 = b.toPolynomial();

  p1.setSize(32);
  p2.setSize(32);

  // Test the number of operations for Nussbaumer's algorithm
  NegaNussbaumer<RingType> nussbaumer(32);
  RingType::getOpCount().reset();
  auto trans1 = nussbaumer.transformSlow(p1);
  auto trans2 = nussbaumer.transformFast(p2);
  auto resTrans = nussbaumer.componentwise(trans1, trans2);
  auto result1 = nussbaumer.inverseTransform(resTrans);
  std::cout << "Nussbaumer's algorithm: " << RingType::getOpCount().reset() << std::endl;

  // Test the number of operations for the classical method.
  auto result2 = naivemult_negacyclic(32, p1, p2);
  std::cout << "Classical method: " << RingType::getOpCount().reset() << std::endl;

  // Test karatsuba's method (reduce manually)
  auto product = karatsuba(p1, p2);
  Polynomial<RingType> result3(32);
  for(std::size_t i = 0; i < 32 - 1; ++i)
    result3[i] = product[i] - product[i + 32];
  result3[31] = product[31];
  std::cout << "Karatsuba's method: " << RingType::getOpCount().reset() << std::endl;

  if(result1 != result2)
    std::cerr << "TEST FAILED: Method 1 and 2 mismatch" << std::endl;
  if(result1 != result3)
    std::cerr << "TEST FAILED: Method 1 and 3 mismatch" << std::endl;

  return 0;
}
