#include <stdio.h>
#include <math.h>
#include <string.h>
#include "nst_main.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

complex_t complex_add(complex_t a, complex_t b)
{
    complex_t result;
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
    return result;
}

complex_t complex_sub(complex_t a, complex_t b)
{
    complex_t result;
    result.real = a.real - b.real;
    result.imag = a.imag - b.imag;
    return result;
}

complex_t complex_mul(complex_t a, complex_t b)
{
    complex_t result;
    result.real = a.real * b.real - a.imag * b.imag;
    result.imag = a.real * b.imag + a.imag * b.real;
    return result;
}

complex_t complex_exp(double theta)
{
    complex_t result;
    result.real = cos(theta);
    result.imag = sin(theta);
    return result;
}

double complex_abs(complex_t a)
{
    return sqrt(a.real * a.real + a.imag * a.imag);
}

void fft(complex_t *X, int N)
{
    if (N <= 1)
        return;

    complex_t even[N / 2];
    complex_t odd[N / 2];

    for (int i = 0; i < N / 2; i++)
    {
        even[i] = X[i * 2];
        odd[i] = X[i * 2 + 1];
    }

    fft(even, N / 2);
    fft(odd, N / 2);

    for (int k = 0; k < N / 2; k++)
    {
        complex_t t = complex_mul(complex_exp(-2 * M_PI * k / N), odd[k]);
        X[k] = complex_add(even[k], t);
        X[k + N / 2] = complex_sub(even[k], t);
    }
}

void init_spectrogram_state(spectrogram_state_t *state, int window_size)
{
    state->buffer_index = 0;
    state->window_size = window_size;
    state->buffer = (nst_event_t *)malloc(window_size * sizeof(nst_event_t));
    state->spectrogram = (double *)malloc((window_size / 2) * sizeof(double));
    state->window = (double *)malloc(window_size * sizeof(double));

    memset(state->buffer, 0, window_size * sizeof(nst_event_t));
    memset(state->spectrogram, 0, (window_size / 2) * sizeof(double));

    // Apply windowing function (Hann window)
    for (int i = 0; i < window_size; i++)
    {
        state->window[i] = 0.5 * (1 - cos(2 * M_PI * i / (window_size - 1)));
    }
}

void algorithm_update(spectrogram_state_t *state, const nst_event_t *input_event)
{
    // Add new event to buffer
    state->buffer[state->buffer_index++] = *input_event;

    int window_size = state->window_size;
    complex_t X[window_size];

    // Approach 1: Without Windowing (Rectangular Window)
    for (int i = 0; i < window_size; i++)
    {
        X[i].real = state->buffer[i].values[0];
        X[i].imag = 0.0;
    }

    // Uncomment the following block for Approach 2: With Windowing (Hann Window)
    /*
    for (int i = 0; i < window_size; i++) {
        X[i].real = state->buffer[i].values[0] * state->window[i]; // Apply Hann window
        X[i].imag = 0.0;
    }
    */

    // Compute FFT
    fft(X, window_size);

    // Store magnitude of FFT results
    for (int i = 0; i < window_size / 2; i++)
    {
        state->spectrogram[i] = complex_abs(X[i]);
    }

    // Output the spectrogram data (for simplicity, we just print it here)
    printf("Spectrogram:\n");
    for (int i = 0; i < window_size / 2; i++)
    {
        printf("%f ", state->spectrogram[i]);
    }
    printf("\n");

    // Shift buffer to remove processed data
    int shift_amount = window_size / 2;
    if (state->buffer_index >= shift_amount)
    {
        memmove(state->buffer, state->buffer + shift_amount, (state->buffer_index - shift_amount) * sizeof(nst_event_t));
        state->buffer_index -= shift_amount;
    }
}

// int main() {
//     // Example usage
//     spectrogram_state_t state;
//     init_spectrogram_state(&state, 256); // Pass window size here

//     nst_event_t input_event;
//     for (int i = 0; i < 1024; i++) {
//         input_event.timestamp = i / (double)1000.0;
//         input_event.values[0] = sin(2 * M_PI * 10 * input_event.timestamp); // Example sine wave
//         algorithm_update(&state, &input_event);
//     }

//     // Free allocated memory
//     free(state.buffer);
//     free(state.spectrogram);
//     free(state.window);

//     return 0;
// }