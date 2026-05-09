#ifndef EMBEDDED_WORKBENCH_STM32F401RE_GPIO_BINDINGS_H
#define EMBEDDED_WORKBENCH_STM32F401RE_GPIO_BINDINGS_H

#include <stddef.h>
#include <stdint.h>

#include "embedded_workbench/stm32_gpio_config.h"
#include "embedded_workbench/stm32_gpio_output.h"
#include "embedded_workbench/stm32_rcc_gpio_clock.h"
#include "embedded_workbench/stm32_rcc_usart_clock.h"
#include "embedded_workbench/stm32_usart.h"

/* 这些常量来自 STM32F401RE 的内存映射：
 * - RCC_AHB1ENR：打开 GPIOA/GPIOB 等外设时钟
 * - GPIOA/GPIOB base：各 GPIO 端口寄存器组的起始地址
 * - BSRR offset：GPIO 端口内“原子置位/复位”寄存器偏移
 *
 * 注意：这里只保存地址数值。主机测试只能比较地址，不能解引用这些地址；
 * 只有在真正跑在 STM32 上时，这些地址才对应真实硬件寄存器。 */
#define STM32F401RE_RCC_AHB1ENR_ADDRESS ((uintptr_t)0x40023830u)
#define STM32F401RE_RCC_APB1ENR_ADDRESS ((uintptr_t)0x40023840u)
#define STM32F401RE_GPIOA_BASE_ADDRESS ((uintptr_t)0x40020000u)
#define STM32F401RE_GPIOB_BASE_ADDRESS ((uintptr_t)0x40020400u)
#define STM32F401RE_GPIO_BSRR_OFFSET ((uintptr_t)0x18u)
#define STM32F401RE_USART2_BASE_ADDRESS ((uintptr_t)0x40004400u)
#define STM32F401RE_USART2_APB1ENR_BIT 17u
#define STM32F401RE_USART2_GPIO_AF 7u

/* 返回值都是静态表的指针，调用者不要释放，也不要修改表内容。
 * count 可以传 0，表示只要表指针，不关心元素数量。 */
volatile uint32_t *stm32f401re_rcc_ahb1enr(void);
volatile uint32_t *stm32f401re_rcc_apb1enr(void);
const stm32_rcc_gpio_clock_port_t *stm32f401re_gpio_clock_ports(size_t *count);
const stm32_rcc_usart_clock_peripheral_t *stm32f401re_usart_clock_peripherals(size_t *count);
const stm32_gpio_config_port_t *stm32f401re_gpio_config_ports(size_t *count);
const stm32_gpio_output_port_t *stm32f401re_gpio_output_ports(size_t *count);
stm32_usart_registers_t *stm32f401re_usart2_registers(void);
board_pin_t stm32f401re_usart2_tx_pin(void);
board_pin_t stm32f401re_usart2_rx_pin(void);

#endif
