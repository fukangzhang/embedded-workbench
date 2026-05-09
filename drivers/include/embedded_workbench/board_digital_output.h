#ifndef EMBEDDED_WORKBENCH_BOARD_DIGITAL_OUTPUT_H
#define EMBEDDED_WORKBENCH_BOARD_DIGITAL_OUTPUT_H

#include <stdbool.h>

#include "embedded_workbench/digital_output.h"

typedef struct {
    /* profile 决定哪些 pin 对应 LED、蜂鸣器、执行器。 */
    const board_profile_t *profile;
    /* 主机模拟后端只记录每个输出脚最后一次写入的电平。 */
    digital_output_level_t alarm_led_level;
    digital_output_level_t alarm_buzzer_level;
    digital_output_level_t alarm_actuator_level;
    /* write_count 帮测试确认确实发生了输出动作。 */
    unsigned int write_count;
} board_digital_output_context_t;

/* 初始化一个 profile-backed 的主机侧数字输出后端。
 * 它不操作硬件，只把写入结果保存在 context 中供测试断言。 */
bool board_digital_output_init(
    digital_output_controller_t *controller,
    board_digital_output_context_t *context,
    const board_profile_t *profile);
bool board_digital_output_get_level(
    const board_digital_output_context_t *context,
    const board_pin_t *pin,
    digital_output_level_t *level);

#endif
