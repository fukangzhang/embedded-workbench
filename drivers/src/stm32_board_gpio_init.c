#include "embedded_workbench/stm32_board_gpio_init.h"

static bool init_output_pin(
    const board_pin_t *pin,
    const stm32_rcc_gpio_clock_context_t *clock_context,
    const stm32_gpio_config_context_t *gpio_context,
    const stm32_gpio_output_config_t *output_config)
{
    /* 板级初始化的顺序很重要：先打开 GPIO 端口时钟，再改 GPIO 配置寄存器。 */
    return stm32_rcc_enable_gpio_port_clock(clock_context, pin->port) &&
           stm32_gpio_configure_output(gpio_context, pin, output_config);
}

bool stm32_board_gpio_init_alarm_outputs(
    const board_profile_t *profile,
    const stm32_rcc_gpio_clock_context_t *clock_context,
    const stm32_gpio_config_context_t *gpio_context,
    const stm32_gpio_output_config_t *output_config)
{
    stm32_gpio_output_config_t default_config;
    const stm32_gpio_output_config_t *effective_config = output_config;

    if (!board_profile_is_valid(profile) || clock_context == 0 || gpio_context == 0) {
        return false;
    }

    if (effective_config == 0) {
        default_config = stm32_gpio_output_config_default();
        effective_config = &default_config;
    }

    return init_output_pin(&profile->alarm_led, clock_context, gpio_context, effective_config) &&
           init_output_pin(&profile->alarm_buzzer, clock_context, gpio_context, effective_config) &&
           init_output_pin(&profile->alarm_actuator, clock_context, gpio_context, effective_config);
}
