#include <stdint.h>
#include <string.h>

#include "embedded_workbench/stm32f401re_gpio_bindings.h"

static int expect_size(size_t actual, size_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_u8(uint8_t actual, uint8_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_uintptr(uintptr_t actual, uintptr_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

int main(void)
{
    size_t count = 0u;
    const stm32_rcc_gpio_clock_port_t *clock_ports = 0;
    const stm32_gpio_config_port_t *config_ports = 0;
    const stm32_gpio_output_port_t *output_ports = 0;

    if (expect_uintptr((uintptr_t)stm32f401re_rcc_ahb1enr(), STM32F401RE_RCC_AHB1ENR_ADDRESS) != 0) {
        return 1;
    }

    clock_ports = stm32f401re_gpio_clock_ports(&count);
    if (expect_size(count, 2u) != 0 ||
        expect_string(clock_ports[0].port, "PA") != 0 ||
        expect_u8(clock_ports[0].enable_bit, 0u) != 0 ||
        expect_string(clock_ports[1].port, "PB") != 0 ||
        expect_u8(clock_ports[1].enable_bit, 1u) != 0) {
        return 2;
    }

    config_ports = stm32f401re_gpio_config_ports(&count);
    if (expect_size(count, 2u) != 0 ||
        expect_string(config_ports[0].port, "PA") != 0 ||
        expect_uintptr((uintptr_t)config_ports[0].registers, STM32F401RE_GPIOA_BASE_ADDRESS) != 0 ||
        expect_string(config_ports[1].port, "PB") != 0 ||
        expect_uintptr((uintptr_t)config_ports[1].registers, STM32F401RE_GPIOB_BASE_ADDRESS) != 0) {
        return 3;
    }

    output_ports = stm32f401re_gpio_output_ports(&count);
    if (expect_size(count, 2u) != 0 ||
        expect_string(output_ports[0].port, "PA") != 0 ||
        expect_uintptr(
            (uintptr_t)output_ports[0].bsrr,
            STM32F401RE_GPIOA_BASE_ADDRESS + STM32F401RE_GPIO_BSRR_OFFSET) != 0 ||
        expect_string(output_ports[1].port, "PB") != 0 ||
        expect_uintptr(
            (uintptr_t)output_ports[1].bsrr,
            STM32F401RE_GPIOB_BASE_ADDRESS + STM32F401RE_GPIO_BSRR_OFFSET) != 0) {
        return 4;
    }

    count = 123u;
    (void)stm32f401re_gpio_clock_ports(0);
    (void)stm32f401re_gpio_config_ports(0);
    (void)stm32f401re_gpio_output_ports(0);
    if (expect_size(count, 123u) != 0) {
        return 5;
    }

    return 0;
}
