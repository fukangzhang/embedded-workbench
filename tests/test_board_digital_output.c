#include <stdbool.h>

#include "embedded_workbench/board_digital_output.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_uint(unsigned int actual, unsigned int expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_level(digital_output_level_t actual, digital_output_level_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_pin_level(
    const board_digital_output_context_t *context,
    const board_pin_t *pin,
    digital_output_level_t expected)
{
    digital_output_level_t actual = DIGITAL_OUTPUT_LEVEL_LOW;

    if (!board_digital_output_get_level(context, pin, &actual)) {
        return 1;
    }

    return expect_level(actual, expected);
}

int main(void)
{
    const board_profile_t *profile = board_profile_default();
    board_digital_output_context_t context;
    digital_output_controller_t controller;
    board_pin_t copied_led_pin = profile->alarm_led;
    board_pin_t unknown_pin = {"PC", 13u, "unknown output"};
    digital_output_level_t level = DIGITAL_OUTPUT_LEVEL_HIGH;

    /* 初始化后，主机模拟后端的三个输出脚都应处于 LOW，且还没有写入动作。 */
    if (expect_true(board_digital_output_init(&controller, &context, profile)) != 0 ||
        expect_pin_level(&context, &profile->alarm_led, DIGITAL_OUTPUT_LEVEL_LOW) != 0 ||
        expect_pin_level(&context, &profile->alarm_buzzer, DIGITAL_OUTPUT_LEVEL_LOW) != 0 ||
        expect_pin_level(&context, &profile->alarm_actuator, DIGITAL_OUTPUT_LEVEL_LOW) != 0 ||
        expect_uint(context.write_count, 0u) != 0) {
        return 1;
    }

    /* 写 LED 和 buzzer 只应改变对应输出脚，actuator 保持 LOW。 */
    if (expect_true(digital_output_write(&controller, &profile->alarm_led, DIGITAL_OUTPUT_LEVEL_HIGH)) != 0 ||
        expect_true(digital_output_write(&controller, &profile->alarm_buzzer, DIGITAL_OUTPUT_LEVEL_HIGH)) != 0 ||
        expect_pin_level(&context, &profile->alarm_led, DIGITAL_OUTPUT_LEVEL_HIGH) != 0 ||
        expect_pin_level(&context, &profile->alarm_buzzer, DIGITAL_OUTPUT_LEVEL_HIGH) != 0 ||
        expect_pin_level(&context, &profile->alarm_actuator, DIGITAL_OUTPUT_LEVEL_LOW) != 0 ||
        expect_uint(context.write_count, 2u) != 0) {
        return 2;
    }

    /* 即使 pin 是 profile 中字段的拷贝，只要端口和编号一致，也应该匹配同一个输出。 */
    if (expect_true(digital_output_write(&controller, &copied_led_pin, DIGITAL_OUTPUT_LEVEL_LOW)) != 0 ||
        expect_pin_level(&context, &profile->alarm_led, DIGITAL_OUTPUT_LEVEL_LOW) != 0 ||
        expect_uint(context.write_count, 3u) != 0) {
        return 3;
    }

    /* 未知 pin 不能改变状态，也不能增加 write_count。 */
    if (expect_false(digital_output_write(&controller, &unknown_pin, DIGITAL_OUTPUT_LEVEL_HIGH)) != 0 ||
        expect_uint(context.write_count, 3u) != 0) {
        return 4;
    }

    if (expect_false(board_digital_output_get_level(&context, &unknown_pin, &level)) != 0 ||
        expect_false(board_digital_output_get_level(&context, &profile->alarm_led, 0)) != 0) {
        return 5;
    }

    if (expect_false(board_digital_output_init(0, &context, profile)) != 0 ||
        expect_false(board_digital_output_init(&controller, 0, profile)) != 0 ||
        expect_false(board_digital_output_init(&controller, &context, 0)) != 0) {
        return 6;
    }

    return 0;
}
