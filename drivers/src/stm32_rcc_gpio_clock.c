#include "embedded_workbench/stm32_rcc_gpio_clock.h"

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

static const stm32_rcc_gpio_clock_port_t *find_port_clock(
    const stm32_rcc_gpio_clock_context_t *context,
    const char *port)
{
    size_t index = 0u;

    if (context == 0 || context->ports == 0 || port == 0) {
        return 0;
    }

    for (index = 0u; index < context->port_count; index++) {
        if (context->ports[index].enable_bit < 32u &&
            text_equals(context->ports[index].port, port)) {
            return &context->ports[index];
        }
    }

    return 0;
}

bool stm32_rcc_gpio_clock_init(
    stm32_rcc_gpio_clock_context_t *context,
    volatile uint32_t *enable_register,
    const stm32_rcc_gpio_clock_port_t *ports,
    size_t port_count)
{
    if (context == 0 || enable_register == 0 || ports == 0 || port_count == 0u) {
        return false;
    }

    context->enable_register = enable_register;
    context->ports = ports;
    context->port_count = port_count;

    return true;
}

bool stm32_rcc_enable_gpio_port_clock(
    const stm32_rcc_gpio_clock_context_t *context,
    const char *port)
{
    const stm32_rcc_gpio_clock_port_t *clock = find_port_clock(context, port);

    if (context == 0 || context->enable_register == 0 || clock == 0) {
        return false;
    }

    /* STM32 的 GPIO 端口时钟通常在 RCC 的 enable register 中按 bit 打开。
     * 这里使用绑定表表达“端口名 -> bit”，避免把具体芯片地址写死在通用驱动里。 */
    *context->enable_register |= (1u << clock->enable_bit);

    return true;
}
