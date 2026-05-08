#ifndef EMBEDDED_WORKBENCH_RESPONSE_FORMAT_H
#define EMBEDDED_WORKBENCH_RESPONSE_FORMAT_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/command_handler.h"
#include "embedded_workbench/sensor_sample.h"

/* 将内部状态格式化为面向通信输出的单行文本。
 * buffer 太小时返回 false，调用者可据此选择丢弃或发送错误响应。 */
bool response_format_result(
    char *buffer,
    size_t buffer_size,
    const command_handler_result_t *result);
bool response_format_status(
    char *buffer,
    size_t buffer_size,
    alarm_state_t state,
    const sensor_sample_t *sample);
bool response_format_config(
    char *buffer,
    size_t buffer_size,
    const alarm_config_t *config);

#endif
