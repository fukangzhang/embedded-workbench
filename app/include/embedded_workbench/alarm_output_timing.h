#ifndef EMBEDDED_WORKBENCH_ALARM_OUTPUT_TIMING_H
#define EMBEDDED_WORKBENCH_ALARM_OUTPUT_TIMING_H

#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/alarm_output.h"

bool alarm_output_indicator_is_on(
    const alarm_output_command_t *command,
    uint32_t elapsed_ms,
    bool *is_on);

#endif
