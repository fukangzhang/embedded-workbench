#include "embedded_workbench/stm32_rcc_usart_clock.h"

/* USART 外设也需要先打开 RCC clock，才能安全访问 USART 寄存器。 */

static bool text_equals(const char *left, const char *right)
{
    if (left == 0 || right == 0) {
        return false;
    }

    while (*left != '\0' && *right != '\0') {
        if (*left != *right) {
            return false;
        }
        left++;
        right++;
    }

    return *left == '\0' && *right == '\0';
}

static const stm32_rcc_usart_clock_peripheral_t *find_peripheral_clock(
    const stm32_rcc_usart_clock_context_t *context,
    const char *peripheral)
{
    size_t index = 0u;

    if (context == 0 || context->peripherals == 0 || peripheral == 0) {
        return 0;
    }

    for (index = 0u; index < context->peripheral_count; index++) {
        if (context->peripherals[index].enable_bit < 32u &&
            text_equals(context->peripherals[index].peripheral, peripheral)) {
            return &context->peripherals[index];
        }
    }

    return 0;
}

bool stm32_rcc_usart_clock_init(
    stm32_rcc_usart_clock_context_t *context,
    volatile uint32_t *enable_register,
    const stm32_rcc_usart_clock_peripheral_t *peripherals,
    size_t peripheral_count)
{
    if (context == 0 || enable_register == 0 || peripherals == 0 || peripheral_count == 0u) {
        return false;
    }

    context->enable_register = enable_register;
    context->peripherals = peripherals;
    context->peripheral_count = peripheral_count;

    return true;
}

bool stm32_rcc_enable_usart_clock(
    const stm32_rcc_usart_clock_context_t *context,
    const char *peripheral)
{
    const stm32_rcc_usart_clock_peripheral_t *clock = find_peripheral_clock(context, peripheral);

    if (context == 0 || context->enable_register == 0 || clock == 0) {
        return false;
    }

    *context->enable_register |= (1u << clock->enable_bit);

    return true;
}
