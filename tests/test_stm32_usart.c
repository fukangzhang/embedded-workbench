#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/stm32_usart.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_u32(uint32_t actual, uint32_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_char(char actual, char expected)
{
    return actual == expected ? 0 : 1;
}

static int test_configure_default_8n1(void)
{
    stm32_usart_registers_t usart = {0u, 0u, 0u, 0xffffffffu, 0xffffffffu, 0xffffffffu};
    stm32_usart_config_t config = stm32_usart_config_default(16000000u, 9600u);

    /* 16 MHz / 9600 约等于 1666.67，四舍五入后 BRR 为 1667，也就是 0x0683。 */
    if (expect_true(stm32_usart_configure_8n1(&usart, &config)) != 0 ||
        expect_u32(usart.brr, 1667u) != 0) {
        return 1;
    }

    /* 8N1 会清 parity、word length、oversampling 和 stop-bit 字段，并打开 UE/TE/RE。 */
    if (expect_u32(usart.cr1 & (1u << 13u), (1u << 13u)) != 0 ||
        expect_u32(usart.cr1 & (1u << 3u), (1u << 3u)) != 0 ||
        expect_u32(usart.cr1 & (1u << 2u), (1u << 2u)) != 0 ||
        expect_u32(usart.cr1 & ((1u << 15u) | (1u << 12u) | (1u << 10u) | (1u << 9u)), 0u) != 0 ||
        expect_u32(usart.cr2 & (3u << 12u), 0u) != 0 ||
        expect_u32(usart.cr3 & ((1u << 8u) | (1u << 9u)), 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_configure_rx_only_and_tx_only(void)
{
    stm32_usart_registers_t usart = {0u, 0u, 0u, 0u, 0u, 0u};
    stm32_usart_config_t config = stm32_usart_config_default(84000000u, 115200u);

    config.transmitter_enabled = false;
    if (expect_true(stm32_usart_configure_8n1(&usart, &config)) != 0 ||
        expect_u32(usart.brr, 729u) != 0 ||
        expect_u32(usart.cr1 & (1u << 2u), (1u << 2u)) != 0 ||
        expect_u32(usart.cr1 & (1u << 3u), 0u) != 0) {
        return 1;
    }

    config.transmitter_enabled = true;
    config.receiver_enabled = false;
    if (expect_true(stm32_usart_configure_8n1(&usart, &config)) != 0 ||
        expect_u32(usart.cr1 & (1u << 2u), 0u) != 0 ||
        expect_u32(usart.cr1 & (1u << 3u), (1u << 3u)) != 0) {
        return 2;
    }

    return 0;
}

static int test_invalid_config_is_rejected_without_register_changes(void)
{
    stm32_usart_registers_t usart = {0x20u, 0x55u, 0x123u, 0x456u, 0x789u, 0xabcu};
    stm32_usart_config_t config = stm32_usart_config_default(16000000u, 9600u);

    config.baud_rate = 0u;
    if (expect_false(stm32_usart_configure_8n1(&usart, &config)) != 0 ||
        expect_u32(usart.sr, 0x20u) != 0 ||
        expect_u32(usart.dr, 0x55u) != 0 ||
        expect_u32(usart.brr, 0x123u) != 0 ||
        expect_u32(usart.cr1, 0x456u) != 0 ||
        expect_u32(usart.cr2, 0x789u) != 0 ||
        expect_u32(usart.cr3, 0xabcu) != 0) {
        return 1;
    }

    config.baud_rate = 9600u;
    config.receiver_enabled = false;
    config.transmitter_enabled = false;
    if (expect_false(stm32_usart_configure_8n1(&usart, &config)) != 0 ||
        expect_false(stm32_usart_configure_8n1(0, &config)) != 0 ||
        expect_false(stm32_usart_configure_8n1(&usart, 0)) != 0) {
        return 2;
    }

    return 0;
}

static int test_polling_read_and_write(void)
{
    stm32_usart_registers_t usart = {0u, 0u, 0u, 0u, 0u, 0u};
    char byte = '\0';

    if (expect_false(stm32_usart_rx_ready(&usart)) != 0 ||
        expect_false(stm32_usart_read_byte(&usart, &byte)) != 0 ||
        expect_false(stm32_usart_write_byte(&usart, 'A')) != 0) {
        return 1;
    }

    usart.sr = (1u << 5u);
    usart.dr = 0x00000142u;
    if (expect_true(stm32_usart_rx_ready(&usart)) != 0 ||
        expect_true(stm32_usart_read_byte(&usart, &byte)) != 0 ||
        expect_char(byte, 'B') != 0) {
        return 2;
    }

    usart.sr = (1u << 7u);
    if (expect_true(stm32_usart_tx_ready(&usart)) != 0 ||
        expect_true(stm32_usart_write_byte(&usart, 'Z')) != 0 ||
        expect_u32(usart.dr, (uint32_t)'Z') != 0) {
        return 3;
    }

    if (expect_false(stm32_usart_rx_ready(0)) != 0 ||
        expect_false(stm32_usart_tx_ready(0)) != 0 ||
        expect_false(stm32_usart_read_byte(0, &byte)) != 0 ||
        expect_false(stm32_usart_read_byte(&usart, 0)) != 0 ||
        expect_false(stm32_usart_write_byte(0, 'A')) != 0) {
        return 4;
    }

    return 0;
}

int main(void)
{
    if (test_configure_default_8n1() != 0) {
        return 1;
    }
    if (test_configure_rx_only_and_tx_only() != 0) {
        return 2;
    }
    if (test_invalid_config_is_rejected_without_register_changes() != 0) {
        return 3;
    }
    if (test_polling_read_and_write() != 0) {
        return 4;
    }

    return 0;
}
