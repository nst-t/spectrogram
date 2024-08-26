#ifndef NST_MAIN_H
#define NST_MAIN_H

#include "nst_types.h"

#define WINDOW_SIZE 256

typedef struct
{
    double real;
    double imag;
} complex_t;

complex_t complex_add(complex_t a, complex_t b);
complex_t complex_sub(complex_t a, complex_t b);
complex_t complex_mul(complex_t a, complex_t b);
complex_t complex_exp(double theta);
double complex_abs(complex_t a);

void fft(complex_t *X, int N);

typedef struct
{
    nst_event_t *buffer;
    int buffer_index;
    double *window;
    double *spectrogram;
    int window_size;
} spectrogram_state_t;

void init_spectrogram_state(spectrogram_state_t *state, int window_size);
void algorithm_update(spectrogram_state_t *state, const nst_event_t *input_event);

#endif // NST_MAIN_H