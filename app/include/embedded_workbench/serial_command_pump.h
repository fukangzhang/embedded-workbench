#ifndef EMBEDDED_WORKBENCH_SERIAL_COMMAND_PUMP_H
#define EMBEDDED_WORKBENCH_SERIAL_COMMAND_PUMP_H

#include <stddef.h>

#include "embedded_workbench/serial_command_service.h"

typedef bool (*serial_command_pump_read_fn)(void *context, char *byte_out);

/* 从 reader 最多读取 max_bytes 个字节，并逐个喂给 serial_command_service。
 *
 * reader 返回 false 表示当前没有字节可读；这不是错误，pump 会返回 IDLE。
 * bytes_read_out 可以传 0；非空时会拿到本轮实际读取的字节数。
 */
serial_command_service_status_t serial_command_pump_poll(
    serial_command_service_t *service,
    serial_command_pump_read_fn read,
    void *read_context,
    size_t max_bytes,
    size_t *bytes_read_out);

#endif
