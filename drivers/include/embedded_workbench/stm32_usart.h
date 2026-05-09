#ifndef EMBEDDED_WORKBENCH_STM32_USART_H
#define EMBEDDED_WORKBENCH_STM32_USART_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    /* STM32 USART 前几个寄存器按参考手册顺序排列；当前只用到 SR/DR/BRR/CR1/CR2/CR3。 */
    volatile uint32_t sr;
    volatile uint32_t dr;
    volatile uint32_t brr;
    volatile uint32_t cr1;
    volatile uint32_t cr2;
    volatile uint32_t cr3;
} stm32_usart_registers_t;

typedef struct {
    /* peripheral_clock_hz 是该 USART 所在 APB 总线时钟，不一定等于 CPU 主频。 */
    uint32_t peripheral_clock_hz;
    uint32_t baud_rate;
    bool receiver_enabled;
    bool transmitter_enabled;
} stm32_usart_config_t;

stm32_usart_config_t stm32_usart_config_default(uint32_t peripheral_clock_hz, uint32_t baud_rate);
bool stm32_usart_configure_8n1(stm32_usart_registers_t *registers, const stm32_usart_config_t *config);
bool stm32_usart_rx_ready(const stm32_usart_registers_t *registers);
bool stm32_usart_tx_ready(const stm32_usart_registers_t *registers);
bool stm32_usart_read_byte(stm32_usart_registers_t *registers, char *byte_out);
bool stm32_usart_write_byte(stm32_usart_registers_t *registers, char byte);

#endif
