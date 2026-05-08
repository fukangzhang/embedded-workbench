#ifndef EMBEDDED_WORKBENCH_COMMAND_HANDLER_H
#define EMBEDDED_WORKBENCH_COMMAND_HANDLER_H

#include <stdbool.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/command_parser.h"

typedef enum {
    COMMAND_RESULT_OK = 0,
    COMMAND_RESULT_INVALID_COMMAND = 1,
    COMMAND_RESULT_INVALID_VALUE = 2
} command_result_t;

typedef struct {
    command_result_t result;
    bool config_changed;
    bool status_requested;
    bool config_requested;
    bool alarm_clear_requested;
} command_handler_result_t;

/* 处理已经解析好的命令。
 * 这个层负责修改配置或设置请求标志，不直接生成串口响应文本。 */
command_handler_result_t command_handler_handle(
    const command_t *command,
    alarm_config_t *config);
const char *command_result_name(command_result_t result);

#endif
