#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/stm32_gpio_output.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_u32(uint32_t actual, uint32_t expected)
{
    return actual == expected ? 0 : 1;
}

int main(void)
{
    volatile uint32_t gpioa_bsrr = 0u;
    volatile uint32_t gpiob_bsrr = 0u;
    static const board_pin_t led_pin = {"PA", 5u, "LD2 user LED / alarm indicator"};
    static const board_pin_t buzzer_pin = {"PB", 6u, "external buzzer placeholder"};
    static const board_pin_t unknown_port_pin = {"PC", 13u, "unknown output"};
    static const board_pin_t invalid_pin = {"PA", 16u, "invalid output"};
    stm32_gpio_output_port_t ports[] = {
        {"PA", &gpioa_bsrr},
        {"PB", &gpiob_bsrr},
    };
    stm32_gpio_output_context_t context;
    digital_output_controller_t controller;

    if (expect_true(stm32_gpio_output_init(&controller, &context, ports, 2u)) != 0) {
        return 1;
    }

    if (expect_true(digital_output_write(&controller, &led_pin, DIGITAL_OUTPUT_LEVEL_HIGH)) != 0 ||
        expect_u32(gpioa_bsrr, (uint32_t)(1u << 5u)) != 0 ||
        expect_u32(gpiob_bsrr, 0u) != 0) {
        return 2;
    }

    if (expect_true(digital_output_write(&controller, &led_pin, DIGITAL_OUTPUT_LEVEL_LOW)) != 0 ||
        expect_u32(gpioa_bsrr, (uint32_t)(1u << (5u + 16u))) != 0) {
        return 3;
    }

    if (expect_true(digital_output_write(&controller, &buzzer_pin, DIGITAL_OUTPUT_LEVEL_HIGH)) != 0 ||
        expect_u32(gpiob_bsrr, (uint32_t)(1u << 6u)) != 0) {
        return 4;
    }

    /* 未绑定端口和非法 pin 不能写寄存器，避免把错误 profile 映射成硬件动作。 */
    if (expect_false(digital_output_write(&controller, &unknown_port_pin, DIGITAL_OUTPUT_LEVEL_HIGH)) != 0 ||
        expect_false(digital_output_write(&controller, &invalid_pin, DIGITAL_OUTPUT_LEVEL_HIGH)) != 0 ||
        expect_u32(gpioa_bsrr, (uint32_t)(1u << (5u + 16u))) != 0 ||
        expect_u32(gpiob_bsrr, (uint32_t)(1u << 6u)) != 0) {
        return 5;
    }

    if (expect_false(stm32_gpio_output_init(0, &context, ports, 2u)) != 0 ||
        expect_false(stm32_gpio_output_init(&controller, 0, ports, 2u)) != 0 ||
        expect_false(stm32_gpio_output_init(&controller, &context, 0, 2u)) != 0 ||
        expect_false(stm32_gpio_output_init(&controller, &context, ports, 0u)) != 0) {
        return 6;
    }

    return 0;
}
