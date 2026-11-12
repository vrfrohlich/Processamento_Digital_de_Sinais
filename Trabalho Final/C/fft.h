#ifndef FFT_H
#define FFT_H

#include <stdint.h>

/*
 * Computes the single-sided magnitude spectrum of a real-valued signal.
 *
 * input:     Input samples (unsigned ADC values).
 * magnitude: Output magnitude spectrum (N/2 + 1 elements).
 * N:         Number of input samples (must be a power of two).
 * re, im:    Working buffers, each containing N elements.
 *
 * Returns:
 *  false: Invalid arguments or FFT size.
 *  true: Operation executed succesfully 
 */
bool fft(
    const uint16_t *input,
    uint16_t *magnitude,
    uint32_t N,
    int16_t *re,
    int16_t *im,
    int32_t *peak_mag_index
);

#endif