#include "embedded_workbench/alarm_output_timing.h"

/* alarm_output_timing 负责把“闪烁模式”变成某一时刻的开/关结果。
 * 这样状态策略和时间节拍分开，测试可以直接喂 elapsed_ms 覆盖边界。 */

static bool indicator_is_valid(alarm_output_indicator_t indicator)
{
    return indicator == ALARM_OUTPUT_INDICATOR_OFF ||
           indicator == ALARM_OUTPUT_INDICATOR_SLOW_BLINK ||
           indicator == ALARM_OUTPUT_INDICATOR_FAST_BLINK ||
           indicator == ALARM_OUTPUT_INDICATOR_FAULT_BLINK;
}

bool alarm_output_indicator_is_on(
    const alarm_output_command_t *command,
    uint32_t elapsed_ms,
    bool *is_on)
{
    uint32_t phase_ms = 0u;
    uint32_t on_window_ms = 0u;

    if (command == 0 || is_on == 0 || !indicator_is_valid(command->indicator)) {
        return false;
    }

    if (command->indicator == ALARM_OUTPUT_INDICATOR_OFF) {
        *is_on = false;
        return true;
    }

    /* period_ms 为 0 表示常亮。这样输出策略可以复用同一接口表达“闪烁”和“持续打开”。 */
    if (command->period_ms == 0u) {
        *is_on = true;
        return true;
    }

    /* 用取模得到当前闪烁周期内的位置，前半周期亮、后半周期灭。
     * +1 后再除以 2 可让奇数周期也尽量保持接近 50% 占空比。 */
    phase_ms = elapsed_ms % command->period_ms;
    on_window_ms = ((uint32_t)command->period_ms + 1u) / 2u;
    *is_on = phase_ms < on_window_ms;

    return true;
}
