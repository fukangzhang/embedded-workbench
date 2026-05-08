#include "embedded_workbench/stm32f401re_gpio_bindings.h"

static const stm32_rcc_gpio_clock_port_t gpio_clock_ports[] = {
    /* AHB1ENR bit0 控制 GPIOA 时钟，bit1 控制 GPIOB 时钟。 */
    {"PA", 0u},
    {"PB", 1u},
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

const stm32_rcc_gpio_clock_port_t *stm32f401re_gpio_clock_ports(size_t *count)
{
    if (count != 0) {
        *count = sizeof(gpio_clock_ports) / sizeof(gpio_clock_ports[0]);
    }

    return gpio_clock_ports;
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
