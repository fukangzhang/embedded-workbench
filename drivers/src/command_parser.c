#include "embedded_workbench/command_parser.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

void command_init(command_t *command)
{
    if (command == 0) {
        return;
    }

    command->type = COMMAND_TYPE_INVALID;
    command->threshold = COMMAND_THRESHOLD_NONE;
    command->value = 0;
}

static bool is_space_char(char value)
{
    return value == ' ' || value == '\t';
}

static bool is_line_end(char value)
{
    return value == '\0' || value == '\r' || value == '\n';
}

static const char *skip_spaces(const char *cursor)
{
    while (is_space_char(*cursor)) {
        cursor++;
    }

    return cursor;
}

static const char *read_token(const char *cursor, char *buffer, size_t buffer_size)
{
    size_t index = 0;

    cursor = skip_spaces(cursor);

    while (!is_space_char(*cursor) && !is_line_end(*cursor)) {
        if (index + 1u >= buffer_size) {
            return 0;
        }

        buffer[index] = *cursor;
        index++;
        cursor++;
    }

    buffer[index] = '\0';
    return cursor;
}

static bool has_only_line_end_after_spaces(const char *cursor)
{
    cursor = skip_spaces(cursor);
    return is_line_end(*cursor);
}

static bool parse_threshold_name(const char *text, command_threshold_t *threshold)
{
    if (strcmp(text, "TEMP_WARN") == 0) {
        *threshold = COMMAND_THRESHOLD_TEMP_WARNING_HIGH;
    } else if (strcmp(text, "TEMP_ALARM") == 0) {
        *threshold = COMMAND_THRESHOLD_TEMP_ALARM_HIGH;
    } else if (strcmp(text, "HUM_WARN") == 0) {
        *threshold = COMMAND_THRESHOLD_HUMIDITY_WARNING_HIGH;
    } else if (strcmp(text, "HUM_ALARM") == 0) {
        *threshold = COMMAND_THRESHOLD_HUMIDITY_ALARM_HIGH;
    } else if (strcmp(text, "LIGHT_WARN_LOW") == 0) {
        *threshold = COMMAND_THRESHOLD_LIGHT_WARNING_LOW;
    } else if (strcmp(text, "LIGHT_ALARM_LOW") == 0) {
        *threshold = COMMAND_THRESHOLD_LIGHT_ALARM_LOW;
    } else if (strcmp(text, "SMOKE_WARN") == 0) {
        *threshold = COMMAND_THRESHOLD_SMOKE_WARNING;
    } else if (strcmp(text, "SMOKE_ALARM") == 0) {
        *threshold = COMMAND_THRESHOLD_SMOKE_ALARM;
    } else {
        *threshold = COMMAND_THRESHOLD_NONE;
        return false;
    }

    return true;
}

static bool parse_int32(const char *cursor, int32_t *value, const char **end_out)
{
    char *end = 0;
    long parsed = 0;

    errno = 0;
    cursor = skip_spaces(cursor);
    parsed = strtol(cursor, &end, 10);

    if (cursor == end || errno != 0) {
        return false;
    }

    *value = (int32_t)parsed;
    *end_out = end;
    return true;
}

bool command_parse(const char *line, command_t *command)
{
    char token[32];
    const char *cursor = line;

    if (line == 0 || command == 0) {
        return false;
    }

    command_init(command);

    cursor = read_token(cursor, token, sizeof(token));
    if (cursor == 0 || token[0] == '\0') {
        return false;
    }

    if (strcmp(token, "STATUS?") == 0) {
        if (!has_only_line_end_after_spaces(cursor)) {
            return false;
        }

        command->type = COMMAND_TYPE_GET_STATUS;
        return true;
    }

    if (strcmp(token, "CONFIG?") == 0) {
        if (!has_only_line_end_after_spaces(cursor)) {
            return false;
        }

        command->type = COMMAND_TYPE_GET_CONFIG;
        return true;
    }

    if (strcmp(token, "CLEAR_ALARM") == 0) {
        if (!has_only_line_end_after_spaces(cursor)) {
            return false;
        }

        command->type = COMMAND_TYPE_CLEAR_ALARM;
        return true;
    }

    if (strcmp(token, "SET") == 0) {
        int32_t value = 0;
        command_threshold_t threshold = COMMAND_THRESHOLD_NONE;

        cursor = read_token(cursor, token, sizeof(token));
        if (cursor == 0 || !parse_threshold_name(token, &threshold)) {
            return false;
        }

        if (!parse_int32(cursor, &value, &cursor)) {
            return false;
        }

        if (!has_only_line_end_after_spaces(cursor)) {
            return false;
        }

        command->type = COMMAND_TYPE_SET_THRESHOLD;
        command->threshold = threshold;
        command->value = value;
        return true;
    }

    return false;
}

const char *command_type_name(command_type_t type)
{
    switch (type) {
    case COMMAND_TYPE_GET_STATUS:
        return "get-status";
    case COMMAND_TYPE_GET_CONFIG:
        return "get-config";
    case COMMAND_TYPE_SET_THRESHOLD:
        return "set-threshold";
    case COMMAND_TYPE_CLEAR_ALARM:
        return "clear-alarm";
    case COMMAND_TYPE_INVALID:
    default:
        return "invalid";
    }
}

const char *command_threshold_name(command_threshold_t threshold)
{
    switch (threshold) {
    case COMMAND_THRESHOLD_TEMP_WARNING_HIGH:
        return "temp-warning-high";
    case COMMAND_THRESHOLD_TEMP_ALARM_HIGH:
        return "temp-alarm-high";
    case COMMAND_THRESHOLD_HUMIDITY_WARNING_HIGH:
        return "humidity-warning-high";
    case COMMAND_THRESHOLD_HUMIDITY_ALARM_HIGH:
        return "humidity-alarm-high";
    case COMMAND_THRESHOLD_LIGHT_WARNING_LOW:
        return "light-warning-low";
    case COMMAND_THRESHOLD_LIGHT_ALARM_LOW:
        return "light-alarm-low";
    case COMMAND_THRESHOLD_SMOKE_WARNING:
        return "smoke-warning";
    case COMMAND_THRESHOLD_SMOKE_ALARM:
        return "smoke-alarm";
    case COMMAND_THRESHOLD_NONE:
    default:
        return "none";
    }
}
