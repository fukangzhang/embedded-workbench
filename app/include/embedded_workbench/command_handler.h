#ifndef EMBEDDED_WORKBENCH_COMMAND_HANDLER_H
#define EMBEDDED_WORKBENCH_COMMAND_HANDLER_H

#include <stdbool.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/command_parser.h"

typedef enum {
    /* 命令合法并已被 handler 接受。 */
    COMMAND_RESULT_OK = 0,
    /* 文本解析失败、命令类型未知，或 handler 收到无效输入。 */
    COMMAND_RESULT_INVALID_COMMAND = 1,
    /* 命令格式正确，但参数值不适合当前配置字段或破坏阈值关系。 */
    COMMAND_RESULT_INVALID_VALUE = 2
} command_result_t;

typedef struct {
    /* result 是每条命令的总结果，response_format_result 会把它写成 OK/ERR 文本。 */
    command_result_t result;
    /* SET 成功提交新配置时为 true；SET 被拒绝时保持 false。 */
    bool config_changed;
    /* STATUS? 不在 handler 内格式化响应，只把请求意图交给 session。 */
    bool status_requested;
    /* CONFIG? 同理，由 session 决定如何追加配置响应。 */
    bool config_requested;
    /* CLEAR_ALARM 当前是预留动作，session 会回显 clear_alarm=requested。 */
    bool alarm_clear_requested;
} command_handler_result_t;

/* 处理已经解析好的命令。
 * 这个层负责修改配置或设置请求标志，不直接生成串口响应文本。 */
command_handler_result_t command_handler_handle(
    const command_t *command,
    alarm_config_t *config);
const char *command_result_name(command_result_t result);

#endif
