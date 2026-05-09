#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/stm32_board_gpio_init.h"

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

static uint32_t with_two_bit_field(uint32_t base, unsigned int pin, uint32_t value)
{
    uint32_t shift = pin * 2u;
    uint32_t mask = 3u << shift;

    return (base & ~mask) | ((value & 3u) << shift);
}

static uint32_t with_one_bit_field(uint32_t base, unsigned int pin, bool enabled)
{
    uint32_t mask = 1u << pin;

    return enabled ? (base | mask) : (base & ~mask);
}

int main(void)
{
    /* 这个测试把 RCC 模拟寄存器和 GPIO 模拟寄存器串起来，
     * 验证 board_profile 的三个告警输出脚能被统一初始化。 */
    const board_profile_t *profile = board_profile_default();
    volatile uint32_t ahb1enr = 0u;
    stm32_gpio_registers_t gpioa = {0u, 0u, 0u, 0u};
    stm32_gpio_registers_t gpiob = {0u, 0u, 0u, 0u};
    stm32_rcc_gpio_clock_port_t clock_ports[] = {
        {"PA", 0u},
        {"PB", 1u},
    };
    stm32_gpio_config_port_t gpio_ports[] = {
        {"PA", &gpioa},
        {"PB", &gpiob},
    };
    stm32_rcc_gpio_clock_context_t clock_context;
    stm32_gpio_config_context_t gpio_context;
    stm32_gpio_output_config_t output_config = stm32_gpio_output_config_default();
    board_profile_t missing_buzzer_clock_profile = *profile;

    if (expect_true(stm32_rcc_gpio_clock_init(&clock_context, &ahb1enr, clock_ports, 2u)) != 0 ||
        expect_true(stm32_gpio_config_init(&gpio_context, gpio_ports, 2u)) != 0) {
        return 1;
    }

    output_config.output_type = STM32_GPIO_OUTPUT_OPEN_DRAIN;
    output_config.speed = STM32_GPIO_SPEED_HIGH;
    output_config.pull = STM32_GPIO_PULL_UP;
    /* 显式配置路径：三个输出脚都应该打开端口时钟并写入指定 GPIO 配置。 */
    if (expect_true(stm32_board_gpio_init_alarm_outputs(
            profile,
            &clock_context,
            &gpio_context,
            &output_config)) != 0 ||
        expect_u32(ahb1enr, (1u << 0u) | (1u << 1u)) != 0 ||
        expect_u32(gpioa.moder, with_two_bit_field(0u, 5u, 1u)) != 0 ||
        expect_u32(gpioa.otyper, with_one_bit_field(0u, 5u, true)) != 0 ||
        expect_u32(gpioa.ospeedr, with_two_bit_field(0u, 5u, 2u)) != 0 ||
        expect_u32(gpioa.pupdr, with_two_bit_field(0u, 5u, 1u)) != 0 ||
        expect_u32(gpiob.moder, with_two_bit_field(with_two_bit_field(0u, 6u, 1u), 7u, 1u)) != 0 ||
        expect_u32(gpiob.otyper, with_one_bit_field(with_one_bit_field(0u, 6u, true), 7u, true)) != 0 ||
        expect_u32(gpiob.ospeedr, with_two_bit_field(with_two_bit_field(0u, 6u, 2u), 7u, 2u)) != 0 ||
        expect_u32(gpiob.pupdr, with_two_bit_field(with_two_bit_field(0u, 6u, 1u), 7u, 1u)) != 0) {
        return 2;
    }

    ahb1enr = 0u;
    gpioa.moder = 0u;
    gpioa.otyper = 0u;
    gpioa.ospeedr = 0u;
    gpioa.pupdr = 0u;
    gpiob.moder = 0u;
    gpiob.otyper = 0u;
    gpiob.ospeedr = 0u;
    gpiob.pupdr = 0u;

    /* 传入空配置时使用默认 push-pull、low-speed、no-pull 输出配置。 */
    if (expect_true(stm32_board_gpio_init_alarm_outputs(
            profile,
            &clock_context,
            &gpio_context,
            0)) != 0 ||
        expect_u32(gpioa.moder, with_two_bit_field(0u, 5u, 1u)) != 0 ||
        expect_u32(gpioa.otyper, 0u) != 0 ||
        expect_u32(gpioa.ospeedr, 0u) != 0 ||
        expect_u32(gpioa.pupdr, 0u) != 0) {
        return 3;
    }

    missing_buzzer_clock_profile.alarm_buzzer.port = "PC";
    ahb1enr = 0u;
    gpioa.moder = 0u;
    gpiob.moder = 0u;
    /* 第二个输出脚缺少 RCC/GPIO 绑定时，流程应失败；
     * 但第一个已经初始化的 LED 模拟寄存器会保留已完成的动作。 */
    if (expect_false(stm32_board_gpio_init_alarm_outputs(
            &missing_buzzer_clock_profile,
            &clock_context,
            &gpio_context,
            &output_config)) != 0 ||
        expect_u32(ahb1enr, 1u << 0u) != 0 ||
        expect_u32(gpioa.moder, with_two_bit_field(0u, 5u, 1u)) != 0 ||
        expect_u32(gpiob.moder, 0u) != 0) {
        return 4;
    }

    if (expect_false(stm32_board_gpio_init_alarm_outputs(0, &clock_context, &gpio_context, &output_config)) != 0 ||
        expect_false(stm32_board_gpio_init_alarm_outputs(profile, 0, &gpio_context, &output_config)) != 0 ||
        expect_false(stm32_board_gpio_init_alarm_outputs(profile, &clock_context, 0, &output_config)) != 0) {
        return 5;
    }

    output_config.pull = (stm32_gpio_pull_t)99;
    if (expect_false(stm32_board_gpio_init_alarm_outputs(
            profile,
            &clock_context,
            &gpio_context,
            &output_config)) != 0) {
        return 6;
    }

    return 0;
}
