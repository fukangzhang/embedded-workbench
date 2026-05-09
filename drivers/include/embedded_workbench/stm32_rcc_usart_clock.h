#ifndef EMBEDDED_WORKBENCH_STM32_RCC_USART_CLOCK_H
#define EMBEDDED_WORKBENCH_STM32_RCC_USART_CLOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    /* peripheral 是 "USART2" 这类名字；enable_bit 是 RCC APB enable register 中的 bit。 */
    const char *peripheral;
    uint8_t enable_bit;
} stm32_rcc_usart_clock_peripheral_t;

typedef struct {
    /* enable_register 指向 RCC 的 USART 所在 APB enable register。 */
    volatile uint32_t *enable_register;
    const stm32_rcc_usart_clock_peripheral_t *peripherals;
    size_t peripheral_count;
} stm32_rcc_usart_clock_context_t;

bool stm32_rcc_usart_clock_init(
    stm32_rcc_usart_clock_context_t *context,
    volatile uint32_t *enable_register,
    const stm32_rcc_usart_clock_peripheral_t *peripherals,
    size_t peripheral_count);
bool stm32_rcc_enable_usart_clock(
    const stm32_rcc_usart_clock_context_t *context,
    const char *peripheral);

#endif
