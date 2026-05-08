#ifndef EMBEDDED_WORKBENCH_ALARM_OUTPUT_H
#define EMBEDDED_WORKBENCH_ALARM_OUTPUT_H

#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/alarm_state.h"

typedef enum {
    ALARM_OUTPUT_INDICATOR_OFF = 0,
    ALARM_OUTPUT_INDICATOR_SLOW_BLINK = 1,
    ALARM_OUTPUT_INDICATOR_FAST_BLINK = 2,
    ALARM_OUTPUT_INDICATOR_FAULT_BLINK = 3
} alarm_output_indicator_t;

/* 应用层给输出层的统一命令。
 * indicator 描述灯的模式，period_ms 只对闪烁有效；蜂鸣器和执行器是独立开关。 */
typedef struct {
    alarm_output_indicator_t indicator;
    bool buzzer_enabled;
    bool actuator_enabled;
    uint16_t period_ms;
} alarm_output_command_t;

/* 把告警状态翻译成可执行的输出策略，调用者提供 command 存放结果。 */
bool alarm_output_command_for_state(alarm_state_t state, alarm_output_command_t *command);
const char *alarm_output_indicator_name(alarm_output_indicator_t indicator);

#endif
