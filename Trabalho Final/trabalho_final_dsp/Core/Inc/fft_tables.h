/*
 * fft_tables.h
 *
 *  Created on: 15 de nov. de 2026
 *      Author: felip
 */

#ifndef INC_FFT_TABLES_H_
#define INC_FFT_TABLES_H_

#include <stdint.h>

#define FFT_TABLE_N 8192u
#define FFT_TABLE_FS 10000u
#define FFT_TABLE_Q 15u
#define FFT_FREQ_SCALE 1000u //Frequency resolution: millihertz

extern const int32_t cos_factors[FFT_TABLE_N / 2];
extern const int32_t sin_factors[FFT_TABLE_N / 2];
extern const uint32_t frequency_bins[FFT_TABLE_N / 2 + 1];

#endif /* INC_FFT_TABLES_H_ */
