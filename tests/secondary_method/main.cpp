#include <iostream>
#include <cassert>

#include "Polynomial.h"
#include "RingModElt.h"
#include "NegaNussbaumer.h"
#include "NegaConvo.h"
#include "compat/Poly.h"

int main() {
  // Get two random polynomials
  typedef RingModElt<PARAM_Q> RingType;
  poly a, b;
  poly_create_random(&a);
  poly_create_random(&b);
  Polynomial<RingType> p1 = a.toPolynomial();
  Polynomial<RingType> p2 = b.toPolynomial();

  // Perform Nussbaumer's algorithm, noting number of operations
  NegaNussbaumer<RingType> nussbaumer(PARAM_N);
  RingType::getOpCount().reset();
  auto trans1 = nussbaumer.transformSlow(p1);
  auto trans2 = nussbaumer.transformFast(p2);

  // Try several methods of multiplications
  for(std::size_t i = 0; i < 2; ++i) {
    RingType::getOpCount().reset();

    NegaNussbaumer<RingType>::Transformed resTrans;
    const char* method;
    switch(i) {
    case 0:
      // Use the naive method
      resTrans = nussbaumer.componentwiseNaive(trans1, trans2);
      method = "Naive";
      break;

    case 1:
      // Use the karatsuba method
      resTrans = nussbaumer.componentwiseKaratsuba(trans1, trans2);
      method = "Karatsuba";
      break;
    }

    std::cout << method << " componentwise multiplication: " << RingType::getOpCount().reset() << std::endl;

    // Do the inverse transform
    auto result = nussbaumer.inverseTransform(resTrans);
    RingType::getOpCount().reset();

    // Validate the test succeeded
    if(result != naivemult_negacyclic(PARAM_N, p1, p2)) {
      std::cerr << "TEST FAILED: results not equal!" << std::endl;
      return 1;
    }
    RingType::getOpCount().reset();
  }

  return 0;
}
