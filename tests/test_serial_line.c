#include <string.h>

#include "embedded_workbench/serial_line.h"

static int expect_bool(bool actual, bool expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_status(serial_line_status_t actual, serial_line_status_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_size(size_t actual, size_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

static int feed_text(serial_line_reader_t *reader, const char *text)
{
    const char *cursor = text;
    char line[32];
    size_t length = 0u;
    serial_line_status_t status = SERIAL_LINE_STATUS_IDLE;

    while (*cursor != '\0') {
        status = serial_line_feed(reader, *cursor, line, sizeof(line), &length);
        if (status == SERIAL_LINE_STATUS_OVERFLOW) {
            return 2;
        }
        cursor++;
    }

    return 0;
}

static int test_complete_line_on_lf(void)
{
    char storage[16];
    char line[16];
    size_t length = 0u;
    serial_line_reader_t reader;

    if (!serial_line_reader_init(&reader, storage, sizeof(storage))) {
        return 1;
    }

    if (expect_status(serial_line_feed(&reader, 'S', line, sizeof(line), &length), SERIAL_LINE_STATUS_IDLE) != 0 ||
        expect_status(serial_line_feed(&reader, 'T', line, sizeof(line), &length), SERIAL_LINE_STATUS_IDLE) != 0 ||
        expect_status(serial_line_feed(&reader, '\n', line, sizeof(line), &length), SERIAL_LINE_STATUS_LINE_READY) != 0 ||
        expect_string(line, "ST") != 0 ||
        expect_size(length, 2u) != 0 ||
        expect_size(serial_line_reader_length(&reader), 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_crlf_produces_single_line(void)
{
    char storage[32];
    char line[32];
    size_t length = 0u;
    serial_line_reader_t reader;

    if (!serial_line_reader_init(&reader, storage, sizeof(storage))) {
        return 1;
    }

    if (feed_text(&reader, "STATUS?") != 0) {
        return 2;
    }

    if (expect_status(serial_line_feed(&reader, '\r', line, sizeof(line), &length), SERIAL_LINE_STATUS_LINE_READY) != 0 ||
        expect_string(line, "STATUS?") != 0 ||
        expect_size(length, 7u) != 0) {
        return 3;
    }

    if (expect_status(serial_line_feed(&reader, '\n', line, sizeof(line), &length), SERIAL_LINE_STATUS_IDLE) != 0) {
        return 4;
    }

    return 0;
}

static int test_empty_lines_are_ignored(void)
{
    char storage[8];
    char line[8];
    size_t length = 123u;
    serial_line_reader_t reader;

    if (!serial_line_reader_init(&reader, storage, sizeof(storage))) {
        return 1;
    }

    if (expect_status(serial_line_feed(&reader, '\n', line, sizeof(line), &length), SERIAL_LINE_STATUS_IDLE) != 0 ||
        expect_status(serial_line_feed(&reader, '\r', line, sizeof(line), &length), SERIAL_LINE_STATUS_IDLE) != 0 ||
        expect_status(serial_line_feed(&reader, '\n', line, sizeof(line), &length), SERIAL_LINE_STATUS_IDLE) != 0 ||
        expect_size(serial_line_reader_length(&reader), 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_overflow_resets_reader(void)
{
    char storage[4];
    char line[8];
    size_t length = 0u;
    serial_line_reader_t reader;

    if (!serial_line_reader_init(&reader, storage, sizeof(storage))) {
        return 1;
    }

    if (expect_status(serial_line_feed(&reader, 'A', line, sizeof(line), &length), SERIAL_LINE_STATUS_IDLE) != 0 ||
        expect_status(serial_line_feed(&reader, 'B', line, sizeof(line), &length), SERIAL_LINE_STATUS_IDLE) != 0 ||
        expect_status(serial_line_feed(&reader, 'C', line, sizeof(line), &length), SERIAL_LINE_STATUS_IDLE) != 0 ||
        expect_status(serial_line_feed(&reader, 'D', line, sizeof(line), &length), SERIAL_LINE_STATUS_OVERFLOW) != 0 ||
        expect_size(serial_line_reader_length(&reader), 0u) != 0) {
        return 2;
    }

    if (feed_text(&reader, "OK\n") != 0) {
        return 3;
    }

    return 0;
}

static int test_invalid_arguments_are_rejected(void)
{
    char storage[8];
    char line[8];
    size_t length = 0u;
    serial_line_reader_t reader;

    if (expect_bool(serial_line_reader_init(0, storage, sizeof(storage)), false) != 0 ||
        expect_bool(serial_line_reader_init(&reader, 0, sizeof(storage)), false) != 0 ||
        expect_bool(serial_line_reader_init(&reader, storage, 0u), false) != 0) {
        return 1;
    }

    if (!serial_line_reader_init(&reader, storage, sizeof(storage))) {
        return 2;
    }

    if (expect_status(serial_line_feed(0, 'A', line, sizeof(line), &length), SERIAL_LINE_STATUS_ERROR) != 0 ||
        expect_status(serial_line_feed(&reader, 'A', 0, sizeof(line), &length), SERIAL_LINE_STATUS_ERROR) != 0 ||
        expect_status(serial_line_feed(&reader, 'A', line, 0u, &length), SERIAL_LINE_STATUS_ERROR) != 0 ||
        expect_status(serial_line_feed(&reader, 'A', line, sizeof(line), 0), SERIAL_LINE_STATUS_ERROR) != 0) {
        return 3;
    }

    return 0;
}

int main(void)
{
    if (test_complete_line_on_lf() != 0) {
        return 1;
    }
    if (test_crlf_produces_single_line() != 0) {
        return 2;
    }
    if (test_empty_lines_are_ignored() != 0) {
        return 3;
    }
    if (test_overflow_resets_reader() != 0) {
        return 4;
    }
    if (test_invalid_arguments_are_rejected() != 0) {
        return 5;
    }

    return 0;
}
