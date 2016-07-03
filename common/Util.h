//
//  Util.h
//  Thesis
//
//  Created by Gerben on 13-02-16.
//  Copyright © 2016 Gerben van der Lubbe. All rights reserved.
//

#ifndef UTIL_H
#define UTIL_H

template<typename T1, typename T2 = T1>
class Multiplies {
  friend T1 operator*(const T1& a, const T2& b) { return T1(a) *= b; }
  friend T1 operator*(const T2& b, const T1& a) { return T1(a) *= b; }
};

template<typename T>
class Multiplies<T, T> {
  friend T operator*(const T& a, const T& b) { return T(a) *= b; }
};

template<typename T>
class Adds {
  friend T operator+(const T& a, const T& b) { return T(a) += b; }
};

template<typename T>
class Subtracts {
  friend T operator-(const T& a, const T& b) { return T(a) -= b; }
};

template<typename T>
class Shifts {
  friend T operator<<(const T& a, int p) { return T(a) <<= p; }
  friend T operator>>(const T& a, int p) { return T(a) >>= p; }
};

template<typename T>
class CompEquality {
  template<typename OtherT>
  friend bool operator!=(const T& a, const OtherT& b) { return !(a == b); }
};

#endif
