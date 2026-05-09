#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "embedded_workbench/serial_command_service.h"

typedef struct {
    char data[1024];
    size_t length;
    unsigned int write_count;
    bool fail_next_write;
} capture_writer_t;

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_status(serial_command_service_status_t actual, serial_command_service_status_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

static int expect_uint(unsigned int actual, unsigned int expected)
{
    return actual == expected ? 0 : 1;
}

static bool capture_write(void *context, const char *data, size_t length)
{
    capture_writer_t *writer = (capture_writer_t *)context;

    if (writer == 0 || data == 0) {
        return false;
    }

    if (writer->fail_next_write) {
        writer->fail_next_write = false;
        return false;
    }

    if (writer->length + length >= sizeof(writer->data)) {
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

static serial_command_service_status_t feed_text(serial_command_service_t *service, const char *text)
{
    serial_command_service_status_t status = SERIAL_COMMAND_SERVICE_STATUS_IDLE;

    while (*text != '\0') {
        status = serial_command_service_feed(service, *text);
        if (status == SERIAL_COMMAND_SERVICE_STATUS_ERROR ||
            status == SERIAL_COMMAND_SERVICE_STATUS_OVERFLOW) {
            return status;
        }
        text++;
    }

    return status;
}

static int test_status_is_sent_after_lf(void)
{
    serial_command_service_t service;
    char rx_buffer[64];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_WARNING;
    sensor_sample_t sample = sensor_sample_make(360, 600u, 250u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u, false};

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    /* 没有换行前只是在累计串口字节，不能提前调用 writer。 */
    if (expect_status(feed_text(&service, "STATUS?"), SERIAL_COMMAND_SERVICE_STATUS_IDLE) != 0 ||
        expect_uint(writer.write_count, 0u) != 0) {
        return 2;
    }

    if (expect_status(serial_command_service_feed(&service, '\n'), SERIAL_COMMAND_SERVICE_STATUS_RESPONSE_SENT) != 0 ||
        expect_uint(writer.write_count, 1u) != 0 ||
        expect_string(
            writer.data,
            "OK result=ok\n"
            "STATUS state=warning temp_c_x10=360 humidity_rh_x10=600 light_lux=250 smoke_ppm=20 indicator=slow_blink buzzer=off actuator=off period_ms=1000\n") != 0) {
        return 3;
    }

    return 0;
}

static int test_crlf_sends_single_response(void)
{
    serial_command_service_t service;
    char rx_buffer[64];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u, false};

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    /* CRLF 是终端和串口工具常见换行，service 只能对同一行响应一次。 */
    if (expect_status(feed_text(&service, "STATUS?\r\n"), SERIAL_COMMAND_SERVICE_STATUS_IDLE) != 0 ||
        expect_uint(writer.write_count, 1u) != 0) {
        return 2;
    }

    return 0;
}

static int test_set_updates_config_and_state(void)
{
    serial_command_service_t service;
    char rx_buffer[96];
    char line_buffer[96];
    char response_buffer[512];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(360, 500u, 300u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u, false};

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    /* SET 经过完整链路：串口行 -> command_session -> 配置更新 -> 状态重算 -> writer。 */
    if (expect_status(feed_text(&service, "SET TEMP_WARN 360\n"), SERIAL_COMMAND_SERVICE_STATUS_RESPONSE_SENT) != 0 ||
        config.temperature_warning_high_c_x10 != 360 ||
        state != ALARM_STATE_WARNING ||
        expect_uint(writer.write_count, 1u) != 0) {
        return 2;
    }

    return 0;
}

static int test_overflow_recovers_for_next_line(void)
{
    serial_command_service_t service;
    char rx_buffer[8];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u, false};

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    /* rx_buffer[8] 只能容纳 7 个普通字符和 '\0'，第 8 个字符触发 overflow。 */
    if (expect_status(feed_text(&service, "ABCDEFGH"), SERIAL_COMMAND_SERVICE_STATUS_OVERFLOW) != 0 ||
        expect_uint(writer.write_count, 0u) != 0) {
        return 2;
    }

    /* overflow 之后 reader 已复位，下一行合法命令仍然能正常响应。 */
    if (expect_status(feed_text(&service, "STATUS?\n"), SERIAL_COMMAND_SERVICE_STATUS_RESPONSE_SENT) != 0 ||
        expect_uint(writer.write_count, 1u) != 0) {
        return 3;
    }

    return 0;
}

static int test_writer_failure_is_error(void)
{
    serial_command_service_t service;
    char rx_buffer[64];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u, true};

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 1;
    }

    if (expect_status(feed_text(&service, "STATUS?\n"), SERIAL_COMMAND_SERVICE_STATUS_ERROR) != 0 ||
        expect_uint(writer.write_count, 0u) != 0) {
        return 2;
    }

    return 0;
}

static int test_invalid_arguments_fail(void)
{
    serial_command_service_t service;
    char rx_buffer[64];
    char line_buffer[64];
    char response_buffer[256];
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    capture_writer_t writer = {{0}, 0u, 0u, false};

    /* 初始化时所有关键指针和 buffer 都必须有效，避免后续串口回调里才炸。 */
    if (expect_false(serial_command_service_init(0, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, capture_write, &writer)) != 0 ||
        expect_false(serial_command_service_init(&service, 0, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, capture_write, &writer)) != 0 ||
        expect_false(serial_command_service_init(&service, rx_buffer, sizeof(rx_buffer), 0, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, capture_write, &writer)) != 0 ||
        expect_false(serial_command_service_init(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            0, sizeof(response_buffer), &config, &state, &sample, capture_write, &writer)) != 0 ||
        expect_false(serial_command_service_init(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, 0, &writer)) != 0) {
        return 1;
    }

    if (!make_service(&service, rx_buffer, sizeof(rx_buffer), line_buffer, sizeof(line_buffer),
            response_buffer, sizeof(response_buffer), &config, &state, &sample, &writer)) {
        return 2;
    }

    if (expect_status(serial_command_service_feed(0, 'A'), SERIAL_COMMAND_SERVICE_STATUS_ERROR) != 0) {
        return 3;
    }

    return 0;
}

int main(void)
{
    if (test_status_is_sent_after_lf() != 0) {
        return 1;
    }
    if (test_crlf_sends_single_response() != 0) {
        return 2;
    }
    if (test_set_updates_config_and_state() != 0) {
        return 3;
    }
    if (test_overflow_recovers_for_next_line() != 0) {
        return 4;
    }
    if (test_writer_failure_is_error() != 0) {
        return 5;
    }
    if (test_invalid_arguments_fail() != 0) {
        return 6;
    }

    return 0;
}
