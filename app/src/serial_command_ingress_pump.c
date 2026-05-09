#include "embedded_workbench/serial_command_ingress_pump.h"

serial_command_ingress_status_t serial_command_ingress_pump_poll(
    serial_command_ingress_t *ingress,
    serial_command_ingress_pump_read_fn read,
    void *read_context,
    size_t max_bytes,
    size_t *bytes_read_out)
{
    size_t bytes_read = 0u;
    char byte = '\0';
    serial_command_ingress_status_t status = SERIAL_COMMAND_INGRESS_STATUS_IDLE;

    if (bytes_read_out != 0) {
        *bytes_read_out = 0u;
    }

    if (ingress == 0 || read == 0 || max_bytes == 0u) {
        return SERIAL_COMMAND_INGRESS_STATUS_ERROR;
    }

    while (bytes_read < max_bytes && read(read_context, &byte)) {
        bytes_read++;
        status = serial_command_ingress_feed(ingress, byte);

        if (status != SERIAL_COMMAND_INGRESS_STATUS_IDLE) {
            if (bytes_read_out != 0) {
                *bytes_read_out = bytes_read;
            }
            return status;
        }
    }

    if (bytes_read_out != 0) {
        *bytes_read_out = bytes_read;
    }

    return SERIAL_COMMAND_INGRESS_STATUS_IDLE;
}
