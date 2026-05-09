#ifndef EMBEDDED_WORKBENCH_RESPONSE_FORMAT_H
#define EMBEDDED_WORKBENCH_RESPONSE_FORMAT_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/command_handler.h"
#include "embedded_workbench/sensor_sample.h"

/* 将内部结果格式化为面向通信输出的单行文本。
 *
 * 这些函数只负责“结构体 -> 文本”，不修改业务状态。
 * buffer 太小时返回 false，调用者可据此选择丢弃或发送错误响应。 */
bool response_format_result(
    char *buffer,
    size_t buffer_size,
    const command_handler_result_t *result);

/* 输出当前状态、最新样本和由状态推导出的输出策略。
 * 这让串口调试时一行就能看出“传感器读数 -> 告警状态 -> 输出动作”。 */
bool response_format_status(
    char *buffer,
    size_t buffer_size,
    alarm_state_t state,
    const sensor_sample_t *sample);

/* 输出完整阈值快照，而不是只输出刚修改的字段。
 * 这样人在终端里能马上确认当前配置整体是否合理。 */
bool response_format_config(
    char *buffer,
    size_t buffer_size,
    const alarm_config_t *config);

#endif
