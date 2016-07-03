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
  std::cout << "Transform 1: " << RingType::getOpCount().reset() << std::endl;
  auto trans2 = nussbaumer.transformFast(p2);
  std::cout << "Transform 2: " << RingType::getOpCount().reset() << std::endl;
  auto resTrans = nussbaumer.componentwise(trans1, trans2);
  std::cout << "Recurse: " << RingType::getOpCount().reset() << std::endl;
  auto result = nussbaumer.inverseTransform(resTrans);
  std::cout << "Inverse transform: " << RingType::getOpCount().reset() << std::endl;
  std::cout << std::endl;

  RingType::getOpCount().reset();
  auto massTransSlow = nussbaumer.massTransform(p1, true);
  std::cout << "Mass transform 1: " << RingType::getOpCount().reset() << std::endl;
  auto massTransFast = nussbaumer.massTransform(p2, false);
  std::cout << "Mass transform 2: " << RingType::getOpCount().reset() << std::endl;
  auto resTrans2 = NegaNussbaumer<RingType>::massComponentWise(massTransSlow, massTransFast);
  std::cout << "Base case: " << RingType::getOpCount().reset() << std::endl;
  auto result2 = nussbaumer.massInverseTransform(resTrans2);
  std::cout << "Mass inverse transform: " << RingType::getOpCount().reset() << std::endl;


  // Validate the test succeeded
  if(result != result2) {
    std::cerr << "TEST FAILED: inconsistency between recursive/iterative approach!" << std::endl;
    return 1;
  }
  if(result != naivemult_negacyclic(PARAM_N, p1, p2)) {
    std::cerr << "TEST FAILED: results not equal!" << std::endl;
    return 1;
  }

  return 0;
}
