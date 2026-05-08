#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/response_format.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

int main(void)
{
    char buffer[256];
    alarm_config_t config = alarm_config_default();
    sensor_sample_t sample = sensor_sample_make(253, 604u, 350u, 12u);
    command_handler_result_t ok_result;
    command_handler_result_t invalid_result;

    ok_result.result = COMMAND_RESULT_OK;
    ok_result.config_changed = false;
    ok_result.status_requested = true;
    ok_result.config_requested = false;
    ok_result.alarm_clear_requested = false;

    invalid_result = ok_result;
    invalid_result.result = COMMAND_RESULT_INVALID_VALUE;

    /* 响应格式使用精确字符串断言，确保协议文本对上位机保持稳定。 */
    if (expect_true(response_format_result(buffer, sizeof(buffer), &ok_result)) != 0 ||
        expect_string(buffer, "OK result=ok\n") != 0) {
        return 1;
    }

    if (expect_true(response_format_result(buffer, sizeof(buffer), &invalid_result)) != 0 ||
        expect_string(buffer, "ERR result=invalid-value\n") != 0) {
        return 2;
    }

    if (expect_true(response_format_status(buffer, sizeof(buffer), ALARM_STATE_WARNING, &sample)) != 0 ||
        expect_string(buffer, "STATUS state=warning temp_c_x10=253 humidity_rh_x10=604 light_lux=350 smoke_ppm=12 indicator=slow_blink buzzer=off actuator=off period_ms=1000\n") != 0) {
        return 3;
    }

    if (expect_true(response_format_config(buffer, sizeof(buffer), &config)) != 0) {
        return 4;
    }

    if (expect_false(response_format_status(buffer, 8u, ALARM_STATE_NORMAL, &sample)) != 0) {
        return 5;
    }

    /* 小缓冲区、空配置和未知状态都应该明确失败，而不是输出截断或误导性文本。 */
    if (expect_false(response_format_config(buffer, sizeof(buffer), 0)) != 0) {
        return 6;
    }

    if (expect_false(response_format_result(0, sizeof(buffer), &ok_result)) != 0) {
        return 7;
    }

    if (expect_false(response_format_status(buffer, sizeof(buffer), (alarm_state_t)99, &sample)) != 0) {
        return 8;
    }

    return 0;
}
