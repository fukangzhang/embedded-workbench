#ifndef EMBEDDED_WORKBENCH_STM32_BOARD_GPIO_INIT_H
#define EMBEDDED_WORKBENCH_STM32_BOARD_GPIO_INIT_H

#include <stdbool.h>

#include "embedded_workbench/board_profile.h"
#include "embedded_workbench/stm32_gpio_config.h"
#include "embedded_workbench/stm32_rcc_gpio_clock.h"

bool stm32_board_gpio_init_alarm_outputs(
    const board_profile_t *profile,
    const stm32_rcc_gpio_clock_context_t *clock_context,
    const stm32_gpio_config_context_t *gpio_context,
    const stm32_gpio_output_config_t *output_config);

#endif
