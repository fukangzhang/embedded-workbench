#ifndef EMBEDDED_WORKBENCH_RESPONSE_FORMAT_H
#define EMBEDDED_WORKBENCH_RESPONSE_FORMAT_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/command_handler.h"
#include "embedded_workbench/sensor_sample.h"

bool response_format_result(
    char *buffer,
    size_t buffer_size,
    const command_handler_result_t *result);
bool response_format_status(
    char *buffer,
    size_t buffer_size,
    alarm_state_t state,
    const sensor_sample_t *sample);
bool response_format_config(
    char *buffer,
    size_t buffer_size,
    const alarm_config_t *config);

#endif
