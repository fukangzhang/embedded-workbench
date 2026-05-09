#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/alarm_output.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_indicator(alarm_output_indicator_t actual, alarm_output_indicator_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_bool(bool actual, bool expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_uint16(uint16_t actual, uint16_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

int main(void)
{
    alarm_output_command_t command;

    /* NORMAL 不应该产生任何告警输出，是最安全的默认策略。 */
    if (expect_true(alarm_output_command_for_state(ALARM_STATE_NORMAL, &command)) != 0 ||
        expect_indicator(command.indicator, ALARM_OUTPUT_INDICATOR_OFF) != 0 ||
        expect_bool(command.buzzer_enabled, false) != 0 ||
        expect_bool(command.actuator_enabled, false) != 0 ||
        expect_uint16(command.period_ms, 0u) != 0) {
        return 1;
    }

    /* WARNING 只慢闪提示，不打开蜂鸣器和执行器，避免轻微异常时动作过重。 */
    if (expect_true(alarm_output_command_for_state(ALARM_STATE_WARNING, &command)) != 0 ||
        expect_indicator(command.indicator, ALARM_OUTPUT_INDICATOR_SLOW_BLINK) != 0 ||
        expect_bool(command.buzzer_enabled, false) != 0 ||
        expect_bool(command.actuator_enabled, false) != 0 ||
        expect_uint16(command.period_ms, 1000u) != 0) {
        return 2;
    }

    /* ALARM 是强告警：快闪、蜂鸣器、执行器都要打开。 */
    if (expect_true(alarm_output_command_for_state(ALARM_STATE_ALARM, &command)) != 0 ||
        expect_indicator(command.indicator, ALARM_OUTPUT_INDICATOR_FAST_BLINK) != 0 ||
        expect_bool(command.buzzer_enabled, true) != 0 ||
        expect_bool(command.actuator_enabled, true) != 0 ||
        expect_uint16(command.period_ms, 250u) != 0) {
        return 3;
    }

    /* SENSOR_FAULT 和真实环境告警区分：提示故障，但不驱动执行器。 */
    if (expect_true(alarm_output_command_for_state(ALARM_STATE_SENSOR_FAULT, &command)) != 0 ||
        expect_indicator(command.indicator, ALARM_OUTPUT_INDICATOR_FAULT_BLINK) != 0 ||
        expect_bool(command.buzzer_enabled, true) != 0 ||
        expect_bool(command.actuator_enabled, false) != 0 ||
        expect_uint16(command.period_ms, 500u) != 0) {
        return 4;
    }

    /* 未知状态和空输出指针都必须失败，不能留下半初始化 command。 */
    if (expect_false(alarm_output_command_for_state((alarm_state_t)99, &command)) != 0) {
        return 5;
    }

    if (expect_false(alarm_output_command_for_state(ALARM_STATE_NORMAL, 0)) != 0) {
        return 6;
    }

    /* 输出名字会进入响应文本，测试固定这些字符串。 */
    if (expect_string(alarm_output_indicator_name(ALARM_OUTPUT_INDICATOR_FAST_BLINK), "fast_blink") != 0 ||
        expect_string(alarm_output_indicator_name((alarm_output_indicator_t)99), "unknown") != 0) {
        return 7;
    }

    return 0;
}
