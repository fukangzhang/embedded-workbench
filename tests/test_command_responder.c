#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/command_responder.h"

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

static command_responder_t make_responder(alarm_config_t *config, alarm_state_t *state, sensor_sample_t *sample)
{
    command_responder_t responder;

    if (!command_responder_init(&responder, config, state, sample)) {
        responder.config = 0;
        responder.state = 0;
        responder.sample = 0;
    }

    return responder;
}

static int test_status_response_from_parsed_command(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_WARNING;
    sensor_sample_t sample = sensor_sample_make(360, 600u, 250u, 20u);
    command_responder_t responder = make_responder(&config, &state, &sample);
    command_t command;
    char response[512];

    command_init(&command);
    command.type = COMMAND_TYPE_GET_STATUS;

    if (expect_true(command_responder_handle_command(&responder, &command, response, sizeof(response))) != 0 ||
        expect_string(
            response,
            "OK result=ok\n"
            "STATUS state=warning temp_c_x10=360 humidity_rh_x10=600 light_lux=250 smoke_ppm=20 indicator=slow_blink buzzer=off actuator=off period_ms=1000\n") != 0) {
        return 1;
    }

    return 0;
}

static int test_set_updates_config_and_status(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(360, 500u, 300u, 20u);
    command_responder_t responder = make_responder(&config, &state, &sample);
    command_responder_status_t status;
    command_t command;
    char response[512];

    command_init(&command);
    command.type = COMMAND_TYPE_SET_THRESHOLD;
    command.threshold = COMMAND_THRESHOLD_TEMP_WARNING_HIGH;
    command.value = 360;

    if (expect_true(command_responder_handle_command_with_status(&responder, &command, response, sizeof(response), &status)) != 0 ||
        expect_true(status.config_changed) != 0 ||
        expect_false(status.sample_changed) != 0 ||
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

static int test_sample_updates_sample_and_status(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_responder_t responder = make_responder(&config, &state, &sample);
    command_responder_status_t status;
    command_t command;
    char response[512];

    command_init(&command);
    command.type = COMMAND_TYPE_SET_SAMPLE;
    command.sample_temperature_c_x10 = 360;
    command.sample_humidity_rh_x10 = 600;
    command.sample_light_lux = 250;
    command.sample_smoke_ppm = 20;

    if (expect_true(command_responder_handle_command_with_status(&responder, &command, response, sizeof(response), &status)) != 0 ||
        expect_false(status.config_changed) != 0 ||
        expect_true(status.sample_changed) != 0 ||
        expect_int(sample.temperature_c_x10, 360) != 0 ||
        expect_int(state, ALARM_STATE_WARNING) != 0 ||
        expect_string(
            response,
            "OK result=ok\n"
            "STATUS state=warning temp_c_x10=360 humidity_rh_x10=600 light_lux=250 smoke_ppm=20 indicator=slow_blink buzzer=off actuator=off period_ms=1000\n") != 0) {
        return 1;
    }

    return 0;
}

static int test_config_and_clear_alarm_responses(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_responder_t responder = make_responder(&config, &state, &sample);
    command_t command;
    char response[512];

    command_init(&command);
    command.type = COMMAND_TYPE_GET_CONFIG;

    if (expect_true(command_responder_handle_command(&responder, &command, response, sizeof(response))) != 0 ||
        expect_string(
            response,
            "OK result=ok\n"
            "CONFIG temp_warn=350 temp_alarm=450 temp_recover=330 hum_warn=800 hum_alarm=900 hum_recover=750 light_warn_low=100 light_alarm_low=30 light_recover_low=150 smoke_warn=200 smoke_alarm=500 smoke_recover=150\n") != 0) {
        return 1;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_CLEAR_ALARM;

    if (expect_true(command_responder_handle_command(&responder, &command, response, sizeof(response))) != 0 ||
        expect_string(response, "OK result=ok\nclear_alarm=requested\n") != 0) {
        return 2;
    }

    return 0;
}

static int test_invalid_command_and_value_response(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_responder_t responder = make_responder(&config, &state, &sample);
    command_responder_status_t status;
    command_t command;
    char response[128];

    command_init(&command);
    command.type = COMMAND_TYPE_INVALID;

    if (expect_true(command_responder_handle_command_with_status(&responder, &command, response, sizeof(response), &status)) != 0 ||
        expect_false(status.config_changed) != 0 ||
        expect_false(status.sample_changed) != 0 ||
        expect_string(response, "ERR result=invalid-command\n") != 0) {
        return 1;
    }

    command_init(&command);
    command.type = COMMAND_TYPE_SET_SAMPLE;
    command.sample_temperature_c_x10 = SENSOR_TEMPERATURE_MAX_C_X10 + 1;
    command.sample_humidity_rh_x10 = 500;
    command.sample_light_lux = 300;
    command.sample_smoke_ppm = 20;

    status.config_changed = true;
    status.sample_changed = true;

    if (expect_true(command_responder_handle_command_with_status(&responder, &command, response, sizeof(response), &status)) != 0 ||
        expect_false(status.config_changed) != 0 ||
        expect_false(status.sample_changed) != 0 ||
        expect_string(response, "ERR result=invalid-value\n") != 0 ||
        expect_int(sample.temperature_c_x10, 250) != 0 ||
        expect_int(state, ALARM_STATE_NORMAL) != 0) {
        return 2;
    }

    return 0;
}

static int test_small_buffer_and_invalid_arguments_fail(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    alarm_config_t invalid_config = config;
    command_responder_t responder = make_responder(&config, &state, &sample);
    command_responder_t invalid_responder;
    command_t command;
    char response[8];

    command_init(&command);
    command.type = COMMAND_TYPE_GET_CONFIG;

    if (expect_false(command_responder_handle_command(&responder, &command, response, sizeof(response))) != 0) {
        return 1;
    }

    invalid_config.temperature_warning_high_c_x10 = 500;
    invalid_config.temperature_alarm_high_c_x10 = 450;

    if (expect_false(command_responder_init(&invalid_responder, &invalid_config, &state, &sample)) != 0 ||
        expect_false(command_responder_init(0, &config, &state, &sample)) != 0 ||
        expect_false(command_responder_init(&invalid_responder, 0, &state, &sample)) != 0 ||
        expect_false(command_responder_init(&invalid_responder, &config, 0, &sample)) != 0 ||
        expect_false(command_responder_init(&invalid_responder, &config, &state, 0)) != 0 ||
        expect_false(command_responder_handle_command(0, &command, response, sizeof(response))) != 0 ||
        expect_false(command_responder_handle_command(&responder, &command, 0, sizeof(response))) != 0 ||
        expect_false(command_responder_handle_command(&responder, &command, response, 0u)) != 0) {
        return 2;
    }

    return 0;
}

int main(void)
{
    if (test_status_response_from_parsed_command() != 0) {
        return 1;
    }
    if (test_set_updates_config_and_status() != 0) {
        return 2;
    }
    if (test_sample_updates_sample_and_status() != 0) {
        return 3;
    }
    if (test_config_and_clear_alarm_responses() != 0) {
        return 4;
    }
    if (test_invalid_command_and_value_response() != 0) {
        return 5;
    }
    if (test_small_buffer_and_invalid_arguments_fail() != 0) {
        return 6;
    }

    return 0;
}
