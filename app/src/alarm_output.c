#include "embedded_workbench/alarm_output.h"

/* alarm_output 是“告警状态 -> 输出命令”的翻译层。
 * 它仍然不碰 GPIO，只生成一个抽象命令，后续 sink 再把命令落到具体硬件或测试后端。 */

bool alarm_output_command_for_state(alarm_state_t state, alarm_output_command_t *command)
{
    if (command == 0) {
        return false;
    }

    /* 先写入安全默认值：后面的 case 只覆盖需要打开的输出。 */
    command->indicator = ALARM_OUTPUT_INDICATOR_OFF;
    command->buzzer_enabled = false;
    command->actuator_enabled = false;
    command->period_ms = 0u;

    switch (state) {
    case ALARM_STATE_NORMAL:
        return true;
    case ALARM_STATE_WARNING:
        /* warning 只给视觉提示，避免轻微异常时蜂鸣器和执行器过早动作。 */
        command->indicator = ALARM_OUTPUT_INDICATOR_SLOW_BLINK;
        command->period_ms = 1000u;
        return true;
    case ALARM_STATE_ALARM:
        /* alarm 同时打开灯、蜂鸣器和执行器，是最强输出策略。 */
        command->indicator = ALARM_OUTPUT_INDICATOR_FAST_BLINK;
        command->buzzer_enabled = true;
        command->actuator_enabled = true;
        command->period_ms = 250u;
        return true;
    case ALARM_STATE_SENSOR_FAULT:
        /* 传感器故障使用独立闪烁模式，并保留蜂鸣器提示，但不驱动执行器。 */
        command->indicator = ALARM_OUTPUT_INDICATOR_FAULT_BLINK;
        command->buzzer_enabled = true;
        command->period_ms = 500u;
        return true;
    default:
        return false;
    }
}

const char *alarm_output_indicator_name(alarm_output_indicator_t indicator)
{
    switch (indicator) {
    case ALARM_OUTPUT_INDICATOR_OFF:
        return "off";
    case ALARM_OUTPUT_INDICATOR_SLOW_BLINK:
        return "slow_blink";
    case ALARM_OUTPUT_INDICATOR_FAST_BLINK:
        return "fast_blink";
    case ALARM_OUTPUT_INDICATOR_FAULT_BLINK:
        return "fault_blink";
    default:
        return "unknown";
    }
}
