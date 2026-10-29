/*
 * FFT radix-2 iterativa, in-place, mesma estrutura do fft.py.
 *   - buffers separados re[] e im[] em float (FPU do Cortex-M4F/M7)
 *   - tabela de twiddles calculada uma vez em fft_init()
 *     (no MCU pode virar const em flash)
 *   - sem printf, sem malloc: pronto para ir para o microcontrolador
 */

#include <stdint.h>
#include <math.h>
#include "fft.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N   FFT_N

static float cos_tab[N / 2];
static float sin_tab[N / 2];

/* ------------------------------------------------------------
 * Tabela de twiddles: cos e sin de 2*pi*k/N
 * ------------------------------------------------------------ */
void fft_init(void)
{
    uint32_t k;

    for (k = 0; k < N / 2; k++) {
        cos_tab[k] = cosf(2.0f * (float)M_PI * k / N);
        sin_tab[k] = sinf(2.0f * (float)M_PI * k / N);
    }
}

/* ------------------------------------------------------------
 * FFT in-place: entra o sinal em re[]/im[], sai o espectro
 * ------------------------------------------------------------ */
void fft(float *re, float *im)
{
    uint32_t i, j, k, m, bit, size, half, step, start, a, b;
    float tmp, wr, wi, tr, ti;

    /* 1) Reordenacao bit-reversal (in-place) */
    j = 0;
    for (i = 1; i < N; i++) {
        bit = N >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j |= bit;
        if (i < j) {
            tmp = re[i]; re[i] = re[j]; re[j] = tmp;
            tmp = im[i]; im[i] = im[j]; im[j] = tmp;
        }
    }

    /* 2) Estagios de borboleta (log2(N) estagios) */
    for (size = 2; size <= N; size <<= 1) {
        half = size >> 1;
        step = N / size;                /* passo na tabela de twiddles */
        for (start = 0; start < N; start += size) {
            k = 0;
            for (m = 0; m < half; m++) {
                a = start + m;
                b = a + half;
                wr = cos_tab[k];
                wi = -sin_tab[k];       /* W = e^(-j*2*pi*k/N) */
                /* t = W * x[b] */
                tr = wr * re[b] - wi * im[b];
                ti = wr * im[b] + wi * re[b];
                /* borboleta */
                re[b] = re[a] - tr;
                im[b] = im[a] - ti;
                re[a] = re[a] + tr;
                im[a] = im[a] + ti;
                k += step;
            }
        }
    }
}

/* ------------------------------------------------------------
 * Magnitude (so metade do espectro, sinal real e simetrico)
 * mag[] precisa ter N/2 posicoes
 * ------------------------------------------------------------ */
void fft_magnitude(const float *re, const float *im, float *mag)
{
    uint32_t k;

    for (k = 0; k < N / 2; k++) {
        mag[k] = sqrtf(re[k] * re[k] + im[k] * im[k]) * 2.0f / N;
    }
}
