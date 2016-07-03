//
//  Karatsuba.h
//  Thesis
//
//  Created by Gerben on 17-04-16.
//  Copyright © 2016 Gerben van der Lubbe. All rights reserved.
//

#ifndef KARATSUBA_H
#define KARATSUBA_H

#include <cassert>
#include <iostream>

#include "Polynomial.h"

/**
 * Perform the Karatsuba method of polynomial multiplication. The two
 * polynomials must be of equal size, and must be a power of 2.
 * @param[in] p1     The first polynomial.
 * @param[in] p2     The second polynomial.
 * @return    The product of p1 and p2.
 */
template<typename RingType>
Polynomial<RingType> karatsuba(const Polynomial<RingType>& p1,
                               const Polynomial<RingType>& p2) {
  assert(p1.getSize() == p2.getSize());
  std::size_t n = p1.getSize();
  if(n == 1)  // Base case for recursion, multiplication of constants
    return Polynomial<RingType>{p1[0]*p2[0]};

  std::size_t m = n/2;
  assert((n & 1) == 0);

  // Split the two polynomials into two halves
  Polynomial<RingType> p1Low(m), p1High(n - m);
  Polynomial<RingType> p2Low(m), p2High(n - m);
  std::size_t from, to;
  for(from = 0; from < p1Low.getSize(); ++from) {
    p1Low[from] = p1[from];
    p2Low[from] = p2[from];
  }
  for(to = 0; from < n; ++from, ++to) {
    p1High[to] = p1[from];
    p2High[to] = p2[from];
  }

  /*
  * We multiply (a + bx^m)(c + dx^m) = ac + (ad + bc)x^m + bdx^{2m}
  *  = ac + [(a + b)(c + d) - ac - bd]x^m + bdx^{2m}
  **/
  Polynomial<RingType> ac = karatsuba(p1Low, p2Low);
  Polynomial<RingType> bd = karatsuba(p1High, p2High);
  Polynomial<RingType> prod = karatsuba(p1Low + p1High, p2Low + p2High);
  Polynomial<RingType> ret(2*n - 1);                     // Big enough to hold p1*p2

  for(from = 0; from < 2*m - 1; ++from)                  // Set the first half to AC
    ret[from] = ac[from];
  for(from = m, to = 2*m; from < 2*m - 1; ++from, ++to)  // Set the third quarter to the second half of prod
    ret[to] = prod[from];
  for(from = m, ++to; from < 2*m - 1; ++from, ++to)      // Set the fourth quarter to the second half of bd.
    ret[to] = bd[from];

  // Set the second quarter to the second half of AC minus the first half of BD
  for(from = 0, to = m; to < 2*m - 1; ++from, ++to)
    ret[to] -= bd[from];

  // Re-use this second quarter by subtracting it to the third quarter; as this requires the same calculations.
  for(from = m, to = 2*m; from < 2*m - 1; ++from, ++to)
    ret[to] -= ret[from];
  ret[3*m - 1] = bd[m - 1];

  // Add the remaining items to the second quarter
  for(from = 0, to = m; from < m - 1; ++from, ++to)
    ret[to] += prod[from] - ac[from];
  ret[2*m - 1] = prod[m - 1] - bd[m - 1] - ac[m - 1];

  // Add the remaining (second half of bd) to the third quater
  for(from = m, to = 2*m; from < 2*m - 1; ++from, ++to)
    ret[to] -= bd[from];

  return ret;
}

#endif
