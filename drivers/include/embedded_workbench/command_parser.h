#ifndef EMBEDDED_WORKBENCH_COMMAND_PARSER_H
#define EMBEDDED_WORKBENCH_COMMAND_PARSER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    COMMAND_TYPE_INVALID = 0,
    COMMAND_TYPE_GET_STATUS = 1,
    COMMAND_TYPE_GET_CONFIG = 2,
    COMMAND_TYPE_SET_THRESHOLD = 3,
    COMMAND_TYPE_CLEAR_ALARM = 4
} command_type_t;

typedef enum {
    COMMAND_THRESHOLD_NONE = 0,
    COMMAND_THRESHOLD_TEMP_WARNING_HIGH = 1,
    COMMAND_THRESHOLD_TEMP_ALARM_HIGH = 2,
    COMMAND_THRESHOLD_HUMIDITY_WARNING_HIGH = 3,
    COMMAND_THRESHOLD_HUMIDITY_ALARM_HIGH = 4,
    COMMAND_THRESHOLD_LIGHT_WARNING_LOW = 5,
    COMMAND_THRESHOLD_LIGHT_ALARM_LOW = 6,
    COMMAND_THRESHOLD_SMOKE_WARNING = 7,
    COMMAND_THRESHOLD_SMOKE_ALARM = 8
} command_threshold_t;

typedef struct {
    command_type_t type;
    command_threshold_t threshold;
    int32_t value;
} command_t;

void command_init(command_t *command);
bool command_parse(const char *line, command_t *command);
const char *command_type_name(command_type_t type);
const char *command_threshold_name(command_threshold_t threshold);

#endif
