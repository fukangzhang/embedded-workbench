#ifndef EMBEDDED_WORKBENCH_ALARM_OUTPUT_DIGITAL_SINK_H
#define EMBEDDED_WORKBENCH_ALARM_OUTPUT_DIGITAL_SINK_H

#include <stdbool.h>

#include "embedded_workbench/alarm_output_sink.h"
#include "embedded_workbench/digital_output.h"

typedef struct {
    digital_output_controller_t *digital_output;
    const board_profile_t *profile;
} alarm_output_digital_sink_context_t;

bool alarm_output_digital_sink_init(
    alarm_output_sink_t *sink,
    alarm_output_digital_sink_context_t *context,
    digital_output_controller_t *digital_output,
    const board_profile_t *profile);

#endif
