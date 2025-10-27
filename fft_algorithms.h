#ifndef FFT_ALGORITHMS_H
#define FFT_ALGORITHMS_H

#include "fft_common.h"

/* 
 * Function declarations for all FFT algorithms
 * This allows algorithms to use each other without circular dependencies
 */

/* Core FFT algorithms */
void radix2_dit_fft(complex_t* x, int n, fft_direction dir);

/* Convenience wrappers */
void fft_radix2_dit(complex_t* x, int n);
void ifft_radix2_dit(complex_t* x, int n);

#endif /* FFT_ALGORITHMS_H */
