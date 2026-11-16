from pathlib import Path
import wave
import numpy as np
import matplotlib.pyplot as plt
import fft
import math


def wav_to_samples(filename: str | Path) -> np.ndarray:

    with wave.open(str(filename), "rb") as wav:
        n_channels = wav.getnchannels()
        sample_width = wav.getsampwidth()  # bytes
        n_frames = wav.getnframes()

        raw = wav.readframes(n_frames)

    source_bits = sample_width * 8

    # Decode samples to signed int32
    if sample_width == 1:
        samples = np.frombuffer(raw, dtype=np.uint8).astype(np.int32)
        samples -= 128  # WAV 8-bit PCM is unsigned

    elif sample_width == 2:
        samples = np.frombuffer(raw, dtype="<i2").astype(np.int32)

    elif sample_width == 3:
        data = np.frombuffer(raw, dtype=np.uint8).reshape(-1, 3)

        samples = (
            data[:, 0].astype(np.int32)
            | (data[:, 1].astype(np.int32) << 8)
            | (data[:, 2].astype(np.int32) << 16)
        )

        # Sign extension from 24 to 32 bits
        samples = np.where(
            samples & 0x800000,
            samples - (1 << 24),
            samples,
        )

    elif sample_width == 4:
        samples = np.frombuffer(raw, dtype="<i4").astype(np.int32)

    else:
        raise ValueError(
            f"Unsupported PCM width: {source_bits} bits"
        )

    # Convert to mono if needed
    if n_channels > 1:
        samples = samples.reshape(-1, n_channels)
        samples = np.round(samples.mean(axis=1)).astype(np.int32)

    # Generic conversion to 12 bits
    if source_bits > 12:
        samples = samples >> (source_bits - 12)
    elif source_bits < 12:
        samples = samples << (12 - source_bits)

    samples += 2048
    return samples.astype(np.uint16)

def plot_samples(samples):
    plt.figure(figsize=(12, 4))
    plt.plot(samples)
    plt.title("12-bit Audio Samples")
    plt.xlabel("Sample Index")
    plt.ylabel("Amplitude")
    plt.grid(True)
    plt.tight_layout()
    plt.show(block=False)

def perform_fft(samples, sample_rate):
    samples = samples.astype(np.float64) - 2048
    fft_result = 2*np.fft.rfft(samples)/len(samples)
    frequencies = np.fft.rfftfreq(len(samples), d=1 / sample_rate)

    # Magnitude spectrum
    magnitude = np.abs(fft_result)

    # Plot
    plt.figure(figsize=(12, 5))
    plt.plot(frequencies, magnitude)
    plt.title("Frequency Spectrum")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.grid(True)
    plt.tight_layout()
    plt.show(block=False)

def custom_fft(samples, sample_rate):
    result = fft.fft(samples)
    result = np.asarray(result)
    frequencies = np.fft.rfftfreq(len(samples), d=1 / sample_rate)

    #ignora DC offset
    frequencies = frequencies[1:]
    result = result[1:]

    # Plot
    plt.figure(figsize=(12, 5))
    plt.plot(frequencies, result)
    plt.title("Frequency Spectrum")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.grid(True)
    plt.tight_layout()
    plt.show(block=False)

def perform_chunked_fft(chunks, sample_rate):
    max_freq = np.arange(len(chunks),dtype=np.float64)
    for i in range(len(chunks)):
        samples = chunks[i].astype(np.float64) - 2048
        fft_result = 2*np.fft.rfft(samples)/len(samples)
        frequencies = np.fft.rfftfreq(len(samples), d=1 / sample_rate)
        max_index = np.argmax(fft_result)
        if(np.abs(fft_result[max_index]) > 100):
            max_freq[i] = frequencies[max_index]
        else:
            max_freq[i] = 0
        
    # Plot
    plt.figure(figsize=(12, 5))
    plt.plot(max_freq)
    plt.title("Frequencies over Spectrum")
    plt.xlabel("Chunks")
    plt.ylabel("Frequency (Hz)")
    plt.grid(True)
    plt.tight_layout()
    plt.show(block=False)

def chunk_samples(samples: np.ndarray,
                  chunk_size: int = 8192) -> np.ndarray:

    samples = np.asarray(samples)

    if chunk_size <= 0:
        raise ValueError("chunk_size must be positive")

    remainder = len(samples) % chunk_size

    if remainder:
        padding = chunk_size - remainder
        samples = np.pad(samples, (0, padding))

    return samples.reshape(-1, chunk_size)

import numpy as np

def dump_to_header(data1, data2, filename="dump.h", array_name="buffer"):
    data1 = np.asarray(data1, dtype=np.uint16).flatten()
    data2 = np.asarray(data2, dtype=np.uint16).flatten()

    assert data1.size == 8192, "Each array must contain exactly 8192 elements"
    assert data2.size == 8192, "Each array must contain exactly 8192 elements"

    with open(filename, "w") as f:
        f.write("#ifndef DUMP_H\n#define DUMP_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"uint16_t {array_name}[2][8192] = {{\n")

        for data in (data1, data2):
            f.write("    {\n")
            np.savetxt(f, data.reshape(-1, 8), fmt="%d",
                       delimiter=", ", newline=",\n")
            f.write("    },\n")

        f.write("};\n\n#endif // DUMP_H\n")

def parse_display(frequency: float, line1: list, line2: list) -> bool:

    # Notes of octave 4 (A4 = 440 Hz)
    NOTES = (
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    )

    FREQS = (
        261.63, 277.18, 293.66, 311.13, 329.63, 349.23,
        369.99, 392.00, 415.30, 440.00, 466.16, 493.88
    )

    HALF_SEMITONE = 1.0293022366
    TOLERANCE_CENTS = 5.0

    # Precomputed note boundaries
    NOTE_UPPER = tuple(f * HALF_SEMITONE for f in FREQS)

    # Precomputed octave boundaries
    OCTAVE_LOWER = NOTE_UPPER[0]
    OCTAVE_UPPER = NOTE_UPPER[11]

    # 1. Normalize frequency to octave 4
    f = frequency
    octave = 4

    while f < OCTAVE_LOWER:
        f *= 2.0
        octave -= 1

    while f > OCTAVE_UPPER:
        f *= 0.5
        octave += 1

    # 2. Find corresponding note
    note = 0

    while note < 11 and f > NOTE_UPPER[note]:
        note += 1

    # 3. Calculate deviation in cents
    cents_float = 1200.0 * math.log2(f / FREQS[note])

    # Position within the note's frequency range
    pos = 0.5 + cents_float * 0.01

    # Rounded deviation in cents
    cents = math.floor(cents_float + 0.5)

    # 4. Generate first LCD line
    name = f"{NOTES[note]}{octave}"
    tuned = abs(cents_float) <= TOLERANCE_CENTS

    if tuned:
        text = f"{name:<3.3} {frequency:4.1f}Hz  OK"
    else:
        text = f"{name:<3.3} {frequency:4.1f}Hz {cents:+3d}"

    line1[0] = text[:16].ljust(16)

    # 5. Generate second LCD line
    line = list("b------|------# ")

    if tuned:
        column = 7
    else:
        column = 1 + int(pos * 12.0 + 0.5)

    line[column] = 'O'

    line2[0] = ''.join(line)

    return True

filename = "./samples/piano-c_C_major.wav"

with wave.open(filename, "rb") as wav:
    sample_rate = wav.getframerate()

samples = wav_to_samples(filename)

#plot_samples(samples)
chunks = chunk_samples(samples)

perform_fft(chunks[1],sample_rate)
custom_fft(chunks[1],sample_rate)

line1 = [""]
line2 = [""]
frequencies = [
    446, 123.5, 57, 368, 307.92, 149.44, 294.21, 2346.4, 
    504.67, 671.46, 621.7, 710.8, 805, 879.6, 930.04, 1003]
for frequency in frequencies:
    parse_display(frequency, line1, line2)
    print(line1[0])
    print(line2[0])

#dump_to_header(chunks[1],chunks[2])
#perform_fft(chunks[5],sample_rate)

input()