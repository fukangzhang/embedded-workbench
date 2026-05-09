#ifndef EMBEDDED_WORKBENCH_ALARM_OUTPUT_H
#define EMBEDDED_WORKBENCH_ALARM_OUTPUT_H

#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/alarm_state.h"

typedef enum {
    /* 指示灯关闭。 */
    ALARM_OUTPUT_INDICATOR_OFF = 0,
    /* warning 使用慢闪，提示异常但不制造强烈告警感。 */
    ALARM_OUTPUT_INDICATOR_SLOW_BLINK = 1,
    /* alarm 使用快闪，配合蜂鸣器和执行器形成强告警。 */
    ALARM_OUTPUT_INDICATOR_FAST_BLINK = 2,
    /* 传感器故障使用独立模式，方便和真实环境告警区分。 */
    ALARM_OUTPUT_INDICATOR_FAULT_BLINK = 3
} alarm_output_indicator_t;

/* 应用层给输出层的统一命令。
 * indicator 描述灯的模式，period_ms 只对闪烁有效；蜂鸣器和执行器是独立开关。 */
typedef struct {
    /* 灯的模式，不直接等于 GPIO 高低电平；闪烁还要经过 timing 模块计算。 */
    alarm_output_indicator_t indicator;
    /* 蜂鸣器是否打开。 */
    bool buzzer_enabled;
    /* 执行器是否打开，例如风扇、继电器或后续真实控制输出。 */
    bool actuator_enabled;
    /* 闪烁周期，单位毫秒；0 表示不闪烁或常亮语义由调用方决定。 */
    uint16_t period_ms;
} alarm_output_command_t;

/* 把告警状态翻译成可执行的输出策略，调用者提供 command 存放结果。
 * 这里不碰硬件，只决定“应该如何输出”。 */
bool alarm_output_command_for_state(alarm_state_t state, alarm_output_command_t *command);
const char *alarm_output_indicator_name(alarm_output_indicator_t indicator);

#endif
