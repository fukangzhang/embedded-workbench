#include "embedded_workbench/stm32_gpio_output.h"

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

static const stm32_gpio_output_port_t *find_port(
    const stm32_gpio_output_context_t *context,
    const char *port_name)
{
    size_t index = 0u;

    if (context == 0 || context->ports == 0 || port_name == 0) {
        return 0;
    }

    for (index = 0u; index < context->port_count; index++) {
        if (context->ports[index].bsrr != 0 &&
            text_equals(context->ports[index].port, port_name)) {
            return &context->ports[index];
        }
    }

    return 0;
}

static bool stm32_gpio_write(
    void *context,
    const board_pin_t *pin,
    digital_output_level_t level)
{
    stm32_gpio_output_context_t *gpio_context = (stm32_gpio_output_context_t *)context;
    const stm32_gpio_output_port_t *port = find_port(gpio_context, pin->port);
    uint32_t bit = 0u;

    if (port == 0 || pin->pin > 15u) {
        return false;
    }

    bit = 1u << pin->pin;

    /* STM32 GPIOx_BSRR 的低 16 位负责置位，高 16 位负责复位。
     * 这种写法是原子写，不需要先读 ODR 再改位，适合中断/RTOS 场景。 */
    if (level == DIGITAL_OUTPUT_LEVEL_HIGH) {
        *port->bsrr = bit;
    } else if (level == DIGITAL_OUTPUT_LEVEL_LOW) {
        *port->bsrr = bit << 16u;
    } else {
        return false;
    }

    return true;
}

bool stm32_gpio_output_init(
    digital_output_controller_t *controller,
    stm32_gpio_output_context_t *context,
    const stm32_gpio_output_port_t *ports,
    size_t port_count)
{
    static const digital_output_ops_t ops = {
        stm32_gpio_write,
    };

    if (controller == 0 || context == 0 || ports == 0 || port_count == 0u) {
        return false;
    }

    context->ports = ports;
    context->port_count = port_count;
    controller->ops = &ops;
    controller->context = context;

    return true;
}
