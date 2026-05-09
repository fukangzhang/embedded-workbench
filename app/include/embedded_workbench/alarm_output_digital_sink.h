#ifndef EMBEDDED_WORKBENCH_ALARM_OUTPUT_DIGITAL_SINK_H
#define EMBEDDED_WORKBENCH_ALARM_OUTPUT_DIGITAL_SINK_H

#include <stdbool.h>

#include "embedded_workbench/alarm_output_sink.h"
#include "embedded_workbench/digital_output.h"

typedef struct {
    /* digital_output 是更底层的“写某个板级引脚高/低”的驱动抽象。 */
    digital_output_controller_t *digital_output;
    /* profile 告诉适配器 LED、蜂鸣器、执行器分别接在哪个板级引脚。 */
    const board_profile_t *profile;
} alarm_output_digital_sink_context_t;

/* 把 alarm_output_sink_t 接到 digital_output_controller_t。
 * 初始化后，alarm_output_sink_apply 可以通过本适配器驱动 profile 中的三个输出脚。 */
bool alarm_output_digital_sink_init(
    alarm_output_sink_t *sink,
    alarm_output_digital_sink_context_t *context,
    digital_output_controller_t *digital_output,
    const board_profile_t *profile);

#endif
