//
//  NegaNussbaumer.h
//  Thesis
//
//  Created by Gerben on 13-02-16.
//  Copyright © 2016 Gerben van der Lubbe. All rights reserved.
//

#ifndef NEGANUSSBAUMER_H
#define NEGANUSSBAUMER_H

#include <vector>
#include <cassert>
#include <cmath>

#include "Polynomial.h"
#include "BitManip.h"

/**
 * Class for performing the Negacyclic Nussbaumer algorithm.
 * To run it, transform both polynomials, perform a componentwise() product, and
 * calculate the inverse transform.
 */
template<typename RingElt>
class NegaNussbaumer {
public:
  /// Transformed polynomial
  typedef std::vector<Polynomial<RingElt>> Transformed;

  NegaNussbaumer(std::size_t N);

  Transformed transform(const Polynomial<RingElt>& orig) const;
  Polynomial<RingElt> inverseTransform(const Transformed& trans) const;
  Polynomial<RingElt> correct(const Polynomial<RingElt>& p) const;

  Transformed componentwise(const Transformed& t1, const Transformed& t2) const;

  static Polynomial<RingElt> multiply(std::size_t N, const Polynomial<RingElt>& p1, const Polynomial<RingElt>& p2);

protected:
  Polynomial<RingElt> addRotatedPolynomial(const Polynomial<RingElt>& p1, const Polynomial<RingElt>& p2, int steps) const;

  unsigned int getFactor() const;

private:
  std::size_t n_;
  std::size_t m_, r_;
};


/**
 * Constructor for an object that will perform multiplications on the given
 * polynomial modulo u^N + 1, according to the Nussbaumer algorithm. The value
 * "N" must be a power of 2 for this algorithm.
 * @param[in] N  The "N" of the algorithm; the multiplication is calculated
 *               modulo "u^N + 1". This must be greater than 2, as a trivial
 *               alternative should be used there.
 */
template<typename RingElt>
NegaNussbaumer<RingElt>::NegaNussbaumer(
                                    std::size_t N
                                        ) {
  assert(N > 1);

  // Get the n = log_2 N (which must be an integer)
  n_ = 0;
  while((1u << n_) < N)
    ++n_;
  assert((1u << n_) == N);

  // Find m = 2^lg_m and r = 2^lg_m (with lg_m and lg_r integers), such that
  // m*r = n with r minimum; that is, m = floor(lg_n/2) and lg_m + lg_r = n.
  std::size_t lg_m, lg_r;
  lg_m = n_ >> 1;
  lg_r = n_ - lg_m;
  m_ = 1 << lg_m;
  r_ = 1 << lg_r;
}


/**
 * Perform the full multiplication of the two given polynomials, modulo u^N + 1.
 * @param[in] N    The N in the modulo u^N + 1
 * @param[in] p1   The first polynomial to multiply.
 * @param[in] p2   The second polynomial to multiply.
 * @return    The Negacyclic convolution
 */
template<typename RingElt>
Polynomial<RingElt> NegaNussbaumer<RingElt>::multiply(
                                        std::size_t N,
                                        const Polynomial<RingElt>& p1,
                                        const Polynomial<RingElt>& p2
                                                     ) {
  // Trivial case modulo X^2 + 1.
  if(N == 2) {
    Polynomial<RingElt> ret(2);
    RingElt t = p1[0]*(p2[0] + p2[1]);
    ret[0] = t - (p1[0] + p1[1])*p2[1];
    ret[1] = t + (p1[1] - p1[0])*p2[0];
    return ret;
  }

  // Otherwise, recurse into the algorithm again
  NegaNussbaumer<RingElt> nussbaumer(N);
  auto t1 = nussbaumer.transform(p1);
  auto t2 = nussbaumer.transform(p2);
  auto resTrans = nussbaumer.componentwise(t1, t2);
  return nussbaumer.inverseTransform(resTrans);
}


/**
 * Perform the componentwise multiplication of the transformed polynomials.
 * @param[in] t1    The first transformed polynomial.
 * @param[in] t2    The second transformed polynomial.
 * @return The transformed result of the multiplication.
 */
template<typename RingElt>
typename NegaNussbaumer<RingElt>::Transformed NegaNussbaumer<RingElt>::componentwise(
                                                                   const Transformed& t1,
                                                                   const Transformed& t2
                                                                               ) const {
  Transformed resTrans;
  resTrans.push_back(Polynomial<RingElt>(r_));
  for(std::size_t i = 1; i < t1.size(); ++i) {
    auto term = NegaNussbaumer<RingElt>::multiply(r_, t1[i], t2[i]);
    resTrans.push_back(term);
  }

  return resTrans;
}


/**
 * Transform the polynomial to the list of polynomials that can be multiplied
 * componentwise (see algorithm description for a more thorough explanation).
 * @param[in] orig     The original polynomial, must be of degree N.
 * @return    The transformed polynomial.
 */
template<typename RingElt>
typename NegaNussbaumer<RingElt>::Transformed
                                    NegaNussbaumer<RingElt>::transform(
                                            const Polynomial<RingElt>& orig
                                                                       ) const {
  assert(orig.getSize() == (1u << n_));
  Transformed trans(2*m_, Polynomial<RingElt>(r_));

  // First get the polynomials to perform the fourier transform on. These are
  // 2m polynomials of which r coefficients will be considered, where two sets
  // of m polynomials are created by shuffling "orig".
  for(std::size_t i = 0; i < 2*m_; ++i) {
    for(std::size_t j = 0; j < r_; ++j) {
      trans[i][j] = orig[m_*j + (i % m_)];
    }
  }

  // Do the fast fourier transform.
  std::size_t j = (n_ >> 1);
  while(j > 0) {
    --j;

    for(std::size_t sPart = 0; sPart < (m_ >> j); ++sPart) {
      std::size_t s, sRev;
      s = sPart << (j+1);
      sRev = bitrev((n_ >> 1) - j, sPart) << j;

      int k = static_cast<int>((r_/m_)*sRev);

      for(std::size_t t = 0; t < (1u << j); ++t) {
        std::size_t e, f;
        e = s + t;
        f = e + (1u << j);

        // Now, we set (simultaneously):
        // trans[e] = trans[e] + u^k*trans[f]
        // trans[f] = trans[e] - u^k*trans[f]
        Polynomial<RingElt> tmp;
        if(e == 0 && j == 0) {
          // Don't calculate trans[0]; we don't need it.
          tmp = Polynomial<RingElt>(r_);
        }
        else {
          tmp = addRotatedPolynomial(trans[e], trans[f], k);
        }
        trans[f] = addRotatedPolynomial(trans[e], trans[f], k + r_);
        trans[e] = tmp;
      }
    }
  }

  return trans;
}



/**
 * Perform the inverse transform (see paper).
 * @param[in] trans    The transformed form of the polynomial.
 * @return    The polynomial form.
 */
template<typename RingElt>
Polynomial<RingElt> NegaNussbaumer<RingElt>::inverseTransform(
                                                    const Transformed& trans
                                                              ) const {
  // Do the inverse FFT (through a DIT with unordered input)
  Transformed z(trans);
  std::size_t jMax = n_ >> 1;
  for(std::size_t j = 0; j <= jMax; ++j) {
    for(std::size_t t = 0; t < (1u << j); ++t) {
      int k = -(int)((r_/m_)*( t << (jMax - j) ));

      for(std::size_t s = 0; s < 2*m_; s += (1 << (j+1))) {
        std::size_t e, f;
        e = s + t;
        f = e + (1u << j);

        Polynomial<RingElt> tmp;
        if(j == 0 && e == 0) {
          z[e] = z[f];
          z[f] = -z[f];
        }
        else {
          // Now, we set (simultaneously):
          // z[e] = z[e] + u^k*z[f]
          // z[f] = z[e] - u^k*z[f]
          tmp = addRotatedPolynomial(z[e], z[f], k);
          z[f] = addRotatedPolynomial(z[e], z[f], k + r_);
          z[e] = tmp;
        }
      }
    }
  }

  // Subtract the last polynomial from each other
  for(std::size_t i = 0; i < 2*m_ - 1; ++i) {
    z[i] -= z[2*m_ - 1];
  }

  // Unpack the polynomial
  Polynomial<RingElt> res(1u << n_);
  for(std::size_t i = 0; i < m_ - 1; ++i) {
    res[i] = z[i][0] - z[m_ + i][r_ - 1];
    for(std::size_t j = 1; j < r_; ++j) {
      res[m_*j + i] = z[i][j] + z[m_ + i][j - 1];
    }
  }

  for(std::size_t j = 0; j < r_; ++j)
    res[m_*j + m_ - 1] = z[m_ - 1][j];

  return res;
}


/**
 * Calculate p1 + (u^steps)*p2 modulo u^r + 1. This could be done by a separate
 * rotate/add step, but this would require possible negations followed by
 * additions, rather than immediately subtracting.
 * @param[in] p1      The first polynomial
 * @param[in] p2      The second polynomial.
 * @param[in] steps   The number of steps to "rotate" p2.
 * @return    p1 + (u^steps)*p2 modulo u^r + 1.
 */
template<typename RingElt>
Polynomial<RingElt> NegaNussbaumer<RingElt>::addRotatedPolynomial(
                                                 const Polynomial<RingElt>& p1,
                                                 const Polynomial<RingElt>& p2,
                                                 int steps
                                                                 ) const {
  Polynomial<RingElt> ret(r_);

  // Get "steps" as negative value, with 2*r_ < steps <= 0
  steps %= 2*r_;
  if(steps > 0)
    steps -= 2*r_;

  for(std::size_t i = 0; i < r_; ++i) {
    int src = static_cast<int>(i) - steps;
    std::size_t srcIndex = static_cast<size_t>(src) % r_;
    bool signInverse = (src / r_) & 1;

    if(signInverse)
      ret[i] = p1[i] - p2[srcIndex];
    else
      ret[i] = p1[i] + p2[srcIndex];
  }

  return ret;
}


/*
 * Corrects the result of the Nussbaumer algorithm by dividing the factor
 * of getFactor out of this one.
 * @param[in] p    The polynomial to correct.
 * @return    The polynomial.
 */
template<typename RingElt>
Polynomial<RingElt> NegaNussbaumer<RingElt>::correct(const Polynomial<RingElt>& p) const {
  // To calculate the inverse FFT we need the inverse of the correction factor
  RingElt inverseElt;
  int inverse;
  if(!RingElt::getInverse(inverseElt, getFactor())) {
    std::cerr << "Factor does not have an inverse in the given ring" << std::endl;
    exit(1);
  }

  inverse = inverseElt.toInt();

  // Multiply with the inverse
  auto ret = p;
  ret *= inverse;
  return ret;
}


/**
 * Calculate the factor that the result is multiplied with after the Nussbaumer
 * algorithm. The final step should be to multiply the result with the inverse
 * of this factor.
 * @return    The factor.
 */
template<typename RingElt>
unsigned int NegaNussbaumer<RingElt>::getFactor() const {
  unsigned int factor = 1;
  unsigned int nTest = n_;
  while(nTest != 1) {
    unsigned int lgmTest = nTest >> 1;
    unsigned int lgrTest = nTest - lgmTest;
    factor *= 2*(1 << lgmTest);
    nTest = lgrTest;
  }
  return factor;
}


#endif
