#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/stm32_rcc_usart_clock.h"

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
    volatile uint32_t apb1enr = 0u;
    stm32_rcc_usart_clock_peripheral_t peripherals[] = {
        {"USART2", 17u},
        {"USART3", 18u},
        {"USARTX", 32u},
    };
    stm32_rcc_usart_clock_context_t context;

    if (expect_true(stm32_rcc_usart_clock_init(&context, &apb1enr, peripherals, 3u)) != 0) {
        return 1;
    }

    /* STM32F401 上 USART2 常见于 APB1ENR bit17，这里用普通变量模拟 enable register。 */
    if (expect_true(stm32_rcc_enable_usart_clock(&context, "USART2")) != 0 ||
        expect_u32(apb1enr, 1u << 17u) != 0) {
        return 2;
    }

    if (expect_true(stm32_rcc_enable_usart_clock(&context, "USART3")) != 0 ||
        expect_u32(apb1enr, (1u << 17u) | (1u << 18u)) != 0) {
        return 3;
    }

    /* 重复开启同一个外设应该保持幂等。 */
    if (expect_true(stm32_rcc_enable_usart_clock(&context, "USART2")) != 0 ||
        expect_u32(apb1enr, (1u << 17u) | (1u << 18u)) != 0) {
        return 4;
    }

    /* 未知外设、非法 bit 和空名字都失败，而且不能改已有 bit。 */
    if (expect_false(stm32_rcc_enable_usart_clock(&context, "USART1")) != 0 ||
        expect_false(stm32_rcc_enable_usart_clock(&context, "USARTX")) != 0 ||
        expect_false(stm32_rcc_enable_usart_clock(&context, 0)) != 0 ||
        expect_u32(apb1enr, (1u << 17u) | (1u << 18u)) != 0) {
        return 5;
    }

    if (expect_false(stm32_rcc_usart_clock_init(0, &apb1enr, peripherals, 3u)) != 0 ||
        expect_false(stm32_rcc_usart_clock_init(&context, 0, peripherals, 3u)) != 0 ||
        expect_false(stm32_rcc_usart_clock_init(&context, &apb1enr, 0, 3u)) != 0 ||
        expect_false(stm32_rcc_usart_clock_init(&context, &apb1enr, peripherals, 0u)) != 0) {
        return 6;
    }

    return 0;
}
