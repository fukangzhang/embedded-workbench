#ifndef EMBEDDED_WORKBENCH_SERIAL_COMMAND_INGRESS_PUMP_H
#define EMBEDDED_WORKBENCH_SERIAL_COMMAND_INGRESS_PUMP_H

#include <stddef.h>

#include "embedded_workbench/serial_command_ingress.h"

typedef bool (*serial_command_ingress_pump_read_fn)(void *context, char *byte_out);

/* 从 reader 有界读取字节并喂给 ingress。
 * 遇到 COMMAND_SUBMITTED / OVERFLOW / ERROR 会立即停下，把控制权还给调用方。 */
serial_command_ingress_status_t serial_command_ingress_pump_poll(
    serial_command_ingress_t *ingress,
    serial_command_ingress_pump_read_fn read,
    void *read_context,
    size_t max_bytes,
    size_t *bytes_read_out);

#endif
