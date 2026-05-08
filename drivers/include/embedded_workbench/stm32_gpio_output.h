#ifndef EMBEDDED_WORKBENCH_STM32_GPIO_OUTPUT_H
#define EMBEDDED_WORKBENCH_STM32_GPIO_OUTPUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "embedded_workbench/digital_output.h"

typedef struct {
    const char *port;
    volatile uint32_t *bsrr;
} stm32_gpio_output_port_t;

typedef struct {
    const stm32_gpio_output_port_t *ports;
    size_t port_count;
} stm32_gpio_output_context_t;

bool stm32_gpio_output_init(
    digital_output_controller_t *controller,
    stm32_gpio_output_context_t *context,
    const stm32_gpio_output_port_t *ports,
    size_t port_count);

#endif
