/*
 * display.c
 *
 *  Created on: 15 de nov. de 2026
 *      Author: felip
 */


#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "display.h"

/* Based on A4 = 440 Hz). Other octaves: x2 or /2. */
static const char *const NOTES[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

static const float FREQS[12] = {
    261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f,
    369.99f, 392.00f, 415.30f, 440.00f, 466.16f, 493.88f
};

/* Precomputed upper boundaries of each note */
static const float NOTE_UPPER[12] = {
    269.281f, 285.286f, 302.251f, 320.232f,
    339.284f, 359.477f, 380.867f, 403.522f,
    427.501f, 452.882f, 479.770f, 508.274f
};

#define OCTAVE_LOWER       269.281f
#define OCTAVE_UPPER       508.274f

#define TOLERANCE_CENTS    5.0f

void parse_display(float freq, char *line1, char *line2)
{
    float f = freq;

    // Normaliza para a 4 oitava
    int octave = 4;
    while (f < OCTAVE_LOWER) {
        f *= 2.0f;
        octave--;
    }
    while (f > OCTAVE_UPPER) {
        f /= 2.0f;
        octave++;
    }

    // Primeira nota cuja faixa contem f
    int note = 0;
    while (note < 11 && f > NOTE_UPPER[note]) {
        note++;
    }

    // Calcula desvio em cents (Centesimo de semitom)
    float cents_float = 1200.0f * log2f(f / FREQS[note]);

    // Posição no range da nota
    // 0 = borda baixa, 0.5 = afinado, 1 = borda alta
    float pos = 0.5f + cents_float * 0.01f;

    // Arredonda cents para inteiro
    int cents = (int)floorf(cents_float + 0.5f);

    bool tuned = fabsf(cents_float) <= TOLERANCE_CENTS;

    // Linha 1: "A4   446.0Hz +23" ou "A4   440.0Hz  OK"
    char name[4];
    snprintf(name, sizeof(name), "%s%d", NOTES[note], octave);

    unsigned int decimals = (unsigned)(freq * 10.0f + 0.5f) % 100000u;
    if (tuned) {
        snprintf(line1, 17, "%-3.3s %4u.%uHz  OK", name, decimals / 10, decimals % 10);
    } else {
        snprintf(line1, 17, "%-3.3s %4u.%uHz %+3d", name, decimals / 10, decimals % 10, cents);
    }

    // ponteiro 'O' entre b (baixo) e # (alto), centro na coluna 7
    // coluna = 1 + pos * 12  ->  pos 0 = col 1, 0.5 = col 7, 1 = col 13 */
    snprintf(line2, 17, "b------|------# ");

		int column = 7;
    if (!tuned){
    		column = 1 + (int)(pos * 12.0f + 0.5f);
    }
    line2[column] = 'O';
}
