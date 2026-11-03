# FFT radix-2 iterativa, in-place, sem funcoes e sem tipo complex.
#   - buffers separados re[] e im[] (float32 ou q15/q31 no MCU)
#   - tabela de twiddles pre-calculada (no MCU vira const em flash)
#   - so lacos, somas, multiplicacoes e operacoes de bit

import math

Q = 15
SCALE  = 1 << Q

def divide_by_power_of_two(x, bits):
    """Symmetric rounding when dividing by a power of two."""
    if x >= 0:
        return (x + (1 << (bits - 1))) >> bits
    else:
        return -((-x + (1 << (bits - 1))) >> bits)

# data precisa ter len em potencia de 2
def fft(data):

    N = len(data)

    re = [int(x) for x in data]
    im = [0] * N

    # ---------------------------------------------------------------
    # Tabela de twiddles: N/2 valores de cos e sin de 2*pi*k/N
    # No MCU: gere offline e grave como const float cos_tab[N/2], sin_tab[N/2]
    # ---------------------------------------------------------------
    cos_tab = [0] * (N // 2)
    sin_tab = [0] * (N // 2)
    for k in range(N // 2):
        cos_tab[k] = round(math.cos(2 * math.pi * k / N) * SCALE)
        sin_tab[k] = round(math.sin(2 * math.pi * k / N)* SCALE)


    # ---------------------------------------------------------------
    # 1) Reordenacao bit-reversal (in-place)
    # ---------------------------------------------------------------
    j = 0
    for i in range(1, N):
        bit = N >> 1
        while j & bit:
            j ^= bit
            bit >>= 1
        j |= bit
        if i < j:
            tmp = re[i]; re[i] = re[j]; re[j] = tmp
            tmp = im[i]; im[i] = im[j]; im[j] = tmp

    # ---------------------------------------------------------------
    # 2) Estagios de borboleta (log2(N) estagios)
    # ---------------------------------------------------------------
    size = 2
    while size <= N:
        half = size // 2
        step = N // size                 # passo na tabela de twiddles
        for start in range(0, N, size):
            for m in range(half):
                a = start + m
                b = a + half
                k = m*step
                wr = cos_tab[k]
                wi = -sin_tab[k]         # W = e^(-j*2*pi*k/N)
                # t = W * x[b]
                tr = divide_by_power_of_two(wr * re[b] - wi * im[b],Q)
                ti = divide_by_power_of_two(wr * im[b] + wi * re[b],Q)
                # borboleta
                re[b] = divide_by_power_of_two(re[a] - tr,1)
                im[b] = divide_by_power_of_two(im[a] - ti,1)
                re[a] = divide_by_power_of_two(re[a] + tr,1)
                im[a] = divide_by_power_of_two(im[a] + ti,1)
        size <<= 1

    # ---------------------------------------------------------------
    # 3) Magnitude (so metade do espectro, sinal real e simetrico)
    # ---------------------------------------------------------------
    mag = [0] * (N // 2 + 1)
    for k in range(N // 2 + 1):
        mag[k] = math.isqrt(re[k] * re[k] + im[k] * im[k])
        if k != 0 and k != N // 2:
            mag[k] *= 2

    return mag