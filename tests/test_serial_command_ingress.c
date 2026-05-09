#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/serial_command_ingress.h"

typedef struct {
    command_t last_command;
    unsigned int submit_count;
    bool fail_next_submit;
} capture_submitter_t;

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_status(serial_command_ingress_status_t actual, serial_command_ingress_status_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_uint(unsigned int actual, unsigned int expected)
{
    return actual == expected ? 0 : 1;
}

static bool capture_submit(void *context, const command_t *command)
{
    capture_submitter_t *submitter = (capture_submitter_t *)context;

    if (submitter == 0 || command == 0) {
        return false;
    }
    if (submitter->fail_next_submit) {
        submitter->fail_next_submit = false;
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

static serial_command_ingress_status_t feed_text(serial_command_ingress_t *ingress, const char *text)
{
    serial_command_ingress_status_t status = SERIAL_COMMAND_INGRESS_STATUS_IDLE;

    while (*text != '\0') {
        status = serial_command_ingress_feed(ingress, *text);
        if (status == SERIAL_COMMAND_INGRESS_STATUS_ERROR ||
            status == SERIAL_COMMAND_INGRESS_STATUS_OVERFLOW ||
            status == SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED) {
            return status;
        }
        text++;
    }

    return status;
}

static int test_partial_line_stays_idle(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u, false};

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(feed_text(&ingress, "STATUS?"), SERIAL_COMMAND_INGRESS_STATUS_IDLE) != 0 ||
        expect_uint(submitter.submit_count, 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_status_command_is_submitted_after_lf(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u, false};

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(feed_text(&ingress, "STATUS?\n"), SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED) != 0 ||
        expect_uint(submitter.submit_count, 1u) != 0 ||
        submitter.last_command.type != COMMAND_TYPE_GET_STATUS) {
        return 2;
    }

    return 0;
}

static int test_set_command_fields_are_submitted(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u, false};

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(feed_text(&ingress, "SET TEMP_WARN 360\n"), SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED) != 0 ||
        expect_uint(submitter.submit_count, 1u) != 0 ||
        submitter.last_command.type != COMMAND_TYPE_SET_THRESHOLD ||
        submitter.last_command.threshold != COMMAND_THRESHOLD_TEMP_WARNING_HIGH ||
        submitter.last_command.value != 360) {
        return 2;
    }

    return 0;
}

static int test_invalid_line_submits_invalid_command(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u, false};

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(feed_text(&ingress, "NOT_A_COMMAND\n"), SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED) != 0 ||
        expect_uint(submitter.submit_count, 1u) != 0 ||
        submitter.last_command.type != COMMAND_TYPE_INVALID) {
        return 2;
    }

    return 0;
}

static int test_crlf_submits_once(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u, false};

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(feed_text(&ingress, "CONFIG?\r\n"), SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED) != 0 ||
        expect_uint(submitter.submit_count, 1u) != 0 ||
        submitter.last_command.type != COMMAND_TYPE_GET_CONFIG ||
        expect_status(serial_command_ingress_feed(&ingress, '\n'), SERIAL_COMMAND_INGRESS_STATUS_IDLE) != 0 ||
        expect_uint(submitter.submit_count, 1u) != 0) {
        return 2;
    }

    return 0;
}

static int test_overflow_recovers_for_next_line(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[8];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u, false};

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(feed_text(&ingress, "ABCDEFGH"), SERIAL_COMMAND_INGRESS_STATUS_OVERFLOW) != 0 ||
        expect_uint(submitter.submit_count, 0u) != 0) {
        return 2;
    }

    if (expect_status(feed_text(&ingress, "STATUS?\n"), SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED) != 0 ||
        expect_uint(submitter.submit_count, 1u) != 0 ||
        submitter.last_command.type != COMMAND_TYPE_GET_STATUS) {
        return 3;
    }

    return 0;
}

static int test_submit_failure_is_error(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u, true};

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 1;
    }

    if (expect_status(feed_text(&ingress, "STATUS?\n"), SERIAL_COMMAND_INGRESS_STATUS_ERROR) != 0 ||
        expect_uint(submitter.submit_count, 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_invalid_arguments_fail(void)
{
    serial_command_ingress_t ingress;
    char rx_buffer[64];
    char line_buffer[64];
    capture_submitter_t submitter = {{0}, 0u, false};

    if (expect_false(serial_command_ingress_init(
            0,
            rx_buffer,
            sizeof(rx_buffer),
            line_buffer,
            sizeof(line_buffer),
            capture_submit,
            &submitter)) != 0 ||
        expect_false(serial_command_ingress_init(
            &ingress,
            0,
            sizeof(rx_buffer),
            line_buffer,
            sizeof(line_buffer),
            capture_submit,
            &submitter)) != 0 ||
        expect_false(serial_command_ingress_init(
            &ingress,
            rx_buffer,
            sizeof(rx_buffer),
            0,
            sizeof(line_buffer),
            capture_submit,
            &submitter)) != 0 ||
        expect_false(serial_command_ingress_init(
            &ingress,
            rx_buffer,
            sizeof(rx_buffer),
            line_buffer,
            sizeof(line_buffer),
            0,
            &submitter)) != 0) {
        return 1;
    }

    if (!make_ingress(&ingress, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer), &submitter)) {
        return 2;
    }
    if (expect_status(serial_command_ingress_feed(0, 'A'), SERIAL_COMMAND_INGRESS_STATUS_ERROR) != 0) {
        return 3;
    }

    return 0;
}

int main(void)
{
    if (test_partial_line_stays_idle() != 0) {
        return 1;
    }
    if (test_status_command_is_submitted_after_lf() != 0) {
        return 2;
    }
    if (test_set_command_fields_are_submitted() != 0) {
        return 3;
    }
    if (test_invalid_line_submits_invalid_command() != 0) {
        return 4;
    }
    if (test_crlf_submits_once() != 0) {
        return 5;
    }
    if (test_overflow_recovers_for_next_line() != 0) {
        return 6;
    }
    if (test_submit_failure_is_error() != 0) {
        return 7;
    }
    if (test_invalid_arguments_fail() != 0) {
        return 8;
    }

    return 0;
}
