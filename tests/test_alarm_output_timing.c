#include <stdbool.h>

#include "embedded_workbench/alarm_output_timing.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_bool(bool actual, bool expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_indicator_state(
    const alarm_output_command_t *command,
    uint32_t elapsed_ms,
    bool expected)
{
    bool is_on = false;

    if (!alarm_output_indicator_is_on(command, elapsed_ms, &is_on)) {
        return 1;
    }

    return expect_bool(is_on, expected);
}

int main(void)
{
    alarm_output_command_t command;
    bool is_on = true;

    if (expect_true(alarm_output_command_for_state(ALARM_STATE_NORMAL, &command)) != 0 ||
        expect_indicator_state(&command, 0u, false) != 0 ||
        expect_indicator_state(&command, 1000u, false) != 0) {
        return 1;
    }

    if (expect_true(alarm_output_command_for_state(ALARM_STATE_WARNING, &command)) != 0 ||
        expect_indicator_state(&command, 0u, true) != 0 ||
        expect_indicator_state(&command, 499u, true) != 0 ||
        expect_indicator_state(&command, 500u, false) != 0 ||
        expect_indicator_state(&command, 999u, false) != 0 ||
        expect_indicator_state(&command, 1000u, true) != 0) {
        return 2;
    }

    if (expect_true(alarm_output_command_for_state(ALARM_STATE_ALARM, &command)) != 0 ||
        expect_indicator_state(&command, 0u, true) != 0 ||
        expect_indicator_state(&command, 124u, true) != 0 ||
        expect_indicator_state(&command, 125u, false) != 0 ||
        expect_indicator_state(&command, 250u, true) != 0) {
        return 3;
    }

    if (expect_true(alarm_output_command_for_state(ALARM_STATE_SENSOR_FAULT, &command)) != 0 ||
        expect_indicator_state(&command, 0u, true) != 0 ||
        expect_indicator_state(&command, 249u, true) != 0 ||
        expect_indicator_state(&command, 250u, false) != 0 ||
        expect_indicator_state(&command, 500u, true) != 0) {
        return 4;
    }

    command.indicator = ALARM_OUTPUT_INDICATOR_FAST_BLINK;
    command.buzzer_enabled = false;
    command.actuator_enabled = false;
    command.period_ms = 0u;
    if (expect_indicator_state(&command, 9999u, true) != 0) {
        return 5;
    }

    command.indicator = (alarm_output_indicator_t)99;
    if (expect_false(alarm_output_indicator_is_on(&command, 0u, &is_on)) != 0 ||
        expect_false(alarm_output_indicator_is_on(0, 0u, &is_on)) != 0 ||
        expect_false(alarm_output_indicator_is_on(&command, 0u, 0)) != 0) {
        return 6;
    }

    return 0;
}
