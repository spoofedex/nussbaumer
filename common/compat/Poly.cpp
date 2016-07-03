#include <iostream>
#include <fstream>

#include "compat/Poly.h"
#include "newhope/ref/poly.h"


Polynomial<std::uint16_t> poly::toPolynomial() {
  Polynomial<std::uint16_t> ret(PARAM_N);
  for(std::size_t i = 0; i < PARAM_N; ++i)
    ret[i] = v[i].toInt();
  return ret;
}

void poly_create_random(poly* p) {
  static std::ifstream urandom("/dev/urandom", std::ios_base::binary);
  if(!urandom) {
    std::cerr << "Failed to open /dev/urandom" << std::endl;
    exit(1);
  }

  unsigned char seed[32];
  if(!urandom.read(reinterpret_cast<char*>(seed), sizeof(seed))) {
    std::cerr << "Failed to read random bytes" << std::endl;
    exit(1);
  }

  poly_uniform(p, seed);
}
