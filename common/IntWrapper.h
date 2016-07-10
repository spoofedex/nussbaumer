/**
 * @file IntWrapper.h
 * @author Gerben van der Lubbe
 *
 * A wrapper for integers, in order to keep track of the number of operations performed on them.
 */

#ifndef INTWRAPPER_H
#define INTWRAPPER_H

#include <iostream>
#include <cassert>

#include "Util.h"
#include "OpCount.h"

/// Operation counter for all intwrappers.
extern OpCount opCountIntWrapper;

template<typename Type>
class IntWrapper;

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
 * Class that wraps around an integer type, to count operations.
 *
 * Note: IntWrappers with different type have the same counts. Also, not all operators are
 * implemented: this class has been implemented on a as-needed basis.
 */
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
  const IntWrapper<Type>& operator*=(const Type& e);
  const IntWrapper<Type>& operator&=(const IntWrapper<Type>& e);
  const IntWrapper<Type>& operator%=(const IntWrapper<Type>& e);

  const IntWrapper<Type>& operator<<=(int p);
  const IntWrapper<Type>& operator>>=(int p);

  friend auto operator+(const IntWrapper& v1, const IntWrapper& v2) {
    auto result = v1.toInt() + v2.toInt();
    opCountIntWrapper.countAddition();
    return build_intwrapper(result);
  }

  friend auto operator-(const IntWrapper& v1, const IntWrapper& v2) {
    auto result = v1.toInt() - v2.toInt();
    opCountIntWrapper.countAddition();
    return build_intwrapper(result);
  }

  friend auto operator&(const IntWrapper& v1, const IntWrapper& v2) {
    auto result = v1.toInt() & v2.toInt();
    opCountIntWrapper.countBitwise();
    return build_intwrapper(result);
  }

  static const OpCount& getOpCount();
  static void setOpCount(const OpCount& opCount);

private:
  Type value_ = 0;
};


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


/**
 * Construct an IntWrapper without a pre-set value.
 */
template<typename Type>
IntWrapper<Type>::IntWrapper()
: value_(Type())
{}


/**
 * Construct an IntWrapper with a constant value.
 * @param[in] value    The value to set.
 */
template<typename Type> template<typename IntType>
IntWrapper<Type>::IntWrapper(const IntType& value)
: value_(value)
{}


/**
 * Construct an IntWrapper with a value from another IntWrapper.
 * @param[in] value    The IntWrapper whose value is copied.
 */
template<typename Type> template<typename IntType>
IntWrapper<Type>::IntWrapper(const IntWrapper<IntType>& value)
: value_(value.toInt())
{}


/**
 * Get the raw integer value of the IntWrapper.
 * @return    The value associated with the wrapper.
 */
template<typename Type>
Type IntWrapper<Type>::toInt() const {
  return value_;
}


/**
 * Add a value, either constant or another IntWrapper, to this one.
 * @param[in] e        The value/IntWrapper to add.
 * @return    A reference to itself.
 */
template<typename Type> template<typename OtherType>
const IntWrapper<Type>& IntWrapper<Type>::operator+=(const OtherType& e) {
  value_ += unwrap(e);
  opCountIntWrapper.countAddition();
  return *this;
}


/**
 * Subtract another IntWrapper from this one.
 * @param[in] e        The value to subtract.
 * @return    A reference to itself.
 */
template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator-=(const IntWrapper<Type>& e) {
  value_ -= e.value_;
  opCountIntWrapper.countAddition();
  return *this;
}


/**
 * Multiply an IntWrapper by another one.
 * @param[in] e        The value to multiply by.
 * @return    A reference to itself.
 */
template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator*=(
                                            const IntWrapper<Type>& e
                                                            ) {
  value_ *= e.value_;
  opCountIntWrapper.countMultiplication();
  return *this;
}


/**
 * Multiply an IntWrapper with a constant value (counted differently than usual multiplications).
 * @param[in] e        The value to multiply by.
 * @return    A reference to itself.
 */
template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator*=(
                                            const Type& e
                                                            ) {
  value_ *= e.value_;
  opCountIntWrapper.countConstMult();
  return *this;
}


/**
 * Multiply two IntWrapper values.
 * @param[in] v1       The value to multiply.
 * @param[in] v2       The value to multiply with.
 * @return    The product of the two.
 */
template<typename Type1, typename Type2>
auto operator*(const IntWrapper<Type1>& v1, const IntWrapper<Type2>& v2) {
  auto result = v1.toInt() * v2.toInt();
  opCountIntWrapper.countMultiplication();
  return build_intwrapper(result);
}


/**
 * Multiply an IntWrapper with a constant.
 * @param[in] v1       The IntWrapper.
 * @param[in] v2       The constant.
 * @return    The new IntWrapper, containing the product of the two.
 */
template<typename Type1, typename Type2>
auto operator*(const IntWrapper<Type1>& v1, const Type2& v2) {
  auto result = v1.toInt() * v2;
  opCountIntWrapper.countConstMult();
  return build_intwrapper(result);
}


/**
 * Multiply a constant with an IntWrapper.
 * @param[in] v1       The constant.
 * @param[in] v2       The IntWrapper.
 * @return    The new IntWrapper, containing the product of the two.
 */
template<typename Type1, typename Type2>
auto operator*(const Type1& v1, const IntWrapper<Type2>& v2) {
  auto result = v1 * v2.toInt();
  opCountIntWrapper.countConstMult();
  return build_intwrapper(result);
}


/**
 * Bitwise and-assign operator.
 * @param[in] e        The value to calculate the bitwise and width.
 * @return    A reference to itself.
 */
template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator&=(const IntWrapper<Type>& e) {
  value_ &= e.toInt();
  opCountIntWrapper.countBitwise();
  return *this;
}


/**
 * Exclusive oroperator for IntWrappers.
 * @param[in] v1       The first value.
 * @param[in] v2       The second value.
 * @return    The exclusive or of the two value.
 */
template<typename Type1, typename Type2>
auto operator^(const IntWrapper<Type1>& v1, const IntWrapper<Type2>& v2) {
  auto result = v1.toInt() ^ v2.toInt();
  opCountIntWrapper.countBitwise();
  return build_intwrapper(result);
}


/**
 * Modulo assignment operator for IntWrapper.
 * @param[in] e        The modulus.
 * @return    A reference to itself.
 */
template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator%=(const IntWrapper<Type>& e) {
  value_ %= unwrap(e);
  opCountIntWrapper.countDivision();
  return *this;
}


/**
 * Modulo operator for IntWrapper.
 * @param[in] t1       The IntWrapper to calculate the modulo for.
 * @param[in] t2       The modulus.
 * @return    t1 reduced modulo t2.
 */
template<typename Type1, typename Type2>
auto operator%(const IntWrapper<Type1>& t1, const Type2& t2) {
  auto result = t1.toInt() % unwrap(t2);
  opCountIntWrapper.countDivision();
  return build_intwrapper(result);
}


/**
 * Bitwise left-shift assignment operator.
 * @param[in] p        Shift the IntWrapper left by p bits.
 * @return    A reference to itself.
 */
template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator<<=(int p) {
  opCountIntWrapper.countShift();
  value_ <<= p;
  return *this;
}


/**
 * Bitwise right-shift assignment operator.
 * @param[in] p        Shift the IntWrapper right by p bits.
 * @return    A reference to itself.
 */
template<typename Type>
const IntWrapper<Type>& IntWrapper<Type>::operator>>=(int p) {
  opCountIntWrapper.countShift();
  value_ >>= p;
  return *this;
}


/**
 * Compare two IntWrappers.
 * @param[in] a        The first IntWrapper.
 * @param[in] b        The second IntWrapper.
 * @return    true iff a equals b.
 */
template<typename Type, typename OtherType>
bool operator==(const IntWrapper<Type>& a, const IntWrapper<OtherType>& b) {
  return a.toInt() == b.toInt();
}


#endif
