#ifndef EMBEDDED_WORKBENCH_SENSOR_ACQUISITION_H
#define EMBEDDED_WORKBENCH_SENSOR_ACQUISITION_H

#include <stdbool.h>

#include "embedded_workbench/sensor_sample.h"
#include "embedded_workbench/sensor_source.h"

typedef bool (*sensor_acquisition_submit_fn)(void *context, const sensor_sample_t *sample);

typedef struct {
    /* source decides where a sample comes from: fake sequence, real sensor, or another driver. */
    const sensor_source_t *source;

    /* submit is the next hop after acquisition, such as an RTOS queue send function. */
    sensor_acquisition_submit_fn submit;
    void *submit_context;
} sensor_acquisition_t;

bool sensor_acquisition_init(
    sensor_acquisition_t *acquisition,
    const sensor_source_t *source,
    sensor_acquisition_submit_fn submit,
    void *submit_context);

bool sensor_acquisition_poll(sensor_acquisition_t *acquisition);

#endif
