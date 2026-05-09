#include "embedded_workbench/stm32_board_gpio_init.h"

/* stm32_board_gpio_init 是板级 GPIO 初始化编排层。
 * 它把 board_profile 中的业务引脚，串到 RCC 时钟和 GPIO 配置两个底层驱动。 */

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
        /* 调用者不传配置时使用保守默认值：推挽、低速、无上下拉。
         * 这样上层只想“把告警输出脚初始化成输出”时，不必先理解所有电气选项。 */
        default_config = stm32_gpio_output_config_default();
        effective_config = &default_config;
    }

    /* 三个输出脚来自 board_profile；本模块只负责按统一规则逐个初始化。 */
    return init_output_pin(&profile->alarm_led, clock_context, gpio_context, effective_config) &&
           init_output_pin(&profile->alarm_buzzer, clock_context, gpio_context, effective_config) &&
           init_output_pin(&profile->alarm_actuator, clock_context, gpio_context, effective_config);
}
