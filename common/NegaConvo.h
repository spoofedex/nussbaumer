/**
 * @file NegaConvo.h
 * @author Gerben van der Lubbe
 *
 * Naive algorithm for calculating the negacyclic convolution; just to use as a check.
 */

#ifndef NEGACONVO_H
#define NEGACONVO_H

#include "Polynomial.h"

/**
 * Naive algorithm for calculating p1*p2 reduced modulo x^n + 1. This can be
 * used to double-check the result of Nussbaumer's algorithm.
 * @param[in] n    The n in x^n + 1, the modulo to calculate the product in.
 * @param[in] p1   The first polynomial.
 * @param[in] p2   The second polynomial.
 */
template<typename RingElt>
auto naivemult_negacyclic(
                     std::size_t n,
                     const Polynomial<RingElt>& p1,
                     const Polynomial<RingElt>& p2) {
  assert(n > 0);


  // Calculate the product polynomial
  Polynomial<RingElt> product(p1);
  product *= p2;

  // For each term X^i with i >= n...
  for(std::size_t i = product.getSize() - 1; i >= n; --i) {
    // Subtract c*X^k*(X^n + 1) with k+n = i to make the highest order term
    // disappear (that is, c = product[i]).
    product[i - n] -= product[i];
    product[i] = 0;
  }

  product.setSize(n);
  return product;
}


#endif
