#ifndef EMBEDDED_WORKBENCH_COMMAND_SESSION_H
#define EMBEDDED_WORKBENCH_COMMAND_SESSION_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/sensor_sample.h"

typedef struct {
    /* session 持有一次通信会话需要读写的应用状态。
     *
     * config/state 用指针，是因为 SET 命令和状态重算需要把结果写回调用者。
     * sample 用 const 指针，是因为命令处理只读取最新样本，不应该在这里改传感器数据。
     * 未来 host_sim、USART2 task、测试假通信都可以复用同一个处理入口。 */
    alarm_config_t *config;
    alarm_state_t *state;
    const sensor_sample_t *sample;
} command_session_t;

/* 处理一行完整命令，并把一条或多条响应拼到 response。
 * line 应该已经由 serial_line 或 fgets 组装完成；本模块不处理逐字节输入。
 *
 * 典型流程：
 * 1. command_parse 把文本变成 command_t
 * 2. command_handler_handle 修改配置或设置请求标志
 * 3. response_format_* 把结果、状态、配置拼成串口可发的文本 */
bool command_session_handle_line(
    command_session_t *session,
    const char *line,
    char *response,
    size_t response_size);

#endif
