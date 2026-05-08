#ifndef EMBEDDED_WORKBENCH_ALARM_OUTPUT_H
#define EMBEDDED_WORKBENCH_ALARM_OUTPUT_H

#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/alarm_state.h"

typedef enum {
    ALARM_OUTPUT_INDICATOR_OFF = 0,
    ALARM_OUTPUT_INDICATOR_SLOW_BLINK = 1,
    ALARM_OUTPUT_INDICATOR_FAST_BLINK = 2,
    ALARM_OUTPUT_INDICATOR_FAULT_BLINK = 3
} alarm_output_indicator_t;

typedef struct {
    alarm_output_indicator_t indicator;
    bool buzzer_enabled;
    bool actuator_enabled;
    uint16_t period_ms;
} alarm_output_command_t;

bool alarm_output_command_for_state(alarm_state_t state, alarm_output_command_t *command);
const char *alarm_output_indicator_name(alarm_output_indicator_t indicator);

#endif
