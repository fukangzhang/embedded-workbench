#include "embedded_workbench/stm32_usart_serial_io.h"

bool stm32_usart_serial_io_init(
    stm32_usart_serial_io_context_t *context,
    stm32_usart_registers_t *registers,
    uint32_t max_poll_attempts)
{
    if (context == 0 || registers == 0 || max_poll_attempts == 0u) {
        return false;
    }

    context->registers = registers;
    context->max_poll_attempts = max_poll_attempts;

    return true;
}

bool stm32_usart_serial_io_read_byte(void *context, char *byte_out)
{
    stm32_usart_serial_io_context_t *serial_context =
        (stm32_usart_serial_io_context_t *)context;

    if (serial_context == 0 || serial_context->registers == 0) {
        return false;
    }

    /* 读取动作交给底层 USART helper；这里的职责只是把 void* callback context 还原出来。 */
    return stm32_usart_read_byte(serial_context->registers, byte_out);
}

bool stm32_usart_serial_io_write(void *context, const char *data, size_t length)
{
    stm32_usart_serial_io_context_t *serial_context =
        (stm32_usart_serial_io_context_t *)context;
    size_t index = 0u;

    if (serial_context == 0 ||
        serial_context->registers == 0 ||
        (data == 0 && length > 0u)) {
        return false;
    }

    for (index = 0u; index < length; index++) {
        uint32_t attempt = 0u;
        bool byte_written = false;

        /* TXE 表示 data register 可以接收下一个字节。
         * 真实硬件上 TXE 可能在前一个字节写入后短暂清零，所以这里做有限轮询。 */
        for (attempt = 0u; attempt < serial_context->max_poll_attempts; attempt++) {
            if (stm32_usart_write_byte(serial_context->registers, data[index])) {
                byte_written = true;
                break;
            }
        }

        if (!byte_written) {
            return false;
        }
    }

    return true;
}
