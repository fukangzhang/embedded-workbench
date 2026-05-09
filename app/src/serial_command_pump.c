#include "embedded_workbench/serial_command_pump.h"

serial_command_service_status_t serial_command_pump_poll(
    serial_command_service_t *service,
    serial_command_pump_read_fn read,
    void *read_context,
    size_t max_bytes,
    size_t *bytes_read_out)
{
    size_t bytes_read = 0u;
    serial_command_service_status_t status = SERIAL_COMMAND_SERVICE_STATUS_IDLE;

    if (bytes_read_out != 0) {
        *bytes_read_out = 0u;
    }

    if (service == 0 || read == 0 || max_bytes == 0u) {
        return SERIAL_COMMAND_SERVICE_STATUS_ERROR;
    }

    while (bytes_read < max_bytes) {
        char byte = '\0';

        /* 非阻塞 reader 没有读到字节时返回 false；pump 本轮就停下来。 */
        if (!read(read_context, &byte)) {
            break;
        }

        bytes_read++;
        status = serial_command_service_feed(service, byte);

        /* 收到完整响应、overflow 或 error 时，把状态交还给调用者，让外层决定下一步。 */
        if (status != SERIAL_COMMAND_SERVICE_STATUS_IDLE) {
            break;
        }
    }

    if (bytes_read_out != 0) {
        *bytes_read_out = bytes_read;
    }

    return status;
}
