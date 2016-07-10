/**
 * @file Polynomial.h
 * @author Gerben van der Lubbe
 *
 * File to deal with polynomials. These polynomials are basically a vector: they have a size, and no real "degree"
 * we care about. Just a few more operators are provided that are logical in the case of polynomials.
 */

#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <iostream>
#include <vector>
#include <type_traits>
#include <initializer_list>

#include "Util.h"

/**
 * Class to store a polynomial with coefficients in RingElt.
 */
template<typename RingElt>
class Polynomial : public Multiplies<Polynomial<RingElt>, RingElt>,
                   public Adds<Polynomial<RingElt>>,
                   public Subtracts<Polynomial<RingElt>>,
                   public CompEquality<Polynomial<RingElt>> {
public:
  Polynomial(std::size_t size = 0);
  Polynomial(const std::initializer_list<RingElt>& elts);

  template<typename OtherType>
  Polynomial(const Polynomial<OtherType>& other);

  void setSize(std::size_t size);
  std::size_t getSize() const;

  const RingElt& operator[](std::size_t index) const;
  RingElt& operator[](std::size_t index);

  template<typename OtherT>
  const Polynomial<RingElt>& operator*=(const OtherT& scalar);

  template<typename OtherRingElt>
  const Polynomial<RingElt>& operator*=(const Polynomial<OtherRingElt>& other);
  const Polynomial<RingElt>& operator+=(const Polynomial<RingElt>& other);
  const Polynomial<RingElt>& operator-=(const Polynomial<RingElt>& other);

  Polynomial<RingElt> operator-() const;

private:
  std::vector<RingElt> coefs_;
};


/**
 * Create a polynomial with the specified size. All coefficients will be
 * initialized with 0.
 * @param[in] size  The size of the polynomial.
 */
template<typename RingElt>
Polynomial<RingElt>::Polynomial(std::size_t size)
: coefs_(size, RingElt())
{}

/**
 * Initialize a polynomial with the specified coefficients. The degree will
 * be set automatically. An empty initializer_list will initialize a polynomial
 * with 1 coefficient, with value 0.
 */
template<typename RingElt>
Polynomial<RingElt>::Polynomial(const std::initializer_list<RingElt>& elts)
: coefs_(elts)
{}


/**
 * Copy the contents of one polynomial to another.
 * @param[in] other    The polynomial to copy.
 */
template<typename RingElt> template<typename OtherType>
Polynomial<RingElt>::Polynomial(const Polynomial<OtherType>& other) {
  coefs_.resize(other.getSize());
  for(std::size_t i = 0; i < other.getSize(); ++i)
    coefs_[i] = other[i];
}

/**
 * Change the size of the polynomial; coefficients may be removed or added.
 * If they are added, they are initialized to 0.
 * @param[in] size   The new size to set.
 */
template<typename RingElt>
void Polynomial<RingElt>::setSize(std::size_t size) {
  coefs_.resize(size, RingElt());
}


/**
 * Get the size of the polynomial.
 * @return The size.
 */
template<typename RingElt>
std::size_t Polynomial<RingElt>::getSize() const {
  return coefs_.size();
}


/**
 * Get the specified coefficient to retrieve.
 * @param[in] index   The index of the coefficient (0 <= index < getDegree())
 * @return    The coefficient.
 */
template<typename RingElt>
const RingElt& Polynomial<RingElt>::operator[](std::size_t index) const {
  return coefs_.at(index);
}


/**
 * Get the specified coefficient to retrieve.
 * @param[in] index   The index of the coefficient (0 <= index < getDegree())
 * @return    The coefficient.
 */
template<typename RingElt>
RingElt& Polynomial<RingElt>::operator[](std::size_t index) {
  return coefs_.at(index);
}


/**
 * Compares two polynomials for equality. Their size must be the same for them
 * to be equal.
 * @param[in] a      The first polynomial.
 * @param[in] b      The second polynomial.
 * @return    true if the two are equal, false if not.
 */
template<typename RingElt, typename OtherRingElt>
bool operator==(const Polynomial<RingElt>& a, const Polynomial<OtherRingElt>& b) {
  if(a.getSize() != b.getSize())
    return false;

  for(std::size_t i = 0; i < a.getSize(); ++i)
    if(a[i] != b[i])
      return false;
  return true;
}


/**
 * Multiply a polynomial by a scalar.
 * @param[in] scalar      The scalar to multiply with.
 * @return    A reference to the polynomial, for operator chaining.
 */
template<typename RingElt> template<typename OtherElt>
const Polynomial<RingElt>& Polynomial<RingElt>::operator*=(
                                              const OtherElt& scalar
                                                          ) {
  for(std::size_t i = 0; i < getSize(); ++i)
    coefs_[i] *= scalar;
  return *this;
}


/**
 * Calculates the multiplication of two polynomials, using the classical algorithm.
 * @param[in] p1       The first polynomial.
 * @param[in] p2       The second polynomial.
 * @return    The product of the two polynomials (its size set to fit this)
 */
template<typename RingElt1, typename RingElt2>
auto operator*(const Polynomial<RingElt1>& p1, const Polynomial<RingElt2>& p2) {
  typedef decltype(p1[0]*p2[0]) RetType;
  Polynomial<RetType> ret;

  // Copy p1 into ret
  ret.setSize(p1.getSize());
  for(std::size_t i = 0; i < p1.getSize(); ++i)
    ret[i] = p1[i];

  // Multiply ret
  ret *= p2;
  return ret;
}


/**
 * Multiply this polynomial with another one.
 * @param[in] other   The polynomial to multiply with.
 * @return    A reference to the result.
 */
template<typename RingElt> template<typename OtherRingElt>
const Polynomial<RingElt>& Polynomial<RingElt>::operator*=(
                                              const Polynomial<OtherRingElt>& other
                                                          ) {
  Polynomial<RingElt> old(*this);

  // Create a new polynomial and calculate the result
  *this = Polynomial<RingElt>(old.getSize() + other.getSize() - 1);
  for(std::size_t i = 0; i < old.getSize(); ++i)
    for(std::size_t j = 0; j < other.getSize(); ++j)
      coefs_[i + j] += old[i]*other[j];
  return *this;
}


/**
 * Adds another polynomial to this one. The resulting size is the maximum
 * of the two polynomials.
 * @param[in] other   The polynomial to add to this one.
 * @return    A reference to this polynomial.
 */
template<typename RingElt>
const Polynomial<RingElt>& Polynomial<RingElt>::operator+=(
                                              const Polynomial<RingElt>& other
                                                          ) {
  if(other.getSize() > getSize())
    setSize(other.getSize());

  std::size_t minSize = std::min(getSize(), other.getSize());
  for(std::size_t i = 0; i < minSize; ++i)
    coefs_[i] += other[i];
  return *this;
}


/**
 * Subtracts another polynomial from this one. The resulting degree is the
 * maximum of the two polynomials.
 * @param[in] other   The polynomial to subtract from this one.
 * @return    A reference to this polynomial.
 */
template<typename RingElt>
const Polynomial<RingElt>& Polynomial<RingElt>::operator-=(
                                              const Polynomial<RingElt>& other
                                                          ) {
  if(other.getSize() > getSize())
    setSize(other.getSize());

  std::size_t minSize = std::min(getSize(), other.getSize());
  for(std::size_t i = 0; i < minSize; ++i)
    coefs_[i] -= other[i];
  return *this;
}


/**
 * Unary sign inversion operator.
 * @return   The sign inversed value.
 */
template<typename RingElt>
Polynomial<RingElt> Polynomial<RingElt>::operator-() const {
  Polynomial<RingElt> ret(*this);
  for(std::size_t i = 0; i < ret.getSize(); ++i)
    ret[i] = -ret[i];
  return ret;
}


/**
 * Print the polynomial onto the output stream.
 * @param[in] out     The output stream.
 * @param[in] p       The polynomial to print.
 * @return    A reference to the output stream, for chaining.
 */
template<typename RingElt>
std::ostream& operator<<(std::ostream& out, const Polynomial<RingElt>& p) {
  std::size_t size = p.getSize();
  for(std::size_t i = 0; i < size; ++i) {
    if(i) out << " + ";
    out << p[i] << "*X^" << i;
  }
  return out;
}


#endif
