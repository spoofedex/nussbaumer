#ifndef COMPAT_POLY_H
#define COMPAT_POLY_H

#include "newhope/ref/params.h"
#include "Polynomial.h"
#include "IntWrapper.h"

typedef struct {
  IntWrapper<std::uint16_t> v[PARAM_N];

  Polynomial<std::uint16_t> toPolynomial();
} poly;

#include "newhope/ref/poly.h"

void poly_create_random(poly* p);

#endif
