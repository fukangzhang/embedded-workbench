#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "embedded_workbench/serial_command_pump.h"

typedef struct {
    const char *text;
    size_t index;
} fake_reader_t;

typedef struct {
    char data[512];
    size_t length;
    unsigned int write_count;
} capture_writer_t;

static int expect_status(serial_command_service_status_t actual, serial_command_service_status_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_size(size_t actual, size_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_uint(unsigned int actual, unsigned int expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

static bool fake_read(void *context, char *byte_out)
{
    fake_reader_t *reader = (fake_reader_t *)context;

    if (reader == 0 || byte_out == 0 || reader->text == 0 || reader->text[reader->index] == '\0') {
        return false;
    }

    *byte_out = reader->text[reader->index];
    reader->index++;

    return true;
}

static bool capture_write(void *context, const char *data, size_t length)
{
    capture_writer_t *writer = (capture_writer_t *)context;

    if (writer == 0 || data == 0 || writer->length + length >= sizeof(writer->data)) {
        return false;
    }

    memcpy(&writer->data[writer->length], data, length);
    writer->length += length;
    writer->data[writer->length] = '\0';
    writer->write_count++;

    return true;
}

static bool make_service(
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
    capture_writer_t *writer)
{
    return serial_command_service_init(
        service,
        rx_buffer,
        rx_capacity,
        line_buffer,
        line_capacity,
        response_buffer,
        response_capacity,
        config,
        state,
        sample,
        capture_write,
        writer);
}

static int test_no_data_is_idle(void)
{
    serial_command_service_t service;
    char rx_buffer[64];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u};
    fake_reader_t reader = {"", 0u};
    size_t bytes_read = 99u;

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    if (expect_status(serial_command_pump_poll(&service, fake_read, &reader, 8u, &bytes_read),
            SERIAL_COMMAND_SERVICE_STATUS_IDLE) != 0 ||
        expect_size(bytes_read, 0u) != 0 ||
        expect_uint(writer.write_count, 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_partial_line_stays_idle(void)
{
    serial_command_service_t service;
    char rx_buffer[64];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u};
    fake_reader_t reader = {"STATUS?", 0u};
    size_t bytes_read = 0u;

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    if (expect_status(serial_command_pump_poll(&service, fake_read, &reader, 16u, &bytes_read),
            SERIAL_COMMAND_SERVICE_STATUS_IDLE) != 0 ||
        expect_size(bytes_read, 7u) != 0 ||
        expect_uint(writer.write_count, 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_complete_command_sends_response(void)
{
    serial_command_service_t service;
    char rx_buffer[64];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_WARNING;
    sensor_sample_t sample = sensor_sample_make(360, 600u, 250u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u};
    fake_reader_t reader = {"STATUS?\nNEXT", 0u};
    size_t bytes_read = 0u;

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    /* pump 在 RESPONSE_SENT 后停下，后面的 NEXT 留给下一轮处理。 */
    if (expect_status(serial_command_pump_poll(&service, fake_read, &reader, 32u, &bytes_read),
            SERIAL_COMMAND_SERVICE_STATUS_RESPONSE_SENT) != 0 ||
        expect_size(bytes_read, 8u) != 0 ||
        expect_size(reader.index, 8u) != 0 ||
        expect_uint(writer.write_count, 1u) != 0 ||
        expect_string(
            writer.data,
            "OK result=ok\n"
            "STATUS state=warning temp_c_x10=360 humidity_rh_x10=600 light_lux=250 smoke_ppm=20 indicator=slow_blink buzzer=off actuator=off period_ms=1000\n") != 0) {
        return 2;
    }

    return 0;
}

static int test_max_bytes_limits_work_per_poll(void)
{
    serial_command_service_t service;
    char rx_buffer[64];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u};
    fake_reader_t reader = {"STATUS?\n", 0u};
    size_t bytes_read = 0u;

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    if (expect_status(serial_command_pump_poll(&service, fake_read, &reader, 3u, &bytes_read),
            SERIAL_COMMAND_SERVICE_STATUS_IDLE) != 0 ||
        expect_size(bytes_read, 3u) != 0 ||
        expect_uint(writer.write_count, 0u) != 0) {
        return 2;
    }

    if (expect_status(serial_command_pump_poll(&service, fake_read, &reader, 8u, &bytes_read),
            SERIAL_COMMAND_SERVICE_STATUS_RESPONSE_SENT) != 0 ||
        expect_size(bytes_read, 5u) != 0 ||
        expect_uint(writer.write_count, 1u) != 0) {
        return 3;
    }

    return 0;
}

static int test_overflow_status_is_returned(void)
{
    serial_command_service_t service;
    char rx_buffer[8];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u};
    fake_reader_t reader = {"ABCDEFGH", 0u};
    size_t bytes_read = 0u;

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    if (expect_status(serial_command_pump_poll(&service, fake_read, &reader, 16u, &bytes_read),
            SERIAL_COMMAND_SERVICE_STATUS_OVERFLOW) != 0 ||
        expect_size(bytes_read, 8u) != 0 ||
        expect_uint(writer.write_count, 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_invalid_arguments_are_error(void)
{
    serial_command_service_t service;
    size_t bytes_read = 99u;
    fake_reader_t reader = {"A", 0u};

    if (expect_status(serial_command_pump_poll(0, fake_read, &reader, 1u, &bytes_read),
            SERIAL_COMMAND_SERVICE_STATUS_ERROR) != 0 ||
        expect_size(bytes_read, 0u) != 0 ||
        expect_status(serial_command_pump_poll(&service, 0, &reader, 1u, 0),
            SERIAL_COMMAND_SERVICE_STATUS_ERROR) != 0 ||
        expect_status(serial_command_pump_poll(&service, fake_read, &reader, 0u, 0),
            SERIAL_COMMAND_SERVICE_STATUS_ERROR) != 0) {
        return 1;
    }

    return 0;
}

int main(void)
{
    if (test_no_data_is_idle() != 0) {
        return 1;
    }
    if (test_partial_line_stays_idle() != 0) {
        return 2;
    }
    if (test_complete_command_sends_response() != 0) {
        return 3;
    }
    if (test_max_bytes_limits_work_per_poll() != 0) {
        return 4;
    }
    if (test_overflow_status_is_returned() != 0) {
        return 5;
    }
    if (test_invalid_arguments_are_error() != 0) {
        return 6;
    }

    return 0;
}
