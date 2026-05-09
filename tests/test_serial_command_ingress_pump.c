#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/serial_command_ingress_pump.h"

typedef struct {
    const char *text;
    size_t index;
} fake_reader_t;

typedef struct {
    command_t last_command;
    unsigned int submit_count;
} capture_submitter_t;

static int expect_status(serial_command_ingress_status_t actual, serial_command_ingress_status_t expected)
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

static bool capture_submit(void *context, const command_t *command)
{
    capture_submitter_t *submitter = (capture_submitter_t *)context;

    if (submitter == 0 || command == 0) {
        return false;
    }

    submitter->last_command = *command;
    submitter->submit_count++;
    return true;
}

static bool make_ingress(
    serial_command_ingress_t *ingress,
    char *rx_buffer,
    size_t rx_capacity,
    char *line_buffer,
    size_t line_capacity,
    capture_submitter_t *submitter)
{
    return serial_command_ingress_init(
        ingress,
        rx_buffer,
        rx_capacity,
        line_buffer,
        line_capacity,
        capture_submit,
        submitter);
}

static int test_no_data_is_idle(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u};
    fake_reader_t reader = {"", 0u};
    size_t bytes_read = 99u;

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(serial_command_ingress_pump_poll(&ingress, fake_read, &reader, 8u, &bytes_read),
            SERIAL_COMMAND_INGRESS_STATUS_IDLE) != 0 ||
        expect_size(bytes_read, 0u) != 0 ||
        expect_uint(submitter.submit_count, 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_partial_line_respects_max_bytes(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u};
    fake_reader_t reader = {"STATUS?\n", 0u};
    size_t bytes_read = 0u;

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(serial_command_ingress_pump_poll(&ingress, fake_read, &reader, 3u, &bytes_read),
            SERIAL_COMMAND_INGRESS_STATUS_IDLE) != 0 ||
        expect_size(bytes_read, 3u) != 0 ||
        expect_uint(submitter.submit_count, 0u) != 0 ||
        expect_size(reader.index, 3u) != 0) {
        return 2;
    }

    return 0;
}

static int test_complete_command_submits_and_stops(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u};
    fake_reader_t reader = {"STATUS?\nNEXT", 0u};
    size_t bytes_read = 0u;

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(serial_command_ingress_pump_poll(&ingress, fake_read, &reader, 32u, &bytes_read),
            SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED) != 0 ||
        expect_size(bytes_read, 8u) != 0 ||
        expect_size(reader.index, 8u) != 0 ||
        expect_uint(submitter.submit_count, 1u) != 0 ||
        submitter.last_command.type != COMMAND_TYPE_GET_STATUS) {
        return 2;
    }

    return 0;
}

static int test_next_poll_continues_after_max_bytes(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u};
    fake_reader_t reader = {"STATUS?\n", 0u};
    size_t bytes_read = 0u;

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(serial_command_ingress_pump_poll(&ingress, fake_read, &reader, 3u, &bytes_read),
            SERIAL_COMMAND_INGRESS_STATUS_IDLE) != 0 ||
        expect_size(bytes_read, 3u) != 0) {
        return 2;
    }

    if (expect_status(serial_command_ingress_pump_poll(&ingress, fake_read, &reader, 8u, &bytes_read),
            SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED) != 0 ||
        expect_size(bytes_read, 5u) != 0 ||
        expect_uint(submitter.submit_count, 1u) != 0) {
        return 3;
    }

    return 0;
}

static int test_overflow_status_is_returned(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[8];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u};
    fake_reader_t reader = {"ABCDEFGH", 0u};
    size_t bytes_read = 0u;

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(serial_command_ingress_pump_poll(&ingress, fake_read, &reader, 16u, &bytes_read),
            SERIAL_COMMAND_INGRESS_STATUS_OVERFLOW) != 0 ||
        expect_size(bytes_read, 8u) != 0 ||
        expect_uint(submitter.submit_count, 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_invalid_arguments_are_error(void)
{
    serial_command_ingress_t ingress;
    size_t bytes_read = 99u;
    fake_reader_t reader = {"A", 0u};

    if (expect_status(serial_command_ingress_pump_poll(0, fake_read, &reader, 1u, &bytes_read),
            SERIAL_COMMAND_INGRESS_STATUS_ERROR) != 0 ||
        expect_size(bytes_read, 0u) != 0 ||
        expect_status(serial_command_ingress_pump_poll(&ingress, 0, &reader, 1u, 0),
            SERIAL_COMMAND_INGRESS_STATUS_ERROR) != 0 ||
        expect_status(serial_command_ingress_pump_poll(&ingress, fake_read, &reader, 0u, 0),
            SERIAL_COMMAND_INGRESS_STATUS_ERROR) != 0) {
        return 1;
    }

    return 0;
}

int main(void)
{
    if (test_no_data_is_idle() != 0) {
        return 1;
    }
    if (test_partial_line_respects_max_bytes() != 0) {
        return 2;
    }
    if (test_complete_command_submits_and_stops() != 0) {
        return 3;
    }
    if (test_next_poll_continues_after_max_bytes() != 0) {
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
