/*
 * Afinador cromatico para LCD 16x2.
 * Recebe uma frequencia em Hz, descobre a nota e monta as duas linhas do LCD.
 */

#include <stdio.h>
#include <math.h>
#include "afinador.h"

/* Notas da oitava 4 (A4 = 440 Hz). Outras oitavas: x2 ou /2. */
static const char *const notas[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};
static const float freqs[12] = {
    261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f,
    369.99f, 392.00f, 415.30f, 440.00f, 466.16f, 493.88f
};

#define MEIO_SEMITOM    1.0293022366f   /* 2^(1/24): borda da faixa de cada nota */
#define TOLERANCIA      0.05f           /* |pos - 0.5| <= 0.05 (5 cents) = afinado */


float afinador(float freq, char *linha1, char *linha2)
{
    float f, minimo, maximo, pos;
    int nota, oitava, cents, coluna;
    unsigned decimos;
    char nome[4];

    /* Fora da faixa C0..B8 (~15,9 a ~8134 Hz) ou sem som */
    if (freq < freqs[0] / 16.0f / MEIO_SEMITOM ||
        freq > freqs[11] * 16.0f * MEIO_SEMITOM) {
        snprintf(linha1, 17, "   Sem sinal    ");
        snprintf(linha2, 17, "                ");
        return -1.0f;
    }

    /* 1) Leva a frequencia para a oitava 4, contando as oitavas */
    f = freq;
    oitava = 4;
    while (f < freqs[0] / MEIO_SEMITOM) {
        f = f * 2.0f;
        oitava--;
    }
    while (f > freqs[11] * MEIO_SEMITOM) {
        f = f / 2.0f;
        oitava++;
    }

    /* 2) Primeira nota cuja faixa contem f */
    nota = 0;
    while (nota < 11 && f > freqs[nota] * MEIO_SEMITOM) {
        nota++;
    }

    /* 3) Posicao na faixa: (f - min) / (max - min), em log2 porque as notas
     *    sao espacadas por multiplicacao. 0 = borda baixa, 0.5 = afinado, 1 = borda alta */
    minimo = freqs[nota] / MEIO_SEMITOM;
    maximo = freqs[nota] * MEIO_SEMITOM;
    pos = (log2f(f) - log2f(minimo)) / (log2f(maximo) - log2f(minimo));
    if (pos < 0.0f) pos = 0.0f;
    if (pos > 1.0f) pos = 1.0f;

    /* Desvio em cents: pos 0..1 -> -50..+50, arredondado */
    cents = (int)floorf((pos - 0.5f) * 100.0f + 0.5f);
    if (cents < -50) cents = -50;
    if (cents > 50) cents = 50;

    /* 4) Linha 1: "A4   446.0Hz +23" ou "A4   440.0Hz  OK"
     *    Frequencia sem %f (printf de float costuma vir desligado no MCU) */
    snprintf(nome, sizeof(nome), "%s%d", notas[nota], oitava);
    decimos = (unsigned)(freq * 10.0f + 0.5f) % 100000u;

    if (fabsf(pos - 0.5f) <= TOLERANCIA) {
        snprintf(linha1, 17, "%-3.3s %4u.%uHz  OK", nome, decimos / 10, decimos % 10);
    } else {
        snprintf(linha1, 17, "%-3.3s %4u.%uHz %+3d", nome, decimos / 10, decimos % 10, cents);
    }

    /* 5) Linha 2: ponteiro 'O' entre b (baixo) e # (alto), centro na coluna 7
     *    coluna = 1 + pos * 12  ->  pos 0 = col 1, 0.5 = col 7, 1 = col 13 */
    snprintf(linha2, 17, "b------|------# ");

    if (fabsf(pos - 0.5f) <= TOLERANCIA) {
        coluna = 7;
    } else {
        coluna = 1 + (int)(pos * 12.0f + 0.5f);
    }
    linha2[coluna] = 'O';

    return pos;     /* 0..1, util para LED ou buzzer */
}
