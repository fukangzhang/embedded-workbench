#ifndef EMBEDDED_WORKBENCH_STM32_RCC_GPIO_CLOCK_H
#define EMBEDDED_WORKBENCH_STM32_RCC_GPIO_CLOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char *port;
    uint8_t enable_bit;
} stm32_rcc_gpio_clock_port_t;

typedef struct {
    volatile uint32_t *enable_register;
    const stm32_rcc_gpio_clock_port_t *ports;
    size_t port_count;
} stm32_rcc_gpio_clock_context_t;

bool stm32_rcc_gpio_clock_init(
    stm32_rcc_gpio_clock_context_t *context,
    volatile uint32_t *enable_register,
    const stm32_rcc_gpio_clock_port_t *ports,
    size_t port_count);
bool stm32_rcc_enable_gpio_port_clock(
    const stm32_rcc_gpio_clock_context_t *context,
    const char *port);

#endif
