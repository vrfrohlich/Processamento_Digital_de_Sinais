import math

N = 8192
FS = 44100
Q = 15

SCALE = 1 << Q
FREQ_SCALE = 1000  # Frequency resolution: millihertz


def generate_factors():
    cos_factors = [
        round(math.cos(2 * math.pi * k / N) * SCALE)
        for k in range(N // 2)
    ]

    sin_factors = [
        round(math.sin(2 * math.pi * k / N) * SCALE)
        for k in range(N // 2)
    ]

    frequency_bins = [
        (k * FS * FREQ_SCALE + N // 2) // N
        for k in range(N // 2 + 1)
    ]

    # Generate header file
    with open("C/fft_tables.h", "w") as f:
        f.write("#ifndef FFT_TABLES_H\n")
        f.write("#define FFT_TABLES_H\n\n")
        f.write("#include <stdint.h>\n\n")

        f.write(f"#define FFT_TABLE_N {N}u\n")
        f.write(f"#define FFT_TABLE_FS {FS}u\n")
        f.write(f"#define FFT_TABLE_Q {Q}u\n")
        f.write(f"#define FFT_FREQ_SCALE {FREQ_SCALE}u\n\n")

        f.write("const int32_t cos_factors[FFT_TABLE_N / 2] = {\n")
        f.write(",\n".join(
            "    " + ", ".join(map(str, cos_factors[i:i+8]))
            for i in range(0, len(cos_factors), 8)
        ))
        f.write("\n};\n\n")

        f.write("const int32_t sin_factors[FFT_TABLE_N / 2] = {\n")
        f.write(",\n".join(
            "    " + ", ".join(map(str, sin_factors[i:i+8]))
            for i in range(0, len(sin_factors), 8)
        ))
        f.write("\n};\n\n")

        f.write("const uint32_t frequency_bins[FFT_TABLE_N / 2 + 1] = {\n")
        f.write(",\n".join(
            "    " + ", ".join(map(str, frequency_bins[i:i+8]))
            for i in range(0, len(frequency_bins), 8)
        ))
        f.write("\n};\n")

        f.write("#endif /* FFT_TABLES_H */\n")


generate_factors()
