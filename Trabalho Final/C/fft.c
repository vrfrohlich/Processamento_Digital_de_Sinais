#include <stdint.h>
#include <stddef.h>

#include "fft.h"
#include "fft_tables.h"

/* ---------------------------------------------------------------
 * Symmetric rounding when dividing by a power of two.
 *
 * Examples:
 *  5 / 2 ->  3
 * -5 / 2 -> -3
 * --------------------------------------------------------------- */
static int32_t divide_by_power_of_two(int32_t x, uint32_t bits)
{
    if (bits == 0)
        return x;

    uint32_t abs_x = (x >= 0) ? (uint32_t)x : (uint32_t)(-(x + 1)) + 1u;
    uint32_t half = 1u << (bits - 1);

    uint32_t result = (abs_x + half) >> bits;

    return (x >= 0) ? (int32_t)result : -(int32_t)result;
}

static uint32_t integer_sqrt(uint32_t x)
{
    uint32_t root = 0;
    uint32_t bit = 1u << 30;

    while (bit > x)
        bit >>= 2;

    while (bit != 0)
    {
        if (x >= root + bit)
        {
            x -= root + bit;
            root = (root >> 1) + bit;
        }
        else
        {
            root >>= 1;
        }

        bit >>= 2;
    }

    return root;
}


/* ---------------------------------------------------------------
 * Radix-2 DIT FFT.
 * --------------------------------------------------------------- */
bool fft(
    const uint16_t *input,
    uint16_t *magnitude,
    uint32_t N,
    int16_t *re,
    int16_t *im,
    int32_t *peak_mag_index
)
{
    uint32_t i, j, k;
    uint32_t size, half, step;
    uint32_t start, m;
    uint32_t table_step;

    int16_t tmp;
    int32_t wr, wi;

    int32_t tr, ti;

    // Input validation
    if (input == NULL || magnitude == NULL || re == NULL || im == NULL)
        return false;

    if (N == 0 || (N & (N - 1)) != 0 || N > FFT_TABLE_N)
        return false;


    // Initialize real and imaginary arrays
    for (i = 0; i < N; i++)
    {
        re[i] = (int16_t)input[i];
        im[i] = 0;
    }


    // Bit-reversal permutation
    j = 0;

    for (i = 1; i < N; i++)
    {
        uint32_t bit = N >> 1;

        while (j & bit)
        {
            j ^= bit;
            bit >>= 1;
        }

        j |= bit;

        if (i < j)
        {
            tmp = re[i];
            re[i] = re[j];
            re[j] = tmp;

            tmp = im[i];
            im[i] = im[j];
            im[j] = tmp;
        }
    }


    /* -----------------------------------------------------------
     * 3) Butterfly stages
     * ----------------------------------------------------------- */

    /*
     * Allows FFT sizes smaller than FFT_TABLE_N.
     * Both sizes must be powers of two.
     */
    table_step = FFT_TABLE_N / N;

    size = 2;

    while (size <= N)
    {
        half = size / 2;

        /*
         * Step through the twiddle table for this stage.
         */
        step = FFT_TABLE_N / size;

        for (start = 0; start < N; start += size)
        {
            for (m = 0; m < half; m++)
            {
                i = start + m;
                j = i + half;

                k = m * step;

                /*
                 * Twiddle factor:
                 * W = cos(2*pi*k/FFT_TABLE_N)
                 *   - j*sin(2*pi*k/FFT_TABLE_N)
                 *
                 * The table index is adjusted for N < FFT_TABLE_N.
                 */
                wr = cos_factors[k * table_step];
                wi = -sin_factors[k * table_step];


                /* Complex multiplication: t = W * x[j] */

                tr = divide_by_power_of_two(
                    wr * re[j] - wi * im[j],
                    FFT_TABLE_Q
                );

                ti = divide_by_power_of_two(
                    wr * im[j] + wi * re[j],
                    FFT_TABLE_Q
                );


                /* Butterfly with division by 2 */

                re[j] = (int16_t)divide_by_power_of_two(
                    re[i] - tr, 1
                );

                im[j] = (int16_t)divide_by_power_of_two(
                    im[i] - ti, 1
                );

                re[i] = (int16_t)divide_by_power_of_two(
                    re[i] + tr, 1
                );

                im[i] = (int16_t)divide_by_power_of_two(
                    im[i] + ti, 1
                );
            }
        }

        size <<= 1;
    }

    uint16_t peak_mag = 0;
    /* -----------------------------------------------------------
     * 4) Single-sided magnitude spectrum
     * ----------------------------------------------------------- */
    for (k = 0; k <= N / 2; k++)
    {
        uint32_t re_squared = (int32_t)re[k] * re[k];
        uint32_t im_squared = (int32_t)im[k] * im[k];

        uint32_t sum = re_squared + im_squared;

        magnitude[k] = (uint16_t)integer_sqrt(sum);
        if(magnitude[k] > peak_mag && k != 0){
            peak_mag = magnitude[k];
            peak_mag_index = k;
        }

        /*
         * Double only the interior frequency bins.
         * DC and Nyquist must not be doubled.
         */
        if (k != 0 && k != N / 2)
        {
            magnitude[k] *= 2u;
        }
    }

    return true;
}