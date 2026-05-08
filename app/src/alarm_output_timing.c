#include "embedded_workbench/alarm_output_timing.h"

static bool indicator_is_valid(alarm_output_indicator_t indicator)
{
    return indicator == ALARM_OUTPUT_INDICATOR_OFF ||
           indicator == ALARM_OUTPUT_INDICATOR_SLOW_BLINK ||
           indicator == ALARM_OUTPUT_INDICATOR_FAST_BLINK ||
           indicator == ALARM_OUTPUT_INDICATOR_FAULT_BLINK;
}

bool alarm_output_indicator_is_on(
    const alarm_output_command_t *command,
    uint32_t elapsed_ms,
    bool *is_on)
{
    uint32_t phase_ms = 0u;
    uint32_t on_window_ms = 0u;

    if (command == 0 || is_on == 0 || !indicator_is_valid(command->indicator)) {
        return false;
    }

    if (command->indicator == ALARM_OUTPUT_INDICATOR_OFF) {
        *is_on = false;
        return true;
    }

    if (command->period_ms == 0u) {
        *is_on = true;
        return true;
    }

    phase_ms = elapsed_ms % command->period_ms;
    on_window_ms = ((uint32_t)command->period_ms + 1u) / 2u;
    *is_on = phase_ms < on_window_ms;

    return true;
}
