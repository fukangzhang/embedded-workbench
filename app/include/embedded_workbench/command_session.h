#ifndef EMBEDDED_WORKBENCH_COMMAND_SESSION_H
#define EMBEDDED_WORKBENCH_COMMAND_SESSION_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/sensor_sample.h"

typedef struct {
    /* session 持有一次通信会话需要读写的应用状态。
     * 未来 host_sim、USART2 task、测试假通信都可以复用同一个处理入口。 */
    alarm_config_t *config;
    alarm_state_t *state;
    const sensor_sample_t *sample;
} command_session_t;

/* 处理一行完整命令，并把一条或多条响应拼到 response。
 * line 应该已经由 serial_line 或 fgets 组装完成；本模块不处理逐字节输入。 */
bool command_session_handle_line(
    command_session_t *session,
    const char *line,
    char *response,
    size_t response_size);

#endif
