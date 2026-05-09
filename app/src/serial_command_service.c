#include "embedded_workbench/serial_command_service.h"

static size_t string_length(const char *text)
{
    size_t length = 0u;

    while (text[length] != '\0') {
        length++;
    }

    return length;
}

bool serial_command_service_init(
    serial_command_service_t *service,
    char *rx_buffer,
    size_t rx_capacity,
    char *line_buffer,
    size_t line_capacity,
    char *response_buffer,
    size_t response_capacity,
    alarm_config_t *config,
    alarm_state_t *state,
    sensor_sample_t *sample,
    serial_command_service_write_fn write,
    void *write_context)
{
    if (service == 0 ||
        line_buffer == 0 ||
        line_capacity == 0u ||
        response_buffer == 0 ||
        response_capacity == 0u ||
        config == 0 ||
        state == 0 ||
        sample == 0 ||
        write == 0) {
        return false;
    }

    if (!serial_line_reader_init(&service->reader, rx_buffer, rx_capacity)) {
        return false;
    }

    service->line_buffer = line_buffer;
    service->line_capacity = line_capacity;
    service->response_buffer = response_buffer;
    service->response_capacity = response_capacity;
    service->session.config = config;
    service->session.state = state;
    service->session.sample = sample;
    service->write = write;
    service->write_context = write_context;

    return true;
}

serial_command_service_status_t serial_command_service_feed(serial_command_service_t *service, char byte)
{
    serial_line_status_t line_status = SERIAL_LINE_STATUS_IDLE;
    size_t line_length = 0u;
    size_t response_length = 0u;

    if (service == 0 ||
        service->line_buffer == 0 ||
        service->response_buffer == 0 ||
        service->write == 0) {
        return SERIAL_COMMAND_SERVICE_STATUS_ERROR;
    }

    line_status = serial_line_feed(
        &service->reader,
        byte,
        service->line_buffer,
        service->line_capacity,
        &line_length);

    if (line_status == SERIAL_LINE_STATUS_IDLE) {
        return SERIAL_COMMAND_SERVICE_STATUS_IDLE;
    }

    if (line_status == SERIAL_LINE_STATUS_OVERFLOW) {
        return SERIAL_COMMAND_SERVICE_STATUS_OVERFLOW;
    }

    if (line_status != SERIAL_LINE_STATUS_LINE_READY) {
        return SERIAL_COMMAND_SERVICE_STATUS_ERROR;
    }

    /* line_buffer 已经是以 '\0' 结尾的一行命令，可以直接交给 command_session。 */
    if (!command_session_handle_line(
            &service->session,
            service->line_buffer,
            service->response_buffer,
            service->response_capacity)) {
        return SERIAL_COMMAND_SERVICE_STATUS_ERROR;
    }

    response_length = string_length(service->response_buffer);
    if (!service->write(service->write_context, service->response_buffer, response_length)) {
        return SERIAL_COMMAND_SERVICE_STATUS_ERROR;
    }

    return SERIAL_COMMAND_SERVICE_STATUS_RESPONSE_SENT;
}
