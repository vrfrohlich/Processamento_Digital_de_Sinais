# FFT radix-2 iterativa, in-place, sem funcoes e sem tipo complex.
#   - buffers separados re[] e im[] (float32 ou q15/q31 no MCU)
#   - tabela de twiddles pre-calculada (no MCU vira const em flash)
#   - so lacos, somas, multiplicacoes e operacoes de bit

import math

FS = 1000.0         # frequencia de amostragem (Hz)

# data precisa ter len em potencia de 2
def fft(data):

    N = len(data)

    # ---------------------------------------------------------------
    # Tabela de twiddles: N/2 valores de cos e sin de 2*pi*k/N
    # No MCU: gere offline e grave como const float cos_tab[N/2], sin_tab[N/2]
    # ---------------------------------------------------------------
    cos_tab = [0.0] * (N // 2)
    sin_tab = [0.0] * (N // 2)
    for k in range(N // 2):
        cos_tab[k] = math.cos(2.0 * math.pi * k / N)
        sin_tab[k] = math.sin(2.0 * math.pi * k / N)

    re = data
    im = [0.0] * N

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
        half = size >> 1
        step = N // size                 # passo na tabela de twiddles
        start = 0
        while start < N:
            k = 0
            for m in range(half):
                a = start + m
                b = a + half
                wr = cos_tab[k]
                wi = -sin_tab[k]         # W = e^(-j*2*pi*k/N)
                # t = W * x[b]
                tr = wr * re[b] - wi * im[b]
                ti = wr * im[b] + wi * re[b]
                # borboleta
                re[b] = re[a] - tr
                im[b] = im[a] - ti
                re[a] = re[a] + tr
                im[a] = im[a] + ti
                k += step
            start += size
        size <<= 1

    # ---------------------------------------------------------------
    # 3) Magnitude (so metade do espectro, sinal real e simetrico)
    # ---------------------------------------------------------------
    mag = [0.0] * (N // 2)
    for k in range(N // 2):
        mag[k] = math.sqrt(re[k] * re[k] + im[k] * im[k]) * 2.0 / N

    #for k in range(N // 2):
    #    print("%3d  %7.2f Hz  %.4f" % (k, k * FS / N, mag[k]))

    return mag
