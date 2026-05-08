#ifndef EMBEDDED_WORKBENCH_STM32_GPIO_CONFIG_H
#define EMBEDDED_WORKBENCH_STM32_GPIO_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "embedded_workbench/board_profile.h"

typedef struct {
    volatile uint32_t moder;
    volatile uint32_t otyper;
    volatile uint32_t ospeedr;
    volatile uint32_t pupdr;
} stm32_gpio_registers_t;

typedef enum {
    STM32_GPIO_OUTPUT_PUSH_PULL = 0,
    STM32_GPIO_OUTPUT_OPEN_DRAIN = 1
} stm32_gpio_output_type_t;

typedef enum {
    STM32_GPIO_SPEED_LOW = 0,
    STM32_GPIO_SPEED_MEDIUM = 1,
    STM32_GPIO_SPEED_HIGH = 2,
    STM32_GPIO_SPEED_VERY_HIGH = 3
} stm32_gpio_speed_t;

typedef enum {
    STM32_GPIO_PULL_NONE = 0,
    STM32_GPIO_PULL_UP = 1,
    STM32_GPIO_PULL_DOWN = 2
} stm32_gpio_pull_t;

typedef struct {
    stm32_gpio_output_type_t output_type;
    stm32_gpio_speed_t speed;
    stm32_gpio_pull_t pull;
} stm32_gpio_output_config_t;

typedef struct {
    const char *port;
    stm32_gpio_registers_t *registers;
} stm32_gpio_config_port_t;

typedef struct {
    const stm32_gpio_config_port_t *ports;
    size_t port_count;
} stm32_gpio_config_context_t;

stm32_gpio_output_config_t stm32_gpio_output_config_default(void);
bool stm32_gpio_config_init(
    stm32_gpio_config_context_t *context,
    const stm32_gpio_config_port_t *ports,
    size_t port_count);
bool stm32_gpio_configure_output(
    const stm32_gpio_config_context_t *context,
    const board_pin_t *pin,
    const stm32_gpio_output_config_t *config);

#endif
