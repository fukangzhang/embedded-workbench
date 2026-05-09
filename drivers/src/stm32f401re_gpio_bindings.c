#include "embedded_workbench/stm32f401re_gpio_bindings.h"

/* 这个文件只绑定 STM32F401RE 的真实地址和端口表。
 * 通用 STM32 驱动通过这些表工作，因此驱动本身不用写死某一颗芯片的地址。 */

static const stm32_rcc_gpio_clock_port_t gpio_clock_ports[] = {
    /* AHB1ENR bit0 控制 GPIOA 时钟，bit1 控制 GPIOB 时钟。 */
    {"PA", 0u},
    {"PB", 1u},
};

static const stm32_rcc_usart_clock_peripheral_t usart_clock_peripherals[] = {
    /* APB1ENR bit17 controls USART2 clock on STM32F401RE. */
    {"USART2", STM32F401RE_USART2_APB1ENR_BIT},
};

static const stm32_gpio_config_port_t gpio_config_ports[] = {
    /* 这些地址会被当作 stm32_gpio_registers_t 使用：
     * 结构体字段顺序必须和芯片寄存器布局中我们关心的部分一致。 */
    {"PA", (stm32_gpio_registers_t *)STM32F401RE_GPIOA_BASE_ADDRESS},
    {"PB", (stm32_gpio_registers_t *)STM32F401RE_GPIOB_BASE_ADDRESS},
};

static const stm32_gpio_output_port_t gpio_output_ports[] = {
    /* 输出高低电平时只需要 BSRR 寄存器，所以这里绑定到 base + BSRR offset。 */
    {"PA", (volatile uint32_t *)(STM32F401RE_GPIOA_BASE_ADDRESS + STM32F401RE_GPIO_BSRR_OFFSET)},
    {"PB", (volatile uint32_t *)(STM32F401RE_GPIOB_BASE_ADDRESS + STM32F401RE_GPIO_BSRR_OFFSET)},
};

volatile uint32_t *stm32f401re_rcc_ahb1enr(void)
{
    /* 返回的是内存映射寄存器地址。不要在主机程序里解引用它。 */
    return (volatile uint32_t *)STM32F401RE_RCC_AHB1ENR_ADDRESS;
}

volatile uint32_t *stm32f401re_rcc_apb1enr(void)
{
    return (volatile uint32_t *)STM32F401RE_RCC_APB1ENR_ADDRESS;
}

const stm32_rcc_gpio_clock_port_t *stm32f401re_gpio_clock_ports(size_t *count)
{
    if (count != 0) {
        *count = sizeof(gpio_clock_ports) / sizeof(gpio_clock_ports[0]);
    }

    return gpio_clock_ports;
}

const stm32_rcc_usart_clock_peripheral_t *stm32f401re_usart_clock_peripherals(size_t *count)
{
    if (count != 0) {
        *count = sizeof(usart_clock_peripherals) / sizeof(usart_clock_peripherals[0]);
    }

    return usart_clock_peripherals;
}

const stm32_gpio_config_port_t *stm32f401re_gpio_config_ports(size_t *count)
{
    if (count != 0) {
        *count = sizeof(gpio_config_ports) / sizeof(gpio_config_ports[0]);
    }

    return gpio_config_ports;
}

const stm32_gpio_output_port_t *stm32f401re_gpio_output_ports(size_t *count)
{
    if (count != 0) {
        *count = sizeof(gpio_output_ports) / sizeof(gpio_output_ports[0]);
    }

    return gpio_output_ports;
}

stm32_usart_registers_t *stm32f401re_usart2_registers(void)
{
    return (stm32_usart_registers_t *)STM32F401RE_USART2_BASE_ADDRESS;
}

board_pin_t stm32f401re_usart2_tx_pin(void)
{
    board_pin_t pin = {"PA", 2u, "USART2 TX"};

    return pin;
}

board_pin_t stm32f401re_usart2_rx_pin(void)
{
    board_pin_t pin = {"PA", 3u, "USART2 RX"};

    return pin;
}
