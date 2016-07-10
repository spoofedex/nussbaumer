/**
 * @file Poly.h
 * @author Gerben van der Lubbe
 *
 * File responsible for dealing with New Hope/this program polynomial compatibility.
 */

#ifndef COMPAT_POLY_H
#define COMPAT_POLY_H

#include "newhope/ref/params.h"
#include "Polynomial.h"
#include "IntWrapper.h"

/**
 * Structure for polynomials to take the place of polynomials in New Hope (it will never know!)
 */
typedef struct {
  /// New Hope form of storing polynomials.
  IntWrapper<std::uint16_t> v[PARAM_N];

  Polynomial<std::uint16_t> toPolynomial();
} poly;

#include "newhope/ref/poly.h"

void poly_create_random(poly* p);

#endif
