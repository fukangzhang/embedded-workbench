#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/alarm_state.h"

static int expect_state(alarm_state_t actual, alarm_state_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

int main(void)
{
    alarm_config_t config = alarm_config_default();
    sensor_sample_t normal = sensor_sample_make(250, 500u, 300u, 20u);
    sensor_sample_t warm = sensor_sample_make(360, 500u, 300u, 20u);
    sensor_sample_t smoky = sensor_sample_make(250, 500u, 300u, 600u);
    sensor_sample_t dim = sensor_sample_make(250, 500u, 80u, 20u);
    sensor_sample_t recovered = sensor_sample_make(250, 500u, 300u, 20u);
    sensor_sample_t not_recovered_from_alarm = sensor_sample_make(360, 500u, 300u, 20u);
    sensor_sample_t invalid_sample = sensor_sample_make(SENSOR_TEMPERATURE_MAX_C_X10 + 1, 500u, 300u, 20u);

    if (expect_true(alarm_config_is_valid(&config)) != 0) {
        return 1;
    }

    if (expect_state(alarm_state_update(ALARM_STATE_NORMAL, &config, &normal), ALARM_STATE_NORMAL) != 0) {
        return 2;
    }

    if (expect_state(alarm_state_update(ALARM_STATE_NORMAL, &config, &warm), ALARM_STATE_WARNING) != 0) {
        return 3;
    }

    if (expect_state(alarm_state_update(ALARM_STATE_NORMAL, &config, &dim), ALARM_STATE_WARNING) != 0) {
        return 4;
    }

    if (expect_state(alarm_state_update(ALARM_STATE_NORMAL, &config, &smoky), ALARM_STATE_ALARM) != 0) {
        return 5;
    }

    if (expect_state(alarm_state_update(ALARM_STATE_ALARM, &config, &not_recovered_from_alarm), ALARM_STATE_ALARM) != 0) {
        return 6;
    }

    if (expect_state(alarm_state_update(ALARM_STATE_ALARM, &config, &recovered), ALARM_STATE_NORMAL) != 0) {
        return 7;
    }

    if (expect_state(alarm_state_update(ALARM_STATE_NORMAL, &config, &invalid_sample), ALARM_STATE_SENSOR_FAULT) != 0) {
        return 8;
    }

    if (expect_state(alarm_state_update(ALARM_STATE_NORMAL, 0, &normal), ALARM_STATE_SENSOR_FAULT) != 0) {
        return 9;
    }

    if (expect_string(alarm_state_name(ALARM_STATE_ALARM), "alarm") != 0) {
        return 10;
    }

    return 0;
}
