#ifndef EMBEDDED_WORKBENCH_STM32_BOARD_GPIO_INIT_H
#define EMBEDDED_WORKBENCH_STM32_BOARD_GPIO_INIT_H

#include <stdbool.h>

#include "embedded_workbench/board_profile.h"
#include "embedded_workbench/stm32_gpio_config.h"
#include "embedded_workbench/stm32_rcc_gpio_clock.h"

/* 按 board_profile 初始化三个告警输出脚。
 *
 * 这个函数把 BSP profile 和 STM32 驱动接起来：
 * 1. 为每个端口打开 RCC GPIO 时钟
 * 2. 把对应 pin 配成输出模式
 *
 * 它不负责后续写高/低电平，真正输出由 stm32_gpio_output/digital_output 完成。 */
bool stm32_board_gpio_init_alarm_outputs(
    const board_profile_t *profile,
    const stm32_rcc_gpio_clock_context_t *clock_context,
    const stm32_gpio_config_context_t *gpio_context,
    const stm32_gpio_output_config_t *output_config);

#endif
