#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/command_session.h"

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

static int expect_int(int actual, int expected)
{
    return actual == expected ? 0 : 1;
}

static command_session_t make_session(alarm_config_t *config, alarm_state_t *state, sensor_sample_t *sample)
{
    command_session_t session;

    /* 测试用一个小工厂函数，避免每个用例重复填 session 的三个指针。 */
    session.config = config;
    session.state = state;
    session.sample = sample;

    return session;
}

static int test_invalid_command_response(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_session_t session = make_session(&config, &state, &sample);
    char response[128];

    /* session 层即使遇到无法解析的命令，也要输出统一错误响应，方便串口端调试。 */
    if (expect_true(command_session_handle_line(&session, "NOPE\n", response, sizeof(response))) != 0 ||
        expect_string(response, "ERR result=invalid-command\n") != 0) {
        return 1;
    }

    return 0;
}

static int test_status_response(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_WARNING;
    sensor_sample_t sample = sensor_sample_make(360, 600u, 250u, 20u);
    command_session_t session = make_session(&config, &state, &sample);
    char response[256];

    /* STATUS? 会产生两段文本：命令结果 + 当前状态快照。 */
    if (expect_true(command_session_handle_line(&session, "STATUS?\n", response, sizeof(response))) != 0 ||
        expect_string(
            response,
            "OK result=ok\n"
            "STATUS state=warning temp_c_x10=360 humidity_rh_x10=600 light_lux=250 smoke_ppm=20 indicator=slow_blink buzzer=off actuator=off period_ms=1000\n") != 0) {
        return 1;
    }

    return 0;
}

static int test_config_response(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_session_t session = make_session(&config, &state, &sample);
    char response[512];

    /* CONFIG? 固定完整配置快照，避免以后字段顺序变化破坏终端/脚本解析。 */
    if (expect_true(command_session_handle_line(&session, "CONFIG?\n", response, sizeof(response))) != 0 ||
        expect_string(
            response,
            "OK result=ok\n"
            "CONFIG temp_warn=350 temp_alarm=450 temp_recover=330 hum_warn=800 hum_alarm=900 hum_recover=750 light_warn_low=100 light_alarm_low=30 light_recover_low=150 smoke_warn=200 smoke_alarm=500 smoke_recover=150\n") != 0) {
        return 1;
    }

    return 0;
}

static int test_set_updates_config_and_status(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(360, 500u, 300u, 20u);
    command_session_t session = make_session(&config, &state, &sample);
    char response[512];

    /* SET 成功是 session 最复杂的路径：
     * 它要更新配置、根据当前样本重算状态，并一次性返回 OK + CONFIG + STATUS。 */
    if (expect_true(command_session_handle_line(&session, "SET TEMP_WARN 360\n", response, sizeof(response))) != 0 ||
        expect_int(config.temperature_warning_high_c_x10, 360) != 0 ||
        expect_int(state, ALARM_STATE_WARNING) != 0 ||
        expect_string(
            response,
            "OK result=ok\n"
            "CONFIG temp_warn=360 temp_alarm=450 temp_recover=330 hum_warn=800 hum_alarm=900 hum_recover=750 light_warn_low=100 light_alarm_low=30 light_recover_low=150 smoke_warn=200 smoke_alarm=500 smoke_recover=150\n"
            "STATUS state=warning temp_c_x10=360 humidity_rh_x10=500 light_lux=300 smoke_ppm=20 indicator=slow_blink buzzer=off actuator=off period_ms=1000\n") != 0) {
        return 1;
    }

    return 0;
}

static int test_small_output_buffer_fails(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_session_t session = make_session(&config, &state, &sample);
    char response[8];

    /* response buffer 太小时，session 必须失败，不能留下截断协议文本。 */
    if (expect_false(command_session_handle_line(&session, "CONFIG?\n", response, sizeof(response))) != 0) {
        return 1;
    }

    return 0;
}

static int test_invalid_arguments_fail(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_session_t session = make_session(&config, &state, &sample);
    char response[128];

    /* 公共入口的防御性检查：session、line、response、response_size 都不能非法。 */
    if (expect_false(command_session_handle_line(0, "STATUS?\n", response, sizeof(response))) != 0 ||
        expect_false(command_session_handle_line(&session, 0, response, sizeof(response))) != 0 ||
        expect_false(command_session_handle_line(&session, "STATUS?\n", 0, sizeof(response))) != 0 ||
        expect_false(command_session_handle_line(&session, "STATUS?\n", response, 0u)) != 0) {
        return 1;
    }

    return 0;
}

int main(void)
{
    if (test_invalid_command_response() != 0) {
        return 1;
    }
    if (test_status_response() != 0) {
        return 2;
    }
    if (test_config_response() != 0) {
        return 3;
    }
    if (test_set_updates_config_and_status() != 0) {
        return 4;
    }
    if (test_small_output_buffer_fails() != 0) {
        return 5;
    }
    if (test_invalid_arguments_fail() != 0) {
        return 6;
    }

    return 0;
}
