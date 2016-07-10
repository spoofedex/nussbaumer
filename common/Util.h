/**
 * @file Util.h
 * @author Gerben van der Lubbe
 *
 * Some useful template-inheritance classes.
 */

#ifndef UTIL_H
#define UTIL_H


/**
 * Base class for classes that multiply two types.
 */
template<typename T1, typename T2 = T1>
class Multiplies {
  friend T1 operator*(const T1& a, const T2& b) { return T1(a) *= b; }
  friend T1 operator*(const T2& b, const T1& a) { return T1(a) *= b; }
};

template<typename T>
class Multiplies<T, T> {
  friend T operator*(const T& a, const T& b) { return T(a) *= b; }
};


/**
 * Base class for classes using additions.
 */
template<typename T>
class Adds {
  friend T operator+(const T& a, const T& b) { return T(a) += b; }
};


/**
 * Base class for classes using subtractions.
 */
template<typename T>
class Subtracts {
  friend T operator-(const T& a, const T& b) { return T(a) -= b; }
};


/**
 * Base class for shiftable classes
 */
template<typename T>
class Shifts {
  friend T operator<<(const T& a, int p) { return T(a) <<= p; }
  friend T operator>>(const T& a, int p) { return T(a) >>= p; }
};


/**
 * Base class for classes comparing for (in)equality.
 */
template<typename T>
class CompEquality {
  template<typename OtherT>
  friend bool operator!=(const T& a, const OtherT& b) { return !(a == b); }
};

#endif
