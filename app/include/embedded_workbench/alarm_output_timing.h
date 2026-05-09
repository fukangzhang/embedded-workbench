#ifndef EMBEDDED_WORKBENCH_ALARM_OUTPUT_TIMING_H
#define EMBEDDED_WORKBENCH_ALARM_OUTPUT_TIMING_H

#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/alarm_output.h"

/* 根据输出命令和已过去的时间，计算指示灯当前应该亮还是灭。
 *
 * alarm_output_command_for_state 只说“慢闪/快闪”，本函数把这种策略落成某一时刻
 * 的 bool 结果。这样 FreeRTOS task 或主机测试都可以复用同一套闪烁计算。 */
bool alarm_output_indicator_is_on(
    const alarm_output_command_t *command,
    uint32_t elapsed_ms,
    bool *is_on);

#endif
