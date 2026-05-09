#ifndef EMBEDDED_WORKBENCH_STM32_GPIO_CONFIG_H
#define EMBEDDED_WORKBENCH_STM32_GPIO_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "embedded_workbench/board_profile.h"

typedef struct {
    /* 这里只列当前初始化输出脚需要的寄存器字段。
     * volatile 表示这些值可能被硬件改变，编译器不能随意缓存或优化掉访问。 */
    volatile uint32_t moder;
    volatile uint32_t otyper;
    volatile uint32_t ospeedr;
    volatile uint32_t pupdr;
    volatile uint32_t idr;
    volatile uint32_t odr;
    volatile uint32_t bsrr;
    volatile uint32_t lckr;
    volatile uint32_t afrl;
    volatile uint32_t afrh;
} stm32_gpio_registers_t;

typedef enum {
    /* 推挽输出：高低电平均由 MCU 主动驱动，是最常见的 LED/蜂鸣器输出方式。 */
    STM32_GPIO_OUTPUT_PUSH_PULL = 0,
    /* 开漏输出：只能主动拉低，高电平通常依赖外部/内部上拉。 */
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
    /* 输出类型写入 OTYPER。 */
    stm32_gpio_output_type_t output_type;
    /* 输出速度写入 OSPEEDR，影响边沿速度和 EMI。 */
    stm32_gpio_speed_t speed;
    /* 上下拉写入 PUPDR。 */
    stm32_gpio_pull_t pull;
} stm32_gpio_output_config_t;

typedef struct {
    /* alternate_function 写入 AFRL/AFRH，STM32F401 的 USART2 TX/RX 使用 AF7。 */
    uint8_t alternate_function;
    /* alternate function pin 也需要配置输出类型、速度和上下拉。 */
    stm32_gpio_output_type_t output_type;
    stm32_gpio_speed_t speed;
    stm32_gpio_pull_t pull;
} stm32_gpio_alternate_config_t;

typedef struct {
    /* port 是 "PA"/"PB" 这类板级名字，registers 指向该 GPIO 端口的寄存器组。
     * 在测试里它可以指向模拟结构体；在真实固件里它会指向芯片手册给出的固定地址。 */
    const char *port;
    stm32_gpio_registers_t *registers;
} stm32_gpio_config_port_t;

typedef struct {
    /* 端口查找表；初始化后不复制表内容，所以调用者要保证表生命周期足够长。 */
    const stm32_gpio_config_port_t *ports;
    size_t port_count;
} stm32_gpio_config_context_t;

/* 默认输出配置采用保守安全值：推挽、低速、无上下拉。 */
stm32_gpio_output_config_t stm32_gpio_output_config_default(void);
stm32_gpio_alternate_config_t stm32_gpio_alternate_config_default(uint8_t alternate_function);
bool stm32_gpio_config_init(
    stm32_gpio_config_context_t *context,
    const stm32_gpio_config_port_t *ports,
    size_t port_count);
bool stm32_gpio_configure_output(
    const stm32_gpio_config_context_t *context,
    const board_pin_t *pin,
    const stm32_gpio_output_config_t *config);
bool stm32_gpio_configure_alternate_function(
    const stm32_gpio_config_context_t *context,
    const board_pin_t *pin,
    const stm32_gpio_alternate_config_t *config);

#endif
