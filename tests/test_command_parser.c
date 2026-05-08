#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/command_parser.h"

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
    command_t command;

    command_init(&command);
    if (expect_int(command.type, COMMAND_TYPE_INVALID) != 0) {
        return 1;
    }

    if (expect_true(command_parse("STATUS?\r\n", &command)) != 0 ||
        expect_int(command.type, COMMAND_TYPE_GET_STATUS) != 0) {
        return 2;
    }

    if (expect_true(command_parse(" CONFIG? ", &command)) != 0 ||
        expect_int(command.type, COMMAND_TYPE_GET_CONFIG) != 0) {
        return 3;
    }

    if (expect_true(command_parse("CLEAR_ALARM", &command)) != 0 ||
        expect_int(command.type, COMMAND_TYPE_CLEAR_ALARM) != 0) {
        return 4;
    }

    if (expect_true(command_parse("SET TEMP_WARN 350", &command)) != 0 ||
        expect_int(command.type, COMMAND_TYPE_SET_THRESHOLD) != 0 ||
        expect_int(command.threshold, COMMAND_THRESHOLD_TEMP_WARNING_HIGH) != 0 ||
        expect_int(command.value, 350) != 0) {
        return 5;
    }

    if (expect_true(command_parse("SET LIGHT_ALARM_LOW 30", &command)) != 0 ||
        expect_int(command.threshold, COMMAND_THRESHOLD_LIGHT_ALARM_LOW) != 0 ||
        expect_int(command.value, 30) != 0) {
        return 6;
    }

    if (expect_true(command_parse("SET TEMP_ALARM -50", &command)) != 0 ||
        expect_int(command.threshold, COMMAND_THRESHOLD_TEMP_ALARM_HIGH) != 0 ||
        expect_int(command.value, -50) != 0) {
        return 7;
    }

    if (expect_false(command_parse("STATUS? NOW", &command)) != 0) {
        return 8;
    }

    if (expect_false(command_parse("SET UNKNOWN 1", &command)) != 0) {
        return 9;
    }

    if (expect_false(command_parse("SET TEMP_WARN abc", &command)) != 0) {
        return 10;
    }

    if (expect_false(command_parse("SET TEMP_WARN 2147483648", &command)) != 0) {
        return 11;
    }

    if (expect_false(command_parse("SET TEMP_WARN -2147483649", &command)) != 0) {
        return 12;
    }

    if (expect_false(command_parse(0, &command)) != 0) {
        return 13;
    }

    if (expect_false(command_parse("STATUS?", 0)) != 0) {
        return 14;
    }

    if (expect_string(command_type_name(COMMAND_TYPE_SET_THRESHOLD), "set-threshold") != 0) {
        return 15;
    }

    if (expect_string(command_threshold_name(COMMAND_THRESHOLD_SMOKE_ALARM), "smoke-alarm") != 0) {
        return 16;
    }

    return 0;
}
