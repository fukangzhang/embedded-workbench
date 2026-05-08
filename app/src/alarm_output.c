#include "embedded_workbench/alarm_output.h"

bool alarm_output_command_for_state(alarm_state_t state, alarm_output_command_t *command)
{
    if (command == 0) {
        return false;
    }

    command->indicator = ALARM_OUTPUT_INDICATOR_OFF;
    command->buzzer_enabled = false;
    command->actuator_enabled = false;
    command->period_ms = 0u;

    switch (state) {
    case ALARM_STATE_NORMAL:
        return true;
    case ALARM_STATE_WARNING:
        command->indicator = ALARM_OUTPUT_INDICATOR_SLOW_BLINK;
        command->period_ms = 1000u;
        return true;
    case ALARM_STATE_ALARM:
        command->indicator = ALARM_OUTPUT_INDICATOR_FAST_BLINK;
        command->buzzer_enabled = true;
        command->actuator_enabled = true;
        command->period_ms = 250u;
        return true;
    case ALARM_STATE_SENSOR_FAULT:
        command->indicator = ALARM_OUTPUT_INDICATOR_FAULT_BLINK;
        command->buzzer_enabled = true;
        command->period_ms = 500u;
        return true;
    default:
        return false;
    }
}

const char *alarm_output_indicator_name(alarm_output_indicator_t indicator)
{
    switch (indicator) {
    case ALARM_OUTPUT_INDICATOR_OFF:
        return "off";
    case ALARM_OUTPUT_INDICATOR_SLOW_BLINK:
        return "slow_blink";
    case ALARM_OUTPUT_INDICATOR_FAST_BLINK:
        return "fast_blink";
    case ALARM_OUTPUT_INDICATOR_FAULT_BLINK:
        return "fault_blink";
    default:
        return "unknown";
    }
}
