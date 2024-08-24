#ifndef NST_MAIN_H
#define NST_MAIN_H
#include "nst_types.h"
#ifdef __cplusplus

extern "C" {

#endif

extern nst_data_t nst_data;

extern void algorithm_init();

extern void algorithm_update(const nst_event_t input_event, nst_event_t output_events[4], int *output_events_count);

#ifdef __cplusplus
}
#endif

#endif