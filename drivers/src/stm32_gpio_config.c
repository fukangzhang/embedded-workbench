#include "embedded_workbench/stm32_gpio_config.h"

/* stm32_gpio_config 负责把 board_pin_t 配成 STM32 GPIO 输出模式。
 * 它写的是 MODER/OTYPER/OSPEEDR/PUPDR 这类配置寄存器，不负责后续输出高低电平。 */

#define STM32_GPIO_MODE_OUTPUT 1u
#define STM32_GPIO_TWO_BIT_MASK 3u

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

static bool output_type_is_valid(stm32_gpio_output_type_t output_type)
{
    return output_type == STM32_GPIO_OUTPUT_PUSH_PULL ||
           output_type == STM32_GPIO_OUTPUT_OPEN_DRAIN;
}

static bool speed_is_valid(stm32_gpio_speed_t speed)
{
    return speed == STM32_GPIO_SPEED_LOW ||
           speed == STM32_GPIO_SPEED_MEDIUM ||
           speed == STM32_GPIO_SPEED_HIGH ||
           speed == STM32_GPIO_SPEED_VERY_HIGH;
}

static bool pull_is_valid(stm32_gpio_pull_t pull)
{
    return pull == STM32_GPIO_PULL_NONE ||
           pull == STM32_GPIO_PULL_UP ||
           pull == STM32_GPIO_PULL_DOWN;
}

static bool config_is_valid(const stm32_gpio_output_config_t *config)
{
    return config != 0 &&
           output_type_is_valid(config->output_type) &&
           speed_is_valid(config->speed) &&
           pull_is_valid(config->pull);
}

static stm32_gpio_registers_t *find_registers(
    const stm32_gpio_config_context_t *context,
    const char *port_name)
{
    size_t index = 0u;

    if (context == 0 || context->ports == 0 || port_name == 0) {
        return 0;
    }

    /* context->ports 是“端口名 -> 寄存器地址”的查找表。
     * 这样 board_profile 只写 PA/PB，不需要知道 0x40020000 这样的芯片地址。 */
    for (index = 0u; index < context->port_count; index++) {
        if (context->ports[index].registers != 0 &&
            text_equals(context->ports[index].port, port_name)) {
            return context->ports[index].registers;
        }
    }

    return 0;
}

static void write_two_bit_field(
    volatile uint32_t *register_value,
    unsigned int pin,
    uint32_t value)
{
    uint32_t shift = pin * 2u;
    uint32_t mask = STM32_GPIO_TWO_BIT_MASK << shift;

    /* MODER/OSPEEDR/PUPDR 每个 pin 占 2 bit。
     * 先用 ~mask 清掉原来的 2 bit，再把新值移到对应位置写回。 */
    *register_value = (*register_value & ~mask) | ((value & STM32_GPIO_TWO_BIT_MASK) << shift);
}

static void write_one_bit_field(
    volatile uint32_t *register_value,
    unsigned int pin,
    bool enabled)
{
    uint32_t mask = 1u << pin;

    /* OTYPER 每个 pin 只占 1 bit，因此这里只需要设置或清除单个 bit。 */
    if (enabled) {
        *register_value |= mask;
    } else {
        *register_value &= ~mask;
    }
}

stm32_gpio_output_config_t stm32_gpio_output_config_default(void)
{
    stm32_gpio_output_config_t config;

    config.output_type = STM32_GPIO_OUTPUT_PUSH_PULL;
    config.speed = STM32_GPIO_SPEED_LOW;
    config.pull = STM32_GPIO_PULL_NONE;

    return config;
}

bool stm32_gpio_config_init(
    stm32_gpio_config_context_t *context,
    const stm32_gpio_config_port_t *ports,
    size_t port_count)
{
    if (context == 0 || ports == 0 || port_count == 0u) {
        return false;
    }

    context->ports = ports;
    context->port_count = port_count;

    return true;
}

bool stm32_gpio_configure_output(
    const stm32_gpio_config_context_t *context,
    const board_pin_t *pin,
    const stm32_gpio_output_config_t *config)
{
    stm32_gpio_registers_t *registers = 0;

    if (pin == 0 || pin->port == 0 || pin->pin > 15u || !config_is_valid(config)) {
        return false;
    }

    registers = find_registers(context, pin->port);
    if (registers == 0) {
        return false;
    }

    /* 一个 STM32 GPIO pin 的初始化会分散在多个寄存器：
     * MODER 选择输出模式，OTYPER 选择推挽/开漏，OSPEEDR 选择速度，PUPDR 选择上下拉。 */
    write_two_bit_field(&registers->moder, pin->pin, STM32_GPIO_MODE_OUTPUT);
    write_one_bit_field(
        &registers->otyper,
        pin->pin,
        config->output_type == STM32_GPIO_OUTPUT_OPEN_DRAIN);
    write_two_bit_field(&registers->ospeedr, pin->pin, (uint32_t)config->speed);
    write_two_bit_field(&registers->pupdr, pin->pin, (uint32_t)config->pull);

    return true;
}
