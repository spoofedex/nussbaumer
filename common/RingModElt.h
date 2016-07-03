//
//  RingModElt.h
//  Thesis
//
//  Created by Gerben on 11-02-16.
//  Copyright © 2016 Gerben van der Lubbe. All rights reserved.
//

#ifndef RINGMODELT_H
#define RINGMODELT_H

#include <iostream>
#include <cassert>

#include "Util.h"
#include "OpCount.h"

template<int Modulus>
class RingModElt : public Multiplies<RingModElt<Modulus>>,
                   public Adds<RingModElt<Modulus>>,
                   public Subtracts<RingModElt<Modulus>>,
                   public CompEquality<RingModElt<Modulus>> {
public:
  RingModElt(int value = 0);

  int toInt() const;

  const RingModElt<Modulus>& operator+=(const RingModElt<Modulus>& e);
  const RingModElt<Modulus>& operator-=(const RingModElt<Modulus>& e);
  const RingModElt<Modulus>& operator*=(const RingModElt<Modulus>& e);

  const RingModElt<Modulus> operator-() const;

  static bool getInverse(RingModElt<Modulus>& inverse,
                         const RingModElt<Modulus>& value);

  static OpCount& getOpCount();
  static void setOpCount(const OpCount& opCount);

private:
  int value_ = 0;
  static OpCount opCount_;
};


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


template<int Modulus>
const RingModElt<Modulus>& RingModElt<Modulus>::operator+=(
                                            const RingModElt<Modulus>& e
                                                            ) {
  value_ = (value_ + e.value_) % Modulus;
  opCount_.countAddition();
  return *this;
}


template<int Modulus>
const RingModElt<Modulus>& RingModElt<Modulus>::operator-=(
                                            const RingModElt<Modulus>& e
                                                            ) {
  value_ = (value_ - e.value_) % Modulus;
  opCount_.countAddition();
  return *this;
}


template<int Modulus>
const RingModElt<Modulus>& RingModElt<Modulus>::operator*=(
                                            const RingModElt<Modulus>& e
                                                            ) {
  value_ = (value_ * e.value_) % Modulus;
  opCount_.countMultiplication();
  return *this;
}

template<int Modulus>
const RingModElt<Modulus> RingModElt<Modulus>::operator-() const {
  RingModElt<Modulus> ret(RingModElt<Modulus>() - *this);
  return ret;
}

template<int Modulus>
bool operator==(const RingModElt<Modulus>& a, const RingModElt<Modulus>& b) {
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
