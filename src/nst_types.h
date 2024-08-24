#ifndef NST_TYPES_H
#define NST_TYPES_H
#include <float.h>
#include <math.h>

#define SPECTROGRAM_ROWS 256
#define SPECTROGRAM_COLS 256
#define NST_EVENT_MAX_VALUES_COUNT 32

typedef struct
{
    double timestamp;
    unsigned int id;
    int values_count;
    double values[NST_EVENT_MAX_VALUES_COUNT];
} nst_event_t;

typedef struct
{
    float data[SPECTROGRAM_ROWS][SPECTROGRAM_COLS]; // Statically allocated 2D array to hold the spectrogram data
} spectrogram;

typedef struct
{
    double x_scale;
} nst_data_t;

#endif