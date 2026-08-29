# Implementação da DFT (Discrete Fourier Analysis)
#
# Nomes: Felipe Pasini Graeff e Vinicius da Rosa Frohlich

import matplotlib.pyplot as plt
import numpy as np

def DFT(data):
    N = data.size
    X = np.arange(N,dtype=complex)
    for k in range(N): # varredura em k
        sum = 0
        for n in range(N): # varredura em n, para cada k
            sum += data[n] * np.exp(1j*2*np.pi*n*k/N) # aplicação da equação
        X[k] = sum/N # pondera pelo número de amostras analisadas

    return X

Fs = 1000             # taxa de amostragem
T = 1                 # periodo amostrado

Ts = 1/Fs             # periodo de amostragem
t = np.arange(0,T,Ts) # vetor de tempo

x1_n = 3*np.cos(2*np.pi*10*t + 0)           # sinal 1
x2_n = 1*np.cos(2*np.pi*60*t + np.pi)       # sinal 2
x3_n = 0.8*np.cos(2*np.pi*113*t + np.pi/6)  # sinal 3

x_n = x1_n + x2_n + x3_n

n = len(x_n)                # numero de amostras
k = np.arange(n)            # vetor em k
frq = k/T                   # os dois lados do vetor de frequencia
frq = frq[range(int(n/2))]  # apenas primeira metada do vetor

X = DFT(x_n) 

X = X[range(int(n/2))]          # apenas primeira metade da transformada também
mod = 2*abs(X)                  # modulos dos complexos 
phase = np.angle(X,deg='true')  # angulos de fase em graus
phase = phase * (mod > 1e-6)    # zera a fase das frequências que não tem modulo

fig, ax = plt.subplots(1, 1)    # plot do sinal no tempo
ax.plot(t,x_n)
ax.set_xlabel('Tempo (s)')
ax.set_ylabel('Amplitude')
fig.show()

fig, ax = plt.subplots(2, 1)    # plot da amplitude e fase da transformada
ax[0].plot(frq,mod,'r')
ax[0].set_xlabel('Freq (Hz)')
ax[0].set_ylabel('Amplitude')

ax[1].plot(frq,phase,'b')
ax[1].set_xlabel('Freq (Hz)')
ax[1].set_ylabel('Fase (º)')

fig.show()
#input() # Não encerra a aplicação imediatamente, aguardando input do usuário.