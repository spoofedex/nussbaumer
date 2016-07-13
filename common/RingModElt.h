/**
 * @file RingModElt.h
 * @author Gerben van der Lubbe
 *
 * File for storing elements in the ring Z/qZ.
 */

#ifndef RINGMODELT_H
#define RINGMODELT_H

#include <iostream>
#include <cassert>

#include "Util.h"
#include "OpCount.h"

/**
 * Class to deal with the ring Z/qZ, where q == Modulus.
 */
template<int Modulus>
class RingModElt : public Multiplies<RingModElt<Modulus>>,
                   public Multiplies<RingModElt<Modulus>, int>,
                   public Adds<RingModElt<Modulus>>,
                   public Subtracts<RingModElt<Modulus>>,
                   public CompEquality<RingModElt<Modulus>> {
public:
  RingModElt(int value = 0);

  int toInt() const;

  const RingModElt<Modulus>& operator+=(const RingModElt<Modulus>& e);
  const RingModElt<Modulus>& operator-=(const RingModElt<Modulus>& e);
  const RingModElt<Modulus>& operator*=(const RingModElt<Modulus>& e);
  const RingModElt<Modulus>& operator*=(const int& e);

  const RingModElt<Modulus> operator-() const;

  static bool getInverse(RingModElt<Modulus>& inverse,
                         const RingModElt<Modulus>& value);

  static OpCount& getOpCount();
  static void setOpCount(const OpCount& opCount);

private:
  int value_ = 0;
  static OpCount opCount_;
};


/**
 * Write a RingModElt to a stream.
 * @param[in] out      The stream to write to.
 * @param[in] e        The RingModElt to write.
 * @return    A reference to the stream.
 */
template<int Modulus>
std::ostream& operator<<(std::ostream& out, const RingModElt<Modulus>& e) {
  out << e.toInt();
  return out;
}


/**
 * Keeps track of the number of operations performed on the ring.
 */
template<int Modulus>
OpCount RingModElt<Modulus>::opCount_;


/**
 * Gets the counter for number of operations on the ring. This value is
 * different for different Modulus.
 * @return A reference to the OpCount class.
 */
template<int Modulus>
OpCount& RingModElt<Modulus>::getOpCount() {
  return opCount_;
}


/**
 * Update the operation counter to this value.
 * @param[in] opCount  The number of operations at this time.
 */
template<int Modulus>
void RingModElt<Modulus>::setOpCount(const OpCount& opCount) {
  opCount_ = opCount;
}


/**
 * Create a ring element modulo the Modulus, with a specified integer value.
 * @param[in] value   The initial value to set (must be in the ring).
 */
template<int Modulus>
RingModElt<Modulus>::RingModElt(int value)
: value_(value)
{}


/**
 * Convert the element of the used ring (Z/mZ where m = Modulus) to the integer
 * value.
 * @return The integer value.
 */
template<int Modulus>
int RingModElt<Modulus>::toInt() const {
  return value_;
}


/**
 * Addition assignment operator for RingModElt.
 * @param[in] e        The value to add.
 * @return    A reference to self.
 */
template<int Modulus>
const RingModElt<Modulus>& RingModElt<Modulus>::operator+=(
                                            const RingModElt<Modulus>& e
                                                            ) {
  value_ = (value_ + e.value_) % Modulus;
  opCount_.countAddition();
  return *this;
}


/**
 * Subtract-assignment from the RingModElt.
 * @param[in] e        The value to subtract.
 * @return    A reference to self.
 */
template<int Modulus>
const RingModElt<Modulus>& RingModElt<Modulus>::operator-=(
                                            const RingModElt<Modulus>& e
                                                            ) {
  value_ = (value_ - e.value_) % Modulus;
  opCount_.countAddition();
  return *this;
}


/**
 * Multiplication-assignment operator by another RingModElt.
 * @param[in] e        The RingModElt value to multiply with.
 * @return    A reference to self.
 */
template<int Modulus>
const RingModElt<Modulus>& RingModElt<Modulus>::operator*=(
                                            const RingModElt<Modulus>& e
                                                            ) {
  value_ = (value_ * e.value_) % Modulus;
  opCount_.countMultiplication();
  return *this;
}


/**
 * Multiplication-assignment operator by a constant.
 * @param[in] e        The constant value to multiply with.
 * @return    A reference to self.
 */
template<int Modulus>
const RingModElt<Modulus>& RingModElt<Modulus>::operator*=(
                                                      const int& e
                                                            ) {
  value_ = (value_ * e) % Modulus;
  opCount_.countConstMult();
  return *this;
}


/**
 * Sign inversion operator for a RingModElt (counted as an addition).
 * @return   The value, sign-inverted.
 */
template<int Modulus>
const RingModElt<Modulus> RingModElt<Modulus>::operator-() const {
  RingModElt<Modulus> ret(RingModElt<Modulus>() - *this);
  return ret;
}


/**
 * Compare two RingModElts for equality.
 * @param[in] a        The first value to test.
 * @param[in] b        The second value to test.
 * @return    true iff the two are equal (using the modulus).
 */
template<int Modulus>
bool operator==(const RingModElt<Modulus>& a, const RingModElt<Modulus>& b) {
  // Modulo is still needed, as an integer may be negative or positive.
  return (a.toInt() - b.toInt()) % Modulus == 0;
}



template<int Modulus>
bool RingModElt<Modulus>::getInverse(RingModElt<Modulus>& inverse,
                                     const RingModElt<Modulus>& value) {
  // Use the extended Euclidian algorithm to get the inverse.
  int t = 0;
  int tNew = 1;
  int r = Modulus;
  int rNew = value.toInt();
  while(rNew != 0) {
    int q = r / rNew;
    int tmp;

    tmp = t - q*tNew;
    t = tNew;
    tNew = tmp;

    tmp = r - q*rNew;
    r = rNew;
    rNew = tmp;
  }

  if(r > 1)
    return false;

  inverse = RingModElt<Modulus>(t);
  assert((t*value.toInt() - 1) % Modulus == 0);
  return true;
}

#endif
