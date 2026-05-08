#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/command_handler.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_int(int actual, int expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

int main(void)
{
    alarm_config_t config = alarm_config_default();
    command_t command;
    command_handler_result_t result;

    command_init(&command);
    command.type = COMMAND_TYPE_GET_STATUS;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_OK) != 0 ||
        expect_true(result.status_requested) != 0 ||
        expect_false(result.config_changed) != 0) {
        return 1;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_GET_CONFIG;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_OK) != 0 ||
        expect_true(result.config_requested) != 0) {
        return 2;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_CLEAR_ALARM;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_OK) != 0 ||
        expect_true(result.alarm_clear_requested) != 0) {
        return 3;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_SET_THRESHOLD;
    command.threshold = COMMAND_THRESHOLD_TEMP_WARNING_HIGH;
    command.value = 360;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_OK) != 0 ||
        expect_true(result.config_changed) != 0 ||
        expect_int(config.temperature_warning_high_c_x10, 360) != 0) {
        return 4;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_SET_THRESHOLD;
    command.threshold = COMMAND_THRESHOLD_SMOKE_ALARM;
    command.value = 700;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_OK) != 0 ||
        expect_int(config.smoke_alarm_ppm, 700) != 0) {
        return 5;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_SET_THRESHOLD;
    command.threshold = COMMAND_THRESHOLD_TEMP_WARNING_HIGH;
    command.value = 500;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_INVALID_VALUE) != 0 ||
        expect_int(config.temperature_warning_high_c_x10, 360) != 0) {
        return 6;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_SET_THRESHOLD;
    command.threshold = COMMAND_THRESHOLD_HUMIDITY_WARNING_HIGH;
    command.value = -1;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_INVALID_VALUE) != 0) {
        return 7;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_SET_THRESHOLD;
    command.threshold = COMMAND_THRESHOLD_SMOKE_WARNING;
    command.value = 70000;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_INVALID_VALUE) != 0) {
        return 8;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_SET_THRESHOLD;
    command.threshold = COMMAND_THRESHOLD_LIGHT_WARNING_LOW;
    command.value = -1;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_INVALID_VALUE) != 0) {
        return 9;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_INVALID;
    result = command_handler_handle(&command, &config);
    if (expect_int(result.result, COMMAND_RESULT_INVALID_COMMAND) != 0) {
        return 10;
    }

    if (expect_int(command_handler_handle(0, &config).result, COMMAND_RESULT_INVALID_COMMAND) != 0) {
        return 11;
    }

    if (expect_int(command_handler_handle(&command, 0).result, COMMAND_RESULT_INVALID_COMMAND) != 0) {
        return 12;
    }

    if (expect_string(command_result_name(COMMAND_RESULT_INVALID_VALUE), "invalid-value") != 0) {
        return 13;
    }

    return 0;
}
