#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/stm32_rcc_gpio_clock.h"

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
    /* ahb1enr 用普通变量模拟 RCC AHB1ENR；测试只看对应 bit 是否被置位。 */
    volatile uint32_t ahb1enr = 0u;
    stm32_rcc_gpio_clock_port_t ports[] = {
        {"PA", 0u},
        {"PB", 1u},
        {"PC", 2u},
        {"PZ", 32u},
    };
    stm32_rcc_gpio_clock_context_t context;

    if (expect_true(stm32_rcc_gpio_clock_init(&context, &ahb1enr, ports, 4u)) != 0) {
        return 1;
    }

    /* PA/PB 分别打开 bit0/bit1。 */
    if (expect_true(stm32_rcc_enable_gpio_port_clock(&context, "PA")) != 0 ||
        expect_u32(ahb1enr, 1u << 0u) != 0) {
        return 2;
    }

    if (expect_true(stm32_rcc_enable_gpio_port_clock(&context, "PB")) != 0 ||
        expect_u32(ahb1enr, (1u << 0u) | (1u << 1u)) != 0) {
        return 3;
    }

    /* 重复开启同一个端口应该保持幂等，不清掉已经开启的其他端口。 */
    if (expect_true(stm32_rcc_enable_gpio_port_clock(&context, "PA")) != 0 ||
        expect_u32(ahb1enr, (1u << 0u) | (1u << 1u)) != 0) {
        return 4;
    }

    /* 未知端口、非法 bit 和空端口名都失败，且不能改变已有 enable bits。 */
    if (expect_false(stm32_rcc_enable_gpio_port_clock(&context, "PD")) != 0 ||
        expect_false(stm32_rcc_enable_gpio_port_clock(&context, "PZ")) != 0 ||
        expect_false(stm32_rcc_enable_gpio_port_clock(&context, 0)) != 0 ||
        expect_u32(ahb1enr, (1u << 0u) | (1u << 1u)) != 0) {
        return 5;
    }

    if (expect_false(stm32_rcc_gpio_clock_init(0, &ahb1enr, ports, 4u)) != 0 ||
        expect_false(stm32_rcc_gpio_clock_init(&context, 0, ports, 4u)) != 0 ||
        expect_false(stm32_rcc_gpio_clock_init(&context, &ahb1enr, 0, 4u)) != 0 ||
        expect_false(stm32_rcc_gpio_clock_init(&context, &ahb1enr, ports, 0u)) != 0) {
        return 6;
    }

    return 0;
}
