/**
 * @file NegaNussbaumer.h
 * @author Gerben van der Lubbe
 *
 * File containing Nussbaumer's negacyclic convolution algorithm (see paper).
 */

#ifndef NEGANUSSBAUMER_H
#define NEGANUSSBAUMER_H

#include <vector>
#include <cassert>
#include <cmath>

#include "Polynomial.h"
#include "BitManip.h"

/**
 * Class for performing the Negacyclic Nussbaumer algorithm.
 * To run it, transform both polynomials, one with the slow transform and
 * one with the fast transform; perform a componentwise() product, and
 * calculate the inverse transform.
 */
template<typename RingElt>
class NegaNussbaumer {
public:
  /// Transformed polynomial
  typedef std::vector<Polynomial<RingElt>> Transformed;
  typedef std::vector<Transformed> MassTransformed;

  NegaNussbaumer(std::size_t N);

  Transformed transformSlow(const Polynomial<RingElt>& orig, bool fixFactor = true) const;
  Transformed transformFast(const Polynomial<RingElt>& orig) const;
  Polynomial<RingElt> inverseTransform(const Transformed& trans) const;

  Transformed componentwise(const Transformed& t1, const Transformed& t2) const;

  MassTransformed massTransform(const Polynomial<RingElt>& orig, bool slow) const;
  static MassTransformed massComponentWise(const MassTransformed& fast, const MassTransformed& slow);
  Polynomial<RingElt> massInverseTransform(const MassTransformed& trans);

  static Polynomial<RingElt> multiply(std::size_t N, const Polynomial<RingElt>& p1, const Polynomial<RingElt>& p2);

protected:
  Polynomial<RingElt> addRotatedPolynomial(const Polynomial<RingElt>& p1, const Polynomial<RingElt>& p2, int steps) const;
  Polynomial<RingElt> rotatePolynomial(const Polynomial<RingElt>& pol, int steps) const;

  Polynomial<RingElt> correct(const Polynomial<RingElt>& p) const;
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
  // Otherwise, recurse into the algorithm again
  NegaNussbaumer<RingElt> nussbaumer(N);
  auto t1 = nussbaumer.transformSlow(p1, false);
  auto t2 = nussbaumer.transformFast(p2);
  auto resTrans = nussbaumer.componentwise(t1, t2);
  return nussbaumer.inverseTransform(resTrans);
}


/**
 * Perform the componentwise multiplication of the transformed polynomials. One must be
 * transformed through the transformSlow method, the other through the transformFast
 * method.
 * @param[in] slow   The slow-transformed polynomial.
 * @param[in] fast   The fast-transformed polynomial.
 * @return The transformed result of the multiplication.
 */
template<typename RingElt>
typename NegaNussbaumer<RingElt>::Transformed NegaNussbaumer<RingElt>::componentwise(
                                                                   const Transformed& slow,
                                                                   const Transformed& fast
                                                                               ) const {
  // Special case where N = 2, where Nussbaumer's algorithm is not applicable.
  if(n_ == 1) {
    Transformed resTrans;
    Polynomial<RingElt> res(2);
    RingElt t = slow[0][0]*fast[0][2];
    res[0] = t - slow[0][1]*fast[0][1];
    res[1] = t + slow[0][2]*fast[0][0];
    resTrans.push_back(res);
    return resTrans;
  }

  Transformed resTrans;
  resTrans.push_back(Polynomial<RingElt>(r_));
  for(std::size_t i = 1; i < slow.size(); ++i) {
    auto term = NegaNussbaumer<RingElt>::multiply(r_, slow[i], fast[i]);
    resTrans.push_back(term);
  }

  return resTrans;
}


/**
 * Transform the polynomial to the list of polynomials that can be multiplied
 * componentwise (see algorithm description for a more thorough explanation). This
 * is the slow transform; the other polynomial input for "componentwise" must be a
 * polynomial transformed by transformFast.
 * @param[in] orig     The original polynomial, must be of degree N.
 * @param[in] fixFactor Whether to compensate for the factor (should be false for recursive calls).
 * @return    The transformed polynomial.
 */
template<typename RingElt>
typename NegaNussbaumer<RingElt>::Transformed
                                NegaNussbaumer<RingElt>::transformSlow(
                                            const Polynomial<RingElt>& orig,
                                            bool fixFactor
                                                                      ) const {
  assert(orig.getSize() == (1u << n_));

  // Handle the special case N = 2, where another algorithm is used (with some pre-processing)
  // We make 2 as the first componentwise multiplication is skipped.
  if(n_ == 1) {
    Transformed trans(1, Polynomial<RingElt>(3));
    trans[0][0] = orig[0];
    trans[0][1] = orig[0] + orig[1];
    trans[0][2] = orig[1] - orig[0];
    return trans;
  }

  Transformed trans(2*m_, Polynomial<RingElt>(r_));

  // Correct the scale of the polynomial. Do so now, as this needs less steps then
  // at a later point. Also, as this result may be re-used, it's better to do here
  // than at the end.
  Polynomial<RingElt> scaledOrig = fixFactor ? correct(orig) : orig;

  // Get the input polynomials, transformed, applying the C_4' matrix immediately.
  for(std::size_t j = 0; j < r_; ++j)
    trans[0][j] = scaledOrig[m_*j];
  for(std::size_t i = 1; i < m_; ++i)
    trans[i][0] = -scaledOrig[m_*(r_  - 1) + m_ - i];
  for(std::size_t i = 1; i < m_; ++i) {
    for(std::size_t j = 1; j < r_; ++j) {
      trans[i][j] = scaledOrig[m_*(j - 1) + m_ - i];
    }
  }

  // Apply the C_3^T matrix.
  for(std::size_t i = 0; i < m_ - 1; ++i)
    trans[m_ + i][0] = -trans[i][r_ - 1];
  for(std::size_t i = 0; i < m_ - 1; ++i) {
    for(std::size_t j = 1; j < r_; ++j) {
      trans[m_ + i][j] = trans[i][j - 1];
    }
  }

  // Apply the C_2^T matrix.
  Polynomial<RingElt> lastEntry(-trans[0]);
  for(std::size_t i = 1; i < 2*m_ - 1; ++i)
    lastEntry -= trans[i];

  trans[2*m_ - 1] = lastEntry;

  // Perform the FFT
  std::size_t j = (n_ >> 1) + 1;
  while(j > 0) {
    --j;

    for(std::size_t sPart = 0; sPart < (m_ >> j); ++sPart) {
      std::size_t s, sRev;
      s = sPart << (j+1);
      sRev = bitrev((n_ >> 1) - j, sPart) << j;

      int k = -static_cast<int>((r_/m_)*sRev);

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
 * Transform the polynomial to the list of polynomials that can be multiplied
 * componentwise (see algorithm description for a more thorough explanation). This
 * is the fast transform; the other polynomial input for "componentwise" must be a
 * polynomial transformed by transformSlow.
 * @param[in] orig     The original polynomial, must be of degree N.
 * @return    The transformed polynomial.
 */
template<typename RingElt>
typename NegaNussbaumer<RingElt>::Transformed
                                NegaNussbaumer<RingElt>::transformFast(
                                            const Polynomial<RingElt>& orig
                                                                      ) const {
  assert(orig.getSize() == (1u << n_));

  // Prepare for another algorithm if N = 2.
  if(n_ == 1) {
    Transformed trans(1, Polynomial<RingElt>(3));
    trans[0][0] = orig[0];
    trans[0][1] = orig[1];
    trans[0][2] = orig[0] + orig[1];
    return trans;
  }


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
  // Special case for N = 2, where no actual inverse transform is needed
  if(n_ == 1)
    return trans[0];

  // Do the inverse FFT (through a DIT with unordered input)
  Transformed z(trans);
  std::size_t jMax = n_ >> 1;
  for(std::size_t j = 0; j <= jMax; ++j) {
    for(std::size_t t = 0; t < (1u << j); ++t) {
      int k = (int)((r_/m_)*( t << (jMax - j) ));

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
          if(j != jMax)  // We don't need the last half of the result.
            z[f] = addRotatedPolynomial(z[e], z[f], k + r_);
          z[e] = tmp;
        }
      }
    }
  }

  // Inverse the order of all but the first element.
  std::size_t from, to;
  for(from = 1, to = m_ - 1; from < to; from++, to--) {
    std::swap(z[from], z[to]);
  }

  // Multiply all but the first element with -u^{r-1} = u^{2r - 1}
  for(std::size_t i = 1; i < m_; ++i)
    z[i] = rotatePolynomial(z[i], 2*r_ - 1);

  // Unpack the polynomial
  Polynomial<RingElt> res(1u << n_);
  for(std::size_t i = 0; i < m_; ++i) {
    for(std::size_t j = 0; j < r_; ++j) {
      res[m_*j + i] = z[i][j];
    }
  }

  return res;
}


/**
 * Perform all forward transforms we can do from the get-go, rather than postponing
 * this to the recursive calls. This allows more work to be re-used.
 * @param[in] orig     The original polynomial.
 * @param[in] slow     true to use the slow transform, false for fast.
 * @return    A mass-transformed collection.
 */
template<typename RingElt>
typename NegaNussbaumer<RingElt>::MassTransformed
NegaNussbaumer<RingElt>::massTransform(const Polynomial<RingElt>& orig,
                                       bool slow) const {
  size_t N = 1 << n_;

  // Transform the original
  MassTransformed last;
  last.push_back(slow ? transformSlow(orig, true) : transformFast(orig));
  if(N == 2)
    return last;

  // Transform all the transformed polynomials iteratively
  N = r_;
  while(true) {
    NegaNussbaumer<RingElt> nb(N);
    MassTransformed newTrans;

    // Transform for the next level. Skip the first polynomial, which is always 0.
    for(auto trans : last)
      for(auto polIt = trans.begin() + 1; polIt != trans.end(); ++polIt)
        newTrans.push_back(slow ? nb.transformSlow(*polIt, false) : nb.transformFast(*polIt));

    last = std::move(newTrans);
    if(N == 2)
      break;
    N = nb.r_;
  }

  return last;
}


/**
 * Perform the base case of the iterative version of Nussbaumer's algorithm.
 * @param[in] slow     A slow mass transformed set.
 * @param[in] fast     A fast mass transformed set.
 * @return    The transformed output, with no inverse transforms performed yet.
 */
template<typename RingElt>
typename NegaNussbaumer<RingElt>::MassTransformed
NegaNussbaumer<RingElt>::massComponentWise(const MassTransformed& slow, const MassTransformed& fast) {
  // We can use multiply here, as the mass transformed outputs have 2 coefficients (not recursing)
  MassTransformed result;
  NegaNussbaumer<RingElt> nb(2);
  for(std::size_t i = 0; i < fast.size(); ++i) {
    result.push_back(nb.componentwise(slow[i], fast[i]));
  }
  return result;
}


/**
 * Perform an inverse for the iterative approach to Nussbaumer's algorithm. The trans input should
 * be the output from massComponentWise.
 * @param[in] trans    The output of massComponentWise.
 * @return    The output polynomial.
 */
template<typename RingElt>
Polynomial<RingElt> NegaNussbaumer<RingElt>::massInverseTransform(const MassTransformed& trans) {
  // Get the classes responsible for the deeper transformations
  std::vector<NegaNussbaumer<RingElt>> nbList;
  size_t N = r_;
  nbList.push_back(*this);
  while(1) {
    NegaNussbaumer<RingElt> next(N);
    nbList.push_back(next);
    if(N == 2)
      break;

    N = next.r_;
  }

  // Perform the deeper level transformations
  MassTransformed curMass = trans;
  for(auto iter = nbList.rbegin(); iter + 1 != nbList.rend(); ++iter) {
    MassTransformed nextMass;
    size_t nextTransSize = 2*(iter + 1)->m_;

    // Each transformed entry creates a polynomial that is input for the next inverse transform,
    // of which we want "nextTransSize" entries.
    Transformed curOutTrans;
    curOutTrans.push_back(Polynomial<RingElt>());  // The first entry is always 0
    for(auto curTrans : curMass) {
      curOutTrans.push_back(iter->inverseTransform(curTrans));
      if(curOutTrans.size() == nextTransSize) {
        nextMass.push_back(curOutTrans);
        curOutTrans.resize(1);
      }
    }

    curMass = nextMass;
  }

  return inverseTransform(curMass[0]);
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


/**
 * The function "rotates" the polynomial, as though multiplying it with u^steps,
 * modulo u^r + 1.
 * @param[in] pol    The polynomial to rotate.
 * @param[in] steps  The number of steps to rotate.
 * @return    The resulting polynomial.
 */
template<typename RingElt>
Polynomial<RingElt> NegaNussbaumer<RingElt>::rotatePolynomial(
                                              const Polynomial<RingElt>& pol,
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
      ret[i] = -pol[srcIndex];
    else
      ret[i] = pol[srcIndex];
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
