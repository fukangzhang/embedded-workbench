#include "embedded_workbench/stm32_board_usart2_init.h"
#include "embedded_workbench/stm32f401re_gpio_bindings.h"

/* 板级 USART2 初始化编排层：按硬件要求串起 GPIO clock、GPIO AF、USART clock 和 USART 配置。 */

bool stm32_board_usart2_init(
    const stm32_rcc_gpio_clock_context_t *gpio_clock_context,
    const stm32_gpio_config_context_t *gpio_context,
    const stm32_rcc_usart_clock_context_t *usart_clock_context,
    stm32_usart_registers_t *usart_registers,
    const stm32_usart_config_t *usart_config)
{
    /* TX/RX 引脚是 NUCLEO-F401RE 上 USART2 的板级事实：
     * PA2 = USART2_TX，PA3 = USART2_RX。这里拿到的是描述，不直接写寄存器。 */
    board_pin_t tx_pin = stm32f401re_usart2_tx_pin();
    board_pin_t rx_pin = stm32f401re_usart2_rx_pin();

    /* USART2 在 PA2/PA3 上走 AF7。先拿默认 AF 配置，下面再按串口场景调速度和上下拉。 */
    stm32_gpio_alternate_config_t af_config =
        stm32_gpio_alternate_config_default(STM32F401RE_USART2_GPIO_AF);

    if (gpio_clock_context == 0 ||
        gpio_context == 0 ||
        usart_clock_context == 0 ||
        usart_registers == 0 ||
        usart_config == 0) {
        return false;
    }

    /* 串口脚通常需要更高的翻转速度；RX 加上拉可以让空闲线保持稳定高电平。
     * 这些属于板级初始化策略，所以放在这个编排层，而不是通用 GPIO AF helper。 */
    af_config.speed = STM32_GPIO_SPEED_VERY_HIGH;
    af_config.pull = STM32_GPIO_PULL_UP;

    /* 顺序很关键：先让 GPIOA 有时钟，再切 PA2/PA3 到 AF7，最后打开并配置 USART2。 */
    return stm32_rcc_enable_gpio_port_clock(gpio_clock_context, tx_pin.port) &&
           stm32_gpio_configure_alternate_function(gpio_context, &tx_pin, &af_config) &&
           stm32_gpio_configure_alternate_function(gpio_context, &rx_pin, &af_config) &&
           stm32_rcc_enable_usart_clock(usart_clock_context, "USART2") &&
           stm32_usart_configure_8n1(usart_registers, usart_config);
}
