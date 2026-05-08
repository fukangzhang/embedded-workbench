#ifndef EMBEDDED_WORKBENCH_BOARD_DIGITAL_OUTPUT_H
#define EMBEDDED_WORKBENCH_BOARD_DIGITAL_OUTPUT_H

#include <stdbool.h>

#include "embedded_workbench/digital_output.h"

typedef struct {
    const board_profile_t *profile;
    digital_output_level_t alarm_led_level;
    digital_output_level_t alarm_buzzer_level;
    digital_output_level_t alarm_actuator_level;
    unsigned int write_count;
} board_digital_output_context_t;

bool board_digital_output_init(
    digital_output_controller_t *controller,
    board_digital_output_context_t *context,
    const board_profile_t *profile);
bool board_digital_output_get_level(
    const board_digital_output_context_t *context,
    const board_pin_t *pin,
    digital_output_level_t *level);

#endif
