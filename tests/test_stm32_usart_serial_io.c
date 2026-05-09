#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "embedded_workbench/stm32_usart_serial_io.h"

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

static int test_init_validates_arguments(void)
{
    stm32_usart_registers_t registers = {0u};
    stm32_usart_serial_io_context_t context;

    /* max_poll_attempts 不能为 0，否则写函数在任何情况下都没有机会检查 TXE。 */
    if (expect_true(stm32_usart_serial_io_init(&context, &registers, 3u)) != 0 ||
        expect_false(stm32_usart_serial_io_init(0, &registers, 3u)) != 0 ||
        expect_false(stm32_usart_serial_io_init(&context, 0, 3u)) != 0 ||
        expect_false(stm32_usart_serial_io_init(&context, &registers, 0u)) != 0) {
        return 1;
    }

    return 0;
}

static int test_read_byte_is_nonblocking(void)
{
    stm32_usart_registers_t registers = {0u};
    stm32_usart_serial_io_context_t context;
    char byte = '\0';

    if (!stm32_usart_serial_io_init(&context, &registers, 3u)) {
        return 1;
    }

    /* RXNE 没置位时没有完整字节可读，适配器应返回 false 而不是阻塞。 */
    if (expect_false(stm32_usart_serial_io_read_byte(&context, &byte)) != 0) {
        return 2;
    }

    registers.sr = 1u << 5u;
    registers.dr = (uint32_t)'A';
    if (expect_true(stm32_usart_serial_io_read_byte(&context, &byte)) != 0 ||
        byte != 'A') {
        return 3;
    }

    return 0;
}

static int test_write_uses_txe_polling(void)
{
    stm32_usart_registers_t registers = {0u};
    stm32_usart_serial_io_context_t context;
    const char text[] = "OK";

    if (!stm32_usart_serial_io_init(&context, &registers, 3u)) {
        return 1;
    }

    /* TXE 没置位时，有限轮询结束后返回 false，避免固件永久卡住。 */
    if (expect_false(stm32_usart_serial_io_write(&context, text, 2u)) != 0 ||
        expect_u32(registers.dr, 0u) != 0) {
        return 2;
    }

    registers.sr = 1u << 7u;
    if (expect_true(stm32_usart_serial_io_write(&context, text, 2u)) != 0 ||
        expect_u32(registers.dr, (uint32_t)'K') != 0) {
        return 3;
    }

    /* 空 buffer 写入应当成功，方便上层统一处理空响应。 */
    if (expect_true(stm32_usart_serial_io_write(&context, 0, 0u)) != 0 ||
        expect_false(stm32_usart_serial_io_write(0, text, 2u)) != 0 ||
        expect_false(stm32_usart_serial_io_write(&context, 0, 2u)) != 0) {
        return 4;
    }

    return 0;
}

int main(void)
{
    if (test_init_validates_arguments() != 0) {
        return 1;
    }
    if (test_read_byte_is_nonblocking() != 0) {
        return 2;
    }
    if (test_write_uses_txe_polling() != 0) {
        return 3;
    }

    return 0;
}
