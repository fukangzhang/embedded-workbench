#ifndef EMBEDDED_WORKBENCH_COMMAND_RESPONDER_H
#define EMBEDDED_WORKBENCH_COMMAND_RESPONDER_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/sensor_sample.h"

typedef struct {
    /* The responder owns no storage; it mutates the application state provided by its caller. */
    alarm_config_t *config;
    alarm_state_t *state;
    sensor_sample_t *sample;
} command_responder_t;

bool command_responder_init(
    command_responder_t *responder,
    alarm_config_t *config,
    alarm_state_t *state,
    sensor_sample_t *sample);

bool command_responder_handle_command(
    command_responder_t *responder,
    const command_t *command,
    char *response,
    size_t response_size);

#endif
