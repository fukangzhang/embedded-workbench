#ifndef EMBEDDED_WORKBENCH_STM32_GPIO_OUTPUT_H
#define EMBEDDED_WORKBENCH_STM32_GPIO_OUTPUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "embedded_workbench/digital_output.h"

typedef struct {
    /* port 是 "PA"/"PB" 这类名字；bsrr 指向该端口的 GPIOx_BSRR 寄存器。 */
    const char *port;
    volatile uint32_t *bsrr;
} stm32_gpio_output_port_t;

typedef struct {
    /* 端口名到 BSRR 寄存器地址的查找表。 */
    const stm32_gpio_output_port_t *ports;
    size_t port_count;
} stm32_gpio_output_context_t;

/* 初始化 STM32 GPIO 输出后端。
 * 初始化后 digital_output_write 会通过 GPIOx_BSRR 原子置位/复位 pin。 */
bool stm32_gpio_output_init(
    digital_output_controller_t *controller,
    stm32_gpio_output_context_t *context,
    const stm32_gpio_output_port_t *ports,
    size_t port_count);

#endif
