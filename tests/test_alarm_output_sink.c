#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/alarm_output_sink.h"

typedef struct {
    /* fake sink 记录调用次数和最后写入值，让测试能观察 apply 的调用顺序。 */
    int indicator_calls;
    int buzzer_calls;
    int actuator_calls;
    alarm_output_indicator_t indicator;
    uint16_t period_ms;
    bool buzzer_enabled;
    bool actuator_enabled;
    bool fail_indicator;
    bool fail_buzzer;
    bool fail_actuator;
} fake_sink_context_t;

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_int(int actual, int expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_indicator(alarm_output_indicator_t actual, alarm_output_indicator_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_uint16(uint16_t actual, uint16_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_bool(bool actual, bool expected)
{
    return actual == expected ? 0 : 1;
}

static bool fake_set_indicator(
    void *context,
    alarm_output_indicator_t indicator,
    uint16_t period_ms)
{
    fake_sink_context_t *fake = (fake_sink_context_t *)context;

    fake->indicator_calls++;
    fake->indicator = indicator;
    fake->period_ms = period_ms;

    return !fake->fail_indicator;
}

static bool fake_set_buzzer(void *context, bool enabled)
{
    fake_sink_context_t *fake = (fake_sink_context_t *)context;

    fake->buzzer_calls++;
    fake->buzzer_enabled = enabled;

    return !fake->fail_buzzer;
}

static bool fake_set_actuator(void *context, bool enabled)
{
    fake_sink_context_t *fake = (fake_sink_context_t *)context;

    fake->actuator_calls++;
    fake->actuator_enabled = enabled;

    return !fake->fail_actuator;
}

int main(void)
{
    static const alarm_output_sink_ops_t ops = {
        fake_set_indicator,
        fake_set_buzzer,
        fake_set_actuator,
    };
    fake_sink_context_t fake = {0};
    alarm_output_sink_t sink = {&ops, &fake};
    alarm_output_command_t command;

    /* fake sink 记录每个输出函数的调用，测试 apply 是否完整传递了命令内容。 */
    command.indicator = ALARM_OUTPUT_INDICATOR_FAST_BLINK;
    command.buzzer_enabled = true;
    command.actuator_enabled = true;
    command.period_ms = 250u;

    if (expect_true(alarm_output_sink_apply(&sink, &command)) != 0 ||
        expect_int(fake.indicator_calls, 1) != 0 ||
        expect_indicator(fake.indicator, ALARM_OUTPUT_INDICATOR_FAST_BLINK) != 0 ||
        expect_uint16(fake.period_ms, 250u) != 0 ||
        expect_int(fake.buzzer_calls, 1) != 0 ||
        expect_bool(fake.buzzer_enabled, true) != 0 ||
        expect_int(fake.actuator_calls, 1) != 0 ||
        expect_bool(fake.actuator_enabled, true) != 0) {
        return 1;
    }

    if (expect_false(alarm_output_sink_apply(0, &command)) != 0 ||
        expect_false(alarm_output_sink_apply(&sink, 0)) != 0) {
        return 2;
    }

    /* 下面模拟第二步失败：indicator 已经调用，buzzer 失败后 actuator 不应再调用。 */
    fake.fail_buzzer = true;
    fake.indicator_calls = 0;
    fake.buzzer_calls = 0;
    fake.actuator_calls = 0;
    /* 蜂鸣器失败时执行器不应继续调用，验证 sink 的失败短路策略。 */
    if (expect_false(alarm_output_sink_apply(&sink, &command)) != 0 ||
        expect_int(fake.indicator_calls, 1) != 0 ||
        expect_int(fake.buzzer_calls, 1) != 0 ||
        expect_int(fake.actuator_calls, 0) != 0) {
        return 3;
    }

    return 0;
}
