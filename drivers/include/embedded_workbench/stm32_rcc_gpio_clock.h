#ifndef EMBEDDED_WORKBENCH_STM32_RCC_GPIO_CLOCK_H
#define EMBEDDED_WORKBENCH_STM32_RCC_GPIO_CLOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    /* port 是 "PA"/"PB"；enable_bit 是 RCC AHB1ENR 中控制该端口时钟的 bit。 */
    const char *port;
    uint8_t enable_bit;
} stm32_rcc_gpio_clock_port_t;

typedef struct {
    /* enable_register 指向 RCC 的 GPIO 时钟使能寄存器。 */
    volatile uint32_t *enable_register;
    /* 端口名到 enable bit 的查找表。 */
    const stm32_rcc_gpio_clock_port_t *ports;
    size_t port_count;
} stm32_rcc_gpio_clock_context_t;

/* 初始化 RCC GPIO clock 后端。
 * 本模块只设置 GPIO 端口时钟，不配置具体 pin。 */
bool stm32_rcc_gpio_clock_init(
    stm32_rcc_gpio_clock_context_t *context,
    volatile uint32_t *enable_register,
    const stm32_rcc_gpio_clock_port_t *ports,
    size_t port_count);
bool stm32_rcc_enable_gpio_port_clock(
    const stm32_rcc_gpio_clock_context_t *context,
    const char *port);

#endif
