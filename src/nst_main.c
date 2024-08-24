#include "nst_main.h"
#include <math.h>
#include <stdio.h>

nst_data_t nst_data;

void algorithm_init()
{

}

void algorithm_update(const nst_event_t input_event, nst_event_t output_events[4], int *output_events_count)
{
    printf("Hello from algorithm_update\n");
    *output_events_count = 0;
}
