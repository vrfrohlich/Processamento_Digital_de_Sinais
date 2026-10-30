from pathlib import Path
import wave
import numpy as np
import matplotlib.pyplot as plt
import fft


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
    frequencies = frequencies[1:]

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

filename = "./samples/piano-c_C_major.wav"

with wave.open(filename, "rb") as wav:
    sample_rate = wav.getframerate()

samples = wav_to_samples(filename)

#plot_samples(samples)
chunks = chunk_samples(samples)

perform_fft(chunks[1],sample_rate)

custom_fft(chunks[1],sample_rate)
#perform_chunked_fft(chunks,sample_rate)

#perform_fft(chunks[5],sample_rate)


# Fs = 1000             # taxa de amostragem
# T = 1                 # periodo amostrado
# Ts = 1/Fs             # periodo de amostragem
# t = np.arange(0,T,Ts) # vetor de tempo
# x1_n = 3*np.cos(2*np.pi*10*t + 0)           # sinal 1
# x2_n = 1*np.cos(2*np.pi*60*t + np.pi)       # sinal 2
# x3_n = 0.8*np.cos(2*np.pi*113*t + np.pi/6)  # sinal 3
# x_n = x1_n + x2_n + x3_n
# n = len(x_n)                # numero de amostras
# k = np.arange(n)            # vetor em k
# frq = k/T                   # os dois lados do vetor de frequencia
# perform_fft(x_n,Fs)

input()