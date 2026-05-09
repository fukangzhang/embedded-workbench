#ifndef EMBEDDED_WORKBENCH_STM32_BOARD_USART2_INIT_H
#define EMBEDDED_WORKBENCH_STM32_BOARD_USART2_INIT_H

#include <stdbool.h>

#include "embedded_workbench/stm32_gpio_config.h"
#include "embedded_workbench/stm32_rcc_gpio_clock.h"
#include "embedded_workbench/stm32_rcc_usart_clock.h"
#include "embedded_workbench/stm32_usart.h"

/* 板级 USART2 初始化入口。
 *
 * 这里故意只做“编排”：
 * - GPIO/RCC/USART 的寄存器细节仍然放在各自 driver。
 * - STM32F401RE 的 PA2/PA3/AF7 等固定事实来自 bindings。
 * - 调用者把 context 和 USART config 传进来，方便主机测试用假寄存器验证顺序。
 *
 * 返回 true 表示整条初始化链路都成功；任何一步找不到绑定或配置非法时返回 false。
 */
bool stm32_board_usart2_init(
    const stm32_rcc_gpio_clock_context_t *gpio_clock_context,
    const stm32_gpio_config_context_t *gpio_context,
    const stm32_rcc_usart_clock_context_t *usart_clock_context,
    stm32_usart_registers_t *usart_registers,
    const stm32_usart_config_t *usart_config);

#endif
