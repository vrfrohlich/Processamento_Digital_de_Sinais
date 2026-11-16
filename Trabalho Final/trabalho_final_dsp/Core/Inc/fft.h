/*
 * fft.h
 *
 *  Created on: 15 de nov. de 2026
 *      Author: felip
 */

#ifndef INC_FFT_H_
#define INC_FFT_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * Computes the single-sided magnitude spectrum of a real-valued signal.
 *
 * input:     Input samples (unsigned ADC values) containing N elements.
 * magnitude: Output magnitude spectrum (N/2 + 1 elements).
 * N:         Number of input samples (must be a power of two).
 * re, im:    Working buffers, each containing N elements.
 *
 * Returns:
 *  false: Invalid arguments or FFT size.
 *  true: Operation executed successfully
 */
bool fft(
    const uint32_t *input,
    uint16_t *magnitude,
    uint32_t N,
    int16_t *re,
    int16_t *im,
    uint32_t *peak_mag_index
);


#endif /* INC_FFT_H_ */
