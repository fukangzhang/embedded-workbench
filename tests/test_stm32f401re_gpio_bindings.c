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
    const stm32_rcc_usart_clock_peripheral_t *usart_clocks = 0;
    const stm32_gpio_config_port_t *config_ports = 0;
    const stm32_gpio_output_port_t *output_ports = 0;
    board_pin_t usart2_tx = stm32f401re_usart2_tx_pin();
    board_pin_t usart2_rx = stm32f401re_usart2_rx_pin();

    /* 这个测试运行在开发机上，不在 STM32 上。
     * 因此只能把指针转成整数比较地址，绝不能写 *stm32f401re_rcc_ahb1enr()。 */
    if (expect_uintptr((uintptr_t)stm32f401re_rcc_ahb1enr(), STM32F401RE_RCC_AHB1ENR_ADDRESS) != 0) {
        return 1;
    }

    if (expect_uintptr((uintptr_t)stm32f401re_rcc_apb1enr(), STM32F401RE_RCC_APB1ENR_ADDRESS) != 0 ||
        expect_uintptr((uintptr_t)stm32f401re_usart2_registers(), STM32F401RE_USART2_BASE_ADDRESS) != 0) {
        return 2;
    }

    clock_ports = stm32f401re_gpio_clock_ports(&count);
    if (expect_size(count, 2u) != 0 ||
        expect_string(clock_ports[0].port, "PA") != 0 ||
        expect_u8(clock_ports[0].enable_bit, 0u) != 0 ||
        expect_string(clock_ports[1].port, "PB") != 0 ||
        expect_u8(clock_ports[1].enable_bit, 1u) != 0) {
        return 3;
    }

    usart_clocks = stm32f401re_usart_clock_peripherals(&count);
    if (expect_size(count, 1u) != 0 ||
        expect_string(usart_clocks[0].peripheral, "USART2") != 0 ||
        expect_u8(usart_clocks[0].enable_bit, STM32F401RE_USART2_APB1ENR_BIT) != 0) {
        return 4;
    }

    config_ports = stm32f401re_gpio_config_ports(&count);
    if (expect_size(count, 2u) != 0 ||
        expect_string(config_ports[0].port, "PA") != 0 ||
        expect_uintptr((uintptr_t)config_ports[0].registers, STM32F401RE_GPIOA_BASE_ADDRESS) != 0 ||
        expect_string(config_ports[1].port, "PB") != 0 ||
        expect_uintptr((uintptr_t)config_ports[1].registers, STM32F401RE_GPIOB_BASE_ADDRESS) != 0) {
        return 5;
    }

    output_ports = stm32f401re_gpio_output_ports(&count);
    /* BSRR 地址来自 GPIO base + BSRR offset；这里同样只比较数值，不解引用。 */
    if (expect_size(count, 2u) != 0 ||
        expect_string(output_ports[0].port, "PA") != 0 ||
        expect_uintptr(
            (uintptr_t)output_ports[0].bsrr,
            STM32F401RE_GPIOA_BASE_ADDRESS + STM32F401RE_GPIO_BSRR_OFFSET) != 0 ||
        expect_string(output_ports[1].port, "PB") != 0 ||
        expect_uintptr(
            (uintptr_t)output_ports[1].bsrr,
            STM32F401RE_GPIOB_BASE_ADDRESS + STM32F401RE_GPIO_BSRR_OFFSET) != 0) {
        return 6;
    }

    if (expect_string(usart2_tx.port, "PA") != 0 ||
        expect_u8(usart2_tx.pin, 2u) != 0 ||
        expect_string(usart2_rx.port, "PA") != 0 ||
        expect_u8(usart2_rx.pin, 3u) != 0 ||
        expect_u8(STM32F401RE_USART2_GPIO_AF, 7u) != 0) {
        return 7;
    }

    /* count 参数允许传 0；这里确认传 0 不会意外改动调用者自己的变量。 */
    count = 123u;
    (void)stm32f401re_gpio_clock_ports(0);
    (void)stm32f401re_usart_clock_peripherals(0);
    (void)stm32f401re_gpio_config_ports(0);
    (void)stm32f401re_gpio_output_ports(0);
    if (expect_size(count, 123u) != 0) {
        return 8;
    }

    return 0;
}
