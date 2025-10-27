#include "fft_common.h"
#include "fft_algorithms.h"

/**
 * @file radix2_dit.c
 * @brief Cooley-Tukey Radix-2 Decimation-in-Time (DIT) FFT Implementation
 * 
 * @details
 * This file implements the classic Cooley-Tukey FFT algorithm using the
 * Decimation-in-Time (DIT) approach. The algorithm recursively divides
 * the DFT computation into smaller DFTs of even and odd indexed samples.
 * 
 * Algorithm Overview:
 * 1. Bit-reversal permutation to reorder input
 * 2. Iterative computation using butterfly operations
 * 3. Each stage combines pairs of smaller DFTs into larger ones
 * 
 * Mathematical Background:
 * The DFT of length N can be split into two DFTs of length N/2:
 *   X[k] = DFT_even[k] + W_N^k * DFT_odd[k]  for k = 0 to N/2-1
 *   X[k+N/2] = DFT_even[k] - W_N^k * DFT_odd[k]
 * where W_N^k = exp(-2πik/N) is the twiddle factor
 * 
 * @author FFT Study Repository
 * @date 2024
 * 
 * Time Complexity: O(n log n)
 * Space Complexity: O(1) for in-place operation
 * 
 * References:
 * [1] Cooley, J. W., & Tukey, J. W. (1965). "An algorithm for the machine 
 *     calculation of complex Fourier series"
 * [2] Brigham, E. O. (1988). "The Fast Fourier Transform and Its Applications"
 */

/**
 * @brief Main Radix-2 DIT FFT implementation
 * 
 * @details
 * This function implements the iterative Cooley-Tukey FFT algorithm.
 * It uses an in-place computation approach with bit-reversal reordering
 * followed by log₂(n) stages of butterfly operations.
 * 
 * Data Structures:
 * - In-place array manipulation (no additional arrays needed)
 * - Bit-reversal permutation for initial reordering
 * - Complex number arithmetic for butterfly operations
 * 
 * Algorithm Steps:
 * 1. Validate input size is power of 2
 * 2. Perform bit-reversal permutation
 * 3. Execute log₂(n) stages of butterfly operations
 * 4. Scale output for inverse transform
 * 
 * @param x Input/output array of complex numbers
 * @param n Length of array (must be power of 2)
 * @param dir Transform direction (FFT_FORWARD or FFT_INVERSE)
 */
void radix2_dit_fft(complex_t* x, int n, fft_direction dir) {
    /* Validate input is power of 2 */
    CHECK_POWER_OF_TWO(n);
    
    int log2n = log2_int(n);
    
    /* 
     * Step 1: Bit-reversal permutation
     * Reorder array so that element at index i moves to bit_reverse(i)
     * This allows the iterative algorithm to work in-place
     */
    for (int i = 0; i < n; i++) {
        int j = bit_reverse(i, log2n);
        if (i < j) {  /* Swap only once */
            complex_t temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }
    
    /* 
     * Step 2: Danielson-Lanczos algorithm
     * Iteratively combine smaller DFTs into larger ones
     * Stage s combines DFTs of size 2^(s-1) into DFTs of size 2^s
     */
    for (int stage = 1; stage <= log2n; stage++) {
        int m = 1 << stage;        /* Current DFT size = 2^stage */
        int half_m = m >> 1;        /* Half of current DFT size */
        
        /* Principal root of unity for this stage */
        complex_t w_m = twiddle_factor(1, m, dir);
        
        /* Process all DFTs of size m */
        for (int k = 0; k < n; k += m) {
            complex_t w = 1.0;  /* Initialize twiddle factor */
            
            /* Butterfly operations within current DFT */
            for (int j = 0; j < half_m; j++) {
                int t = k + j;          /* Top butterfly index */
                int u = t + half_m;     /* Bottom butterfly index */
                
                /* Butterfly computation:
                 * X[t] = X[t] + w * X[u]
                 * X[u] = X[t] - w * X[u]
                 */
                complex_t temp = x[u] * w;
                x[u] = x[t] - temp;
                x[t] = x[t] + temp;
                
                /* Update twiddle factor for next butterfly */
                w *= w_m;
            }
        }
    }
    
    /* Step 3: Scale for inverse FFT */
    if (dir == FFT_INVERSE) {
        for (int i = 0; i < n; i++) {
            x[i] /= n;
        }
    }
}

/**
 * @brief Compute forward FFT using Radix-2 DIT
 * @param x Input/output array
 * @param n Array length (must be power of 2)
 */
void fft_radix2_dit(complex_t* x, int n) {
    radix2_dit_fft(x, n, FFT_FORWARD);
}

/**
 * @brief Compute inverse FFT using Radix-2 DIT
 * @param x Input/output array
 * @param n Array length (must be power of 2)
 */
void ifft_radix2_dit(complex_t* x, int n) {
    radix2_dit_fft(x, n, FFT_INVERSE);
}

#ifndef LIB_BUILD

/**
 * @brief Visualize the butterfly operation pattern
 * 
 * @param n FFT size
 */
static void visualize_butterfly_pattern(int n) {
    printf("\nButterfly Operation Pattern (n=%d):\n", n);
    printf("===================================\n");
    
    int log2n = log2_int(n);
    
    /* Show first few stages */
    for (int stage = 1; stage <= log2n && stage <= 3; stage++) {
        int m = 1 << stage;
        int half_m = m >> 1;
        
        printf("\nStage %d (DFT size = %d):\n", stage, m);
        
        /* Show a few butterflies */
        int shown = 0;
        for (int k = 0; k < n && shown < 4; k += m) {
            for (int j = 0; j < half_m && shown < 4; j++) {
                int t = k + j;
                int u = t + half_m;
                printf("  Butterfly: x[%d] <-> x[%d] with W_%d^%d\n", 
                       t, u, m, j);
                shown++;
            }
        }
        if (n > 8) printf("  ...\n");
    }
}

/**
 * @brief Analyze algorithm complexity
 * 
 * @param max_n Maximum FFT size to analyze
 */
static void analyze_complexity(int max_n) {
    printf("\n\nComplexity Analysis:\n");
    printf("===================\n");
    printf("N\tlog₂(N)\tButterflies\tComplex Muls\n");
    printf("----\t-------\t-----------\t------------\n");
    
    for (int n = 4; n <= max_n; n *= 2) {
        int log2n = log2_int(n);
        int butterflies = (n / 2) * log2n;
        int muls = butterflies;  /* One complex multiplication per butterfly */
        
        printf("%d\t%d\t%d\t\t%d\n", n, log2n, butterflies, muls);
    }
    
    printf("\nFormula: (n/2) × log₂(n) complex multiplications\n");
}

#endif /* LIB_BUILD */
