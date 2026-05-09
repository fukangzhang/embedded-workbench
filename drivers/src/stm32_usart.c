#include "embedded_workbench/stm32_usart.h"

#define STM32_USART_SR_RXNE (1u << 5u)
#define STM32_USART_SR_TXE (1u << 7u)

#define STM32_USART_CR1_RE (1u << 2u)
#define STM32_USART_CR1_TE (1u << 3u)
#define STM32_USART_CR1_PCE (1u << 10u)
#define STM32_USART_CR1_PS (1u << 9u)
#define STM32_USART_CR1_M (1u << 12u)
#define STM32_USART_CR1_UE (1u << 13u)
#define STM32_USART_CR1_OVER8 (1u << 15u)

#define STM32_USART_CR2_STOP_MASK (3u << 12u)
#define STM32_USART_CR3_RTSE (1u << 8u)
#define STM32_USART_CR3_CTSE (1u << 9u)

static bool config_is_valid(const stm32_usart_config_t *config)
{
    return config != 0 &&
           config->peripheral_clock_hz > 0u &&
           config->baud_rate > 0u &&
           (config->receiver_enabled || config->transmitter_enabled);
}

static uint32_t baud_register_value(uint32_t peripheral_clock_hz, uint32_t baud_rate)
{
    /* OVER8=0 时，常见 8N1 配置下 BRR 可用 APB clock / baud 四舍五入得到。 */
    return (peripheral_clock_hz + (baud_rate / 2u)) / baud_rate;
}

stm32_usart_config_t stm32_usart_config_default(uint32_t peripheral_clock_hz, uint32_t baud_rate)
{
    stm32_usart_config_t config;

    config.peripheral_clock_hz = peripheral_clock_hz;
    config.baud_rate = baud_rate;
    config.receiver_enabled = true;
    config.transmitter_enabled = true;

    return config;
}

bool stm32_usart_configure_8n1(stm32_usart_registers_t *registers, const stm32_usart_config_t *config)
{
    uint32_t brr = 0u;
    uint32_t cr1 = 0u;

    if (registers == 0 || !config_is_valid(config)) {
        return false;
    }

    brr = baud_register_value(config->peripheral_clock_hz, config->baud_rate);
    if (brr == 0u || brr > 0xffffu) {
        return false;
    }

    /* 配置期间先清 UE，避免一边启用 USART 一边改 BRR/帧格式。 */
    registers->cr1 &= ~STM32_USART_CR1_UE;
    registers->brr = brr;

    /* 8N1：8 data bits、no parity、1 stop bit、oversampling by 16。 */
    cr1 = registers->cr1;
    cr1 &= ~(STM32_USART_CR1_M |
             STM32_USART_CR1_PCE |
             STM32_USART_CR1_PS |
             STM32_USART_CR1_OVER8 |
             STM32_USART_CR1_RE |
             STM32_USART_CR1_TE |
             STM32_USART_CR1_UE);
    if (config->receiver_enabled) {
        cr1 |= STM32_USART_CR1_RE;
    }
    if (config->transmitter_enabled) {
        cr1 |= STM32_USART_CR1_TE;
    }
    cr1 |= STM32_USART_CR1_UE;
    registers->cr1 = cr1;

    registers->cr2 &= ~STM32_USART_CR2_STOP_MASK;
    registers->cr3 &= ~(STM32_USART_CR3_RTSE | STM32_USART_CR3_CTSE);

    return true;
}

bool stm32_usart_rx_ready(const stm32_usart_registers_t *registers)
{
    return registers != 0 && (registers->sr & STM32_USART_SR_RXNE) != 0u;
}

bool stm32_usart_tx_ready(const stm32_usart_registers_t *registers)
{
    return registers != 0 && (registers->sr & STM32_USART_SR_TXE) != 0u;
}

bool stm32_usart_read_byte(stm32_usart_registers_t *registers, char *byte_out)
{
    if (registers == 0 || byte_out == 0 || !stm32_usart_rx_ready(registers)) {
        return false;
    }

    *byte_out = (char)(registers->dr & 0xffu);
    return true;
}

bool stm32_usart_write_byte(stm32_usart_registers_t *registers, char byte)
{
    if (registers == 0 || !stm32_usart_tx_ready(registers)) {
        return false;
    }

    registers->dr = (uint32_t)(uint8_t)byte;
    return true;
}
