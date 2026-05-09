#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/stm32_gpio_config.h"

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

static uint32_t with_four_bit_field(uint32_t base, unsigned int field, uint32_t value)
{
    uint32_t shift = field * 4u;
    uint32_t mask = 15u << shift;

    return (base & ~mask) | ((value & 15u) << shift);
}

int main(void)
{
    /* gpioa 初始为全 1，gpiob 初始为全 0，用来确认配置函数只改目标 pin 的字段。 */
    stm32_gpio_registers_t gpioa = {0xffffffffu, 0xffffffffu, 0xffffffffu, 0xffffffffu};
    stm32_gpio_registers_t gpiob = {0u, 0u, 0u, 0u};
    stm32_gpio_config_port_t ports[] = {
        {"PA", &gpioa},
        {"PB", &gpiob},
    };
    stm32_gpio_config_context_t context;
    stm32_gpio_output_config_t config = stm32_gpio_output_config_default();
    board_pin_t led_pin = {"PA", 5u, "LD2 user LED / alarm indicator"};
    board_pin_t buzzer_pin = {"PB", 6u, "external buzzer placeholder"};
    board_pin_t unknown_port_pin = {"PC", 13u, "unknown output"};
    board_pin_t invalid_pin = {"PA", 16u, "invalid output"};
    board_pin_t usart2_tx_pin = {"PA", 2u, "USART2 TX"};
    board_pin_t usart1_tx_pin = {"PA", 9u, "USART1 TX"};
    stm32_gpio_alternate_config_t alternate_config = stm32_gpio_alternate_config_default(7u);
    uint32_t gpioa_moder_before_error = 0u;
    uint32_t gpioa_otyper_before_error = 0u;
    uint32_t gpioa_ospeedr_before_error = 0u;
    uint32_t gpioa_pupdr_before_error = 0u;
    uint32_t gpioa_afrl_before_error = 0u;
    uint32_t gpioa_afrh_before_error = 0u;

    if (expect_true(stm32_gpio_config_init(&context, ports, 2u)) != 0) {
        return 1;
    }

    config.output_type = STM32_GPIO_OUTPUT_PUSH_PULL;
    config.speed = STM32_GPIO_SPEED_HIGH;
    config.pull = STM32_GPIO_PULL_UP;
    /* PA5 验证推挽/高速/上拉如何写入 MODER/OTYPER/OSPEEDR/PUPDR。 */
    if (expect_true(stm32_gpio_configure_output(&context, &led_pin, &config)) != 0 ||
        expect_u32(gpioa.moder, with_two_bit_field(0xffffffffu, 5u, 1u)) != 0 ||
        expect_u32(gpioa.otyper, with_one_bit_field(0xffffffffu, 5u, false)) != 0 ||
        expect_u32(gpioa.ospeedr, with_two_bit_field(0xffffffffu, 5u, 2u)) != 0 ||
        expect_u32(gpioa.pupdr, with_two_bit_field(0xffffffffu, 5u, 1u)) != 0) {
        return 2;
    }

    config.output_type = STM32_GPIO_OUTPUT_OPEN_DRAIN;
    config.speed = STM32_GPIO_SPEED_VERY_HIGH;
    config.pull = STM32_GPIO_PULL_DOWN;
    /* PB6 验证开漏/最高速/下拉这一组配置。 */
    if (expect_true(stm32_gpio_configure_output(&context, &buzzer_pin, &config)) != 0 ||
        expect_u32(gpiob.moder, with_two_bit_field(0u, 6u, 1u)) != 0 ||
        expect_u32(gpiob.otyper, with_one_bit_field(0u, 6u, true)) != 0 ||
        expect_u32(gpiob.ospeedr, with_two_bit_field(0u, 6u, 3u)) != 0 ||
        expect_u32(gpiob.pupdr, with_two_bit_field(0u, 6u, 2u)) != 0) {
        return 3;
    }

    alternate_config.output_type = STM32_GPIO_OUTPUT_PUSH_PULL;
    alternate_config.speed = STM32_GPIO_SPEED_VERY_HIGH;
    alternate_config.pull = STM32_GPIO_PULL_UP;
    /* PA2 使用 AFRL，AF7 是 STM32F401 上 USART2_TX/USART2_RX 常用 alternate function。 */
    if (expect_true(stm32_gpio_configure_alternate_function(&context, &usart2_tx_pin, &alternate_config)) != 0 ||
        expect_u32(gpioa.moder, with_two_bit_field(with_two_bit_field(0xffffffffu, 5u, 1u), 2u, 2u)) != 0 ||
        expect_u32(gpioa.otyper, with_one_bit_field(with_one_bit_field(0xffffffffu, 5u, false), 2u, false)) != 0 ||
        expect_u32(gpioa.ospeedr, with_two_bit_field(with_two_bit_field(0xffffffffu, 5u, 2u), 2u, 3u)) != 0 ||
        expect_u32(gpioa.pupdr, with_two_bit_field(with_two_bit_field(0xffffffffu, 5u, 1u), 2u, 1u)) != 0 ||
        expect_u32(gpioa.afrl, with_four_bit_field(0u, 2u, 7u)) != 0) {
        return 4;
    }

    alternate_config.alternate_function = 7u;
    alternate_config.pull = STM32_GPIO_PULL_NONE;
    /* PA9 使用 AFRH，验证 pin8..15 的 AF 字段会写到另一组寄存器。 */
    if (expect_true(stm32_gpio_configure_alternate_function(&context, &usart1_tx_pin, &alternate_config)) != 0 ||
        expect_u32(gpioa.afrh, with_four_bit_field(0u, 1u, 7u)) != 0) {
        return 5;
    }

    gpioa_moder_before_error = gpioa.moder;
    gpioa_otyper_before_error = gpioa.otyper;
    gpioa_ospeedr_before_error = gpioa.ospeedr;
    gpioa_pupdr_before_error = gpioa.pupdr;
    gpioa_afrl_before_error = gpioa.afrl;
    gpioa_afrh_before_error = gpioa.afrh;

    /* 失败路径不能改动已经存在的寄存器值。 */
    if (expect_false(stm32_gpio_configure_output(&context, &unknown_port_pin, &config)) != 0 ||
        expect_false(stm32_gpio_configure_output(&context, &invalid_pin, &config)) != 0 ||
        expect_false(stm32_gpio_configure_output(&context, &led_pin, 0)) != 0 ||
        expect_u32(gpioa.moder, gpioa_moder_before_error) != 0 ||
        expect_u32(gpioa.otyper, gpioa_otyper_before_error) != 0 ||
        expect_u32(gpioa.ospeedr, gpioa_ospeedr_before_error) != 0 ||
        expect_u32(gpioa.pupdr, gpioa_pupdr_before_error) != 0 ||
        expect_u32(gpioa.afrl, gpioa_afrl_before_error) != 0 ||
        expect_u32(gpioa.afrh, gpioa_afrh_before_error) != 0) {
        return 6;
    }

    config.pull = (stm32_gpio_pull_t)99;
    if (expect_false(stm32_gpio_configure_output(&context, &led_pin, &config)) != 0 ||
        expect_u32(gpioa.moder, gpioa_moder_before_error) != 0 ||
        expect_u32(gpioa.otyper, gpioa_otyper_before_error) != 0 ||
        expect_u32(gpioa.ospeedr, gpioa_ospeedr_before_error) != 0 ||
        expect_u32(gpioa.pupdr, gpioa_pupdr_before_error) != 0 ||
        expect_u32(gpioa.afrl, gpioa_afrl_before_error) != 0 ||
        expect_u32(gpioa.afrh, gpioa_afrh_before_error) != 0) {
        return 7;
    }

    alternate_config.alternate_function = 16u;
    if (expect_false(stm32_gpio_configure_alternate_function(&context, &usart2_tx_pin, &alternate_config)) != 0 ||
        expect_false(stm32_gpio_configure_alternate_function(&context, &invalid_pin, &alternate_config)) != 0 ||
        expect_false(stm32_gpio_configure_alternate_function(&context, &unknown_port_pin, &alternate_config)) != 0 ||
        expect_u32(gpioa.afrl, gpioa_afrl_before_error) != 0 ||
        expect_u32(gpioa.afrh, gpioa_afrh_before_error) != 0) {
        return 8;
    }

    if (expect_false(stm32_gpio_config_init(0, ports, 2u)) != 0 ||
        expect_false(stm32_gpio_config_init(&context, 0, 2u)) != 0 ||
        expect_false(stm32_gpio_config_init(&context, ports, 0u)) != 0) {
        return 9;
    }

    return 0;
}
