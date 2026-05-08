#ifndef EMBEDDED_WORKBENCH_ALARM_OUTPUT_SINK_H
#define EMBEDDED_WORKBENCH_ALARM_OUTPUT_SINK_H

#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/alarm_output.h"

typedef bool (*alarm_output_sink_set_indicator_fn)(
    void *context,
    alarm_output_indicator_t indicator,
    uint16_t period_ms);
typedef bool (*alarm_output_sink_set_enabled_fn)(void *context, bool enabled);

/* sink 是应用层和具体硬件输出之间的适配接口。
 * app 只依赖这些函数指针，不需要知道 LED、蜂鸣器或执行器最终接在哪个引脚。 */
typedef struct {
    alarm_output_sink_set_indicator_fn set_indicator;
    alarm_output_sink_set_enabled_fn set_buzzer;
    alarm_output_sink_set_enabled_fn set_actuator;
} alarm_output_sink_ops_t;

typedef struct {
    const alarm_output_sink_ops_t *ops;
    void *context;
} alarm_output_sink_t;

/* 按固定顺序应用灯、蜂鸣器、执行器；任一步失败都会停止并返回 false。 */
bool alarm_output_sink_apply(
    alarm_output_sink_t *sink,
    const alarm_output_command_t *command);

#endif
