#include "embedded_workbench/command_parser.h"

#include <stddef.h>

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

static bool string_equals(const char *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        if (*left != *right) {
            return false;
        }

        left++;
        right++;
    }

    return *left == '\0' && *right == '\0';
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

    /* token 只读到空白或行尾；缓冲区不足时直接失败，避免截断后误识别命令。 */
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
    if (string_equals(text, "TEMP_WARN")) {
        *threshold = COMMAND_THRESHOLD_TEMP_WARNING_HIGH;
    } else if (string_equals(text, "TEMP_ALARM")) {
        *threshold = COMMAND_THRESHOLD_TEMP_ALARM_HIGH;
    } else if (string_equals(text, "HUM_WARN")) {
        *threshold = COMMAND_THRESHOLD_HUMIDITY_WARNING_HIGH;
    } else if (string_equals(text, "HUM_ALARM")) {
        *threshold = COMMAND_THRESHOLD_HUMIDITY_ALARM_HIGH;
    } else if (string_equals(text, "LIGHT_WARN_LOW")) {
        *threshold = COMMAND_THRESHOLD_LIGHT_WARNING_LOW;
    } else if (string_equals(text, "LIGHT_ALARM_LOW")) {
        *threshold = COMMAND_THRESHOLD_LIGHT_ALARM_LOW;
    } else if (string_equals(text, "SMOKE_WARN")) {
        *threshold = COMMAND_THRESHOLD_SMOKE_WARNING;
    } else if (string_equals(text, "SMOKE_ALARM")) {
        *threshold = COMMAND_THRESHOLD_SMOKE_ALARM;
    } else {
        *threshold = COMMAND_THRESHOLD_NONE;
        return false;
    }

    return true;
}

static bool parse_int32(const char *cursor, int32_t *value, const char **end_out)
{
    bool negative = false;
    uint32_t parsed = 0u;
    bool has_digit = false;

    cursor = skip_spaces(cursor);

    /* 手写整数解析是为了在裸机/小 libc 场景下避免依赖 strtol。 */
    if (*cursor == '-') {
        negative = true;
        cursor++;
    } else if (*cursor == '+') {
        cursor++;
    }

    while (*cursor >= '0' && *cursor <= '9') {
        uint32_t digit = (uint32_t)(*cursor - '0');

        has_digit = true;

        /* 每加入一位前先检查边界，保证 2147483647 和 -2147483648 都能被正确处理。 */
        if (!negative && parsed > (2147483647u - digit) / 10u) {
            return false;
        }

        if (negative && parsed > (2147483648u - digit) / 10u) {
            return false;
        }

        parsed = parsed * 10u + digit;
        cursor++;
    }

    if (!has_digit) {
        return false;
    }

    if (negative && parsed == 2147483648u) {
        *value = (-2147483647 - 1);
    } else if (negative) {
        *value = -(int32_t)parsed;
    } else {
        *value = (int32_t)parsed;
    }
    *end_out = cursor;
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

    /* 第一个 token 决定命令类型；后续分支再按命令类型读取剩余参数。 */
    cursor = read_token(cursor, token, sizeof(token));
    if (cursor == 0 || token[0] == '\0') {
        return false;
    }

    if (string_equals(token, "STATUS?")) {
        if (!has_only_line_end_after_spaces(cursor)) {
            return false;
        }

        command->type = COMMAND_TYPE_GET_STATUS;
        return true;
    }

    if (string_equals(token, "CONFIG?")) {
        if (!has_only_line_end_after_spaces(cursor)) {
            return false;
        }

        command->type = COMMAND_TYPE_GET_CONFIG;
        return true;
    }

    if (string_equals(token, "CLEAR_ALARM")) {
        if (!has_only_line_end_after_spaces(cursor)) {
            return false;
        }

        command->type = COMMAND_TYPE_CLEAR_ALARM;
        return true;
    }

    if (string_equals(token, "SET")) {
        int32_t value = 0;
        command_threshold_t threshold = COMMAND_THRESHOLD_NONE;

        /* SET 需要“阈值名 + 整数值”，并且值后面只能有空白或行尾。 */
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
