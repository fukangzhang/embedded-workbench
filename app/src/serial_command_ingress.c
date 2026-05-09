#include "embedded_workbench/serial_command_ingress.h"

bool serial_command_ingress_init(
    serial_command_ingress_t *ingress,
    char *rx_buffer,
    size_t rx_capacity,
    char *line_buffer,
    size_t line_capacity,
    serial_command_ingress_submit_fn submit,
    void *submit_context)
{
    if (ingress == 0 ||
        line_buffer == 0 ||
        line_capacity == 0u ||
        submit == 0) {
        return false;
    }

    if (!serial_line_reader_init(&ingress->reader, rx_buffer, rx_capacity)) {
        return false;
    }

    ingress->line_buffer = line_buffer;
    ingress->line_capacity = line_capacity;
    ingress->submit = submit;
    ingress->submit_context = submit_context;

    return true;
}

serial_command_ingress_status_t serial_command_ingress_feed(serial_command_ingress_t *ingress, char byte)
{
    serial_line_status_t line_status = SERIAL_LINE_STATUS_IDLE;
    size_t line_length = 0u;
    command_t command;

    if (ingress == 0 ||
        ingress->line_buffer == 0 ||
        ingress->submit == 0) {
        return SERIAL_COMMAND_INGRESS_STATUS_ERROR;
    }

    line_status = serial_line_feed(
        &ingress->reader,
        byte,
        ingress->line_buffer,
        ingress->line_capacity,
        &line_length);

    if (line_status == SERIAL_LINE_STATUS_IDLE) {
        return SERIAL_COMMAND_INGRESS_STATUS_IDLE;
    }
    if (line_status == SERIAL_LINE_STATUS_OVERFLOW) {
        return SERIAL_COMMAND_INGRESS_STATUS_OVERFLOW;
    }
    if (line_status != SERIAL_LINE_STATUS_LINE_READY) {
        return SERIAL_COMMAND_INGRESS_STATUS_ERROR;
    }

    command_init(&command);
    (void)command_parse(ingress->line_buffer, &command);

    if (!ingress->submit(ingress->submit_context, &command)) {
        return SERIAL_COMMAND_INGRESS_STATUS_ERROR;
    }

    return SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED;
}
