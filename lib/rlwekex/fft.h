/* This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * See LICENSE for complete information.
 */

#ifndef _FFT_H_
#define _FFT_H_

#include <stdint.h>

#include "RingModElt.h"

typedef RingModElt<2047> FftRingType;

struct fft_ctx {
	FftRingType **x1;
	FftRingType **y1;
	FftRingType **z1;
	FftRingType *t1;
};
typedef struct fft_ctx FFT_CTX;

int FFT_CTX_init(FFT_CTX *ctx);
void FFT_CTX_free(FFT_CTX *ctx);

void FFT_mul(FftRingType *z, const FftRingType *x, const FftRingType *y, FFT_CTX *ctx);
void FFT_add(FftRingType *z, const FftRingType *x, const FftRingType *y);

#endif /* _FFT_H_ */
