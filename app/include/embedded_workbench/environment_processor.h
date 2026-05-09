#ifndef EMBEDDED_WORKBENCH_ENVIRONMENT_PROCESSOR_H
#define EMBEDDED_WORKBENCH_ENVIRONMENT_PROCESSOR_H

#include <stdbool.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/sensor_sample.h"

typedef bool (*environment_processor_publish_fn)(void *context, alarm_state_t state);

typedef struct {
    /* config and state belong together: the same sample can produce different states under different thresholds. */
    alarm_config_t config;
    alarm_state_t state;

    /* publish is the next hop after processing, such as an RTOS alarm event queue send function. */
    environment_processor_publish_fn publish;
    void *publish_context;
} environment_processor_t;

bool environment_processor_init(
    environment_processor_t *processor,
    const alarm_config_t *config,
    alarm_state_t initial_state,
    environment_processor_publish_fn publish,
    void *publish_context);

bool environment_processor_process_sample(
    environment_processor_t *processor,
    const sensor_sample_t *sample);

bool environment_processor_update_config(
    environment_processor_t *processor,
    const alarm_config_t *config);

alarm_state_t environment_processor_current_state(const environment_processor_t *processor);

#endif
