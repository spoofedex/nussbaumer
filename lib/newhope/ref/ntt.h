#ifndef NTT_H
#define NTT_H

#include "inttypes.h"
#include "IntWrapper.h"

extern uint16_t omegas_montgomery[];
extern uint16_t omegas_inv_montgomery[];

extern uint16_t psis_bitrev_montgomery[];
extern uint16_t psis_inv_montgomery[];

void bitrev_vector(IntWrapper<uint16_t>* poly);
void mul_coefficients(IntWrapper<uint16_t>* poly, uint16_t* factors);
void ntt(IntWrapper<uint16_t>* poly, const uint16_t* omegas);

#endif
