//
//  IntWrapper.h
//  Thesis
//
//  Created by Gerben on 2-03-16.
//  Copyright © 2016 Gerben van der Lubbe. All rights reserved.
//

#ifndef INTWRAPPER_H
#define INTWRAPPER_H

#include <iostream>
#include <cassert>

#include "Util.h"
#include "OpCount.h"

template<typename Type>
class IntWrapper : public CompEquality<IntWrapper<Type>>,
                   public Shifts<IntWrapper<Type>> {
public:
  IntWrapper();

  template<typename IntType>
  IntWrapper(const IntType& value);
  template<typename IntType>
  IntWrapper(const IntWrapper<IntType>& value);

  Type toInt() const;

  template<typename OtherType>
  const IntWrapper<Type>& operator+=(const OtherType& e);
  const IntWrapper<Type>& operator-=(const IntWrapper<Type>& e);
  const IntWrapper<Type>& operator*=(const IntWrapper<Type>& e);
  const IntWrapper<Type>& operator&=(const IntWrapper<Type>& e);
  const IntWrapper<Type>& operator%=(const IntWrapper<Type>& e);

  const IntWrapper<Type>& operator<<=(int p);
  const IntWrapper<Type>& operator>>=(int p);

  static const OpCount& getOpCount();
  static void setOpCount(const OpCount& opCount);

private:
  Type value_ = 0;
};

extern OpCount opCountIntWrapper;

/**
 * Print the value of an int wrapper to the output stream.
 * @param[in] out   The output stream to write to.
 * @param[in] e     The IntWrapper to display.
 * @return    A reference to the output stream.
 */
template<typename Type>
std::ostream& operator<<(std::ostream& out, const IntWrapper<Type>& e) {
  out << e.toInt();
  return out;
}

/**
 * Build an IntWrapper of the correct type.
 * @param[in] value   The value to wrap.
 * @return    The IntWrapper
 */
template<typename Type>
IntWrapper<Type> build_intwrapper(const Type& value) {
  return IntWrapper<Type>(value);
}


/**
 * Helper class for a function to convert both an integer and IntWrapper to
 * the correct integer type. This prevents duplication of code.
 */
template<typename Type>
struct ToInt {
  static Type get(const Type& val) { return val; }
};
template<typename WrappedType>
struct ToInt<IntWrapper<WrappedType>> {
  static WrappedType get(const IntWrapper<WrappedType>& val) { return val.toInt(); }
};

/**
 * This function allows either an integer or a wrapped integer, and returns the
 * corresponding integer value.
 * @param[in] val    The value to unwrap.
 * @return    The integer value.
 */
template<typename Type>
auto unwrap(const Type& val) {
  return ToInt<Type>::get(val);
}


/**
 * Gets the counter for number of operations on the ring. This value is
 * resulterent for resulterent types.
 * @return A reference to the OpCount class.
 */
template<typename Type>
const OpCount& IntWrapper<Type>::getOpCount() {
  return opCountIntWrapper;
}

/**
 * Update the operation counter to this value.
 * @param[in] opCount  The number of operations at this time.
 */
template<typename Type>
void IntWrapper<Type>::setOpCount(const OpCount& opCount) {
  opCountIntWrapper = opCount;
}


template<typename Type>
IntWrapper<Type>::IntWrapper()
: value_(Type())
{}


template<typename Type> template<typename IntType>
IntWrapper<Type>::IntWrapper(const IntType& value)
: value_(value)
{}


template<typename Type> template<typename IntType>
IntWrapper<Type>::IntWrapper(const IntWrapper<IntType>& value)
: value_(value.toInt())
{}



template<typename Type>
Type IntWrapper<Type>::toInt() const {
  return value_;
}


template<typename Type> template<typename OtherType>
const IntWrapper<Type>& IntWrapper<Type>::operator+=(
                                            const OtherType& e
                                                            ) {
  value_ += unwrap(e);
  opCountIntWrapper.countAddition();
  return *this;
}

template<typename Type1, typename Type2>
auto operator+(const IntWrapper<Type1>& v1, const IntWrapper<Type2>& v2) {
  auto result = v1.toInt() + v2.toInt();
  opCountIntWrapper.countAddition();
  return build_intwrapper(result);
}


template<typename Type1, typename Type2>
auto operator+(const IntWrapper<Type1>& v1, const Type2& v2) {
  auto result = v1.toInt() + unwrap(v2);
  opCountIntWrapper.countAddition();
  return build_intwrapper(result);
}

template<typename Type1, typename Type2>
auto operator+(const Type1& v1, const IntWrapper<Type2>& v2) {
  auto result = unwrap(v1) + v2.toInt();
  opCountIntWrapper.countAddition();
  return build_intwrapper(result);
}

template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator-=(
                                            const IntWrapper<Type>& e
                                                            ) {
  value_ -= e.value_;
  opCountIntWrapper.countAddition();
  return *this;
}

template<typename Type1, typename Type2>
auto operator-(const IntWrapper<Type1>& v1, const IntWrapper<Type2>& v2) {
  auto result = v1.toInt() - v2.toInt();
  opCountIntWrapper.countAddition();
  return build_intwrapper(result);
}

template<typename Type1, typename Type2>
auto operator-(const IntWrapper<Type1>& v1, const Type2& v2) {
  auto result = v1.toInt() - v2;
  opCountIntWrapper.countAddition();
  return build_intwrapper(result);
}

template<typename Type1, typename Type2>
auto operator-(const Type1& v1, const IntWrapper<Type2>& v2) {
  auto result = v1 - v2.toInt();
  opCountIntWrapper.countAddition();
  return build_intwrapper(result);
}

template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator*=(
                                            const IntWrapper<Type>& e
                                                            ) {
  value_ *= e.value_;
  opCountIntWrapper.countMultiplication();
  return *this;
}

template<typename Type1, typename Type2>
auto operator*(const IntWrapper<Type1>& v1, const IntWrapper<Type2>& v2) {
  auto result = v1.toInt() * v2.toInt();
  opCountIntWrapper.countMultiplication();
  return build_intwrapper(result);
}


template<typename Type1, typename Type2>
auto operator*(const IntWrapper<Type1>& v1, const Type2& v2) {
  auto result = v1.toInt() * v2;
  opCountIntWrapper.countMultiplication();
  return build_intwrapper(result);
}

template<typename Type1, typename Type2>
auto operator*(const Type1& v1, const IntWrapper<Type2>& v2) {
  auto result = v1 * v2.toInt();
  opCountIntWrapper.countMultiplication();
  return build_intwrapper(result);
}

template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator&=(const IntWrapper<Type>& e) {
  value_ &= e.toInt();
  opCountIntWrapper.countBitwise();
  return *this;
}


template<typename Type1, typename Type2>
auto operator&(const IntWrapper<Type1>& v1, const Type2& v2) {
  auto result = v1.toInt() & v2;
  opCountIntWrapper.countBitwise();
  return build_intwrapper(result);
}

template<typename Type1, typename Type2>
auto operator&(const IntWrapper<Type1>& v1, const IntWrapper<Type2>& v2) {
  return v1 & v2.toInt();
}

template<typename Type1, typename Type2>
auto operator^(const IntWrapper<Type1>& v1, const IntWrapper<Type2>& v2) {
  auto result = v1.toInt() ^ v2.toInt();
  opCountIntWrapper.countBitwise();
  return build_intwrapper(result);
}

template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator%=(const IntWrapper<Type>& e) {
  value_ %= unwrap(e);
  opCountIntWrapper.countDivision();
  return *this;
}

template<typename Type1, typename Type2>
auto operator%(const IntWrapper<Type1>& t1, const Type2& t2) {
  auto result = t1.toInt() % unwrap(t2);
  opCountIntWrapper.countDivision();
  return build_intwrapper(result);
}


template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator<<=(int p) {
  opCountIntWrapper.countShift();
  value_ <<= p;
  return *this;
}

template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator>>=(int p) {
  opCountIntWrapper.countShift();
  value_ >>= p;
  return *this;
}


template<typename Type, typename OtherType>
bool operator==(const IntWrapper<Type>& a, const IntWrapper<OtherType>& b) {
  // When optimized this won't need a reduction, as a and b are already
  // reduced. Just set for clarity.
  return a.toInt() == b.toInt();
}


#endif
