#ifndef EMBEDDED_WORKBENCH_STM32F401RE_GPIO_BINDINGS_H
#define EMBEDDED_WORKBENCH_STM32F401RE_GPIO_BINDINGS_H

#include <stddef.h>
#include <stdint.h>

#include "embedded_workbench/stm32_gpio_config.h"
#include "embedded_workbench/stm32_gpio_output.h"
#include "embedded_workbench/stm32_rcc_gpio_clock.h"

#define STM32F401RE_RCC_AHB1ENR_ADDRESS ((uintptr_t)0x40023830u)
#define STM32F401RE_GPIOA_BASE_ADDRESS ((uintptr_t)0x40020000u)
#define STM32F401RE_GPIOB_BASE_ADDRESS ((uintptr_t)0x40020400u)
#define STM32F401RE_GPIO_BSRR_OFFSET ((uintptr_t)0x18u)

volatile uint32_t *stm32f401re_rcc_ahb1enr(void);
const stm32_rcc_gpio_clock_port_t *stm32f401re_gpio_clock_ports(size_t *count);
const stm32_gpio_config_port_t *stm32f401re_gpio_config_ports(size_t *count);
const stm32_gpio_output_port_t *stm32f401re_gpio_output_ports(size_t *count);

#endif
