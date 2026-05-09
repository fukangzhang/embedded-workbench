#ifndef EMBEDDED_WORKBENCH_STM32_USART_SERIAL_IO_H
#define EMBEDDED_WORKBENCH_STM32_USART_SERIAL_IO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "embedded_workbench/stm32_usart.h"

#define STM32_USART_SERIAL_IO_DEFAULT_MAX_POLL_ATTEMPTS 100000u

typedef struct {
    /* registers 指向 USART 寄存器块。
     * 在测试里它指向假寄存器；在真实固件里它会指向 USART2 等内存映射寄存器。 */
    stm32_usart_registers_t *registers;
    /* 写多个字节时等待 TXE 的最大轮询次数，避免硬件异常时永久卡死。 */
    uint32_t max_poll_attempts;
} stm32_usart_serial_io_context_t;

bool stm32_usart_serial_io_init(
    stm32_usart_serial_io_context_t *context,
    stm32_usart_registers_t *registers,
    uint32_t max_poll_attempts);

/* 非阻塞读 1 个字节：没有 RXNE 时返回 false，调用者之后可以再试。 */
bool stm32_usart_serial_io_read_byte(void *context, char *byte_out);

/* 写一段 buffer，签名兼容 serial_command_service_write_fn。
 * 每个字节都会等待 TXE；超过 max_poll_attempts 仍未 ready 时返回 false。 */
bool stm32_usart_serial_io_write(void *context, const char *data, size_t length);

#endif
