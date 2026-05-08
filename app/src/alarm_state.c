#include "embedded_workbench/alarm_state.h"

alarm_config_t alarm_config_default(void)
{
    alarm_config_t config;

    config.temperature_warning_high_c_x10 = 350;
    config.temperature_alarm_high_c_x10 = 450;
    config.temperature_recovery_high_c_x10 = 330;
    config.humidity_warning_high_rh_x10 = 800;
    config.humidity_alarm_high_rh_x10 = 900;
    config.humidity_recovery_high_rh_x10 = 750;
    config.light_warning_low_lux = 100u;
    config.light_alarm_low_lux = 30u;
    config.light_recovery_low_lux = 150u;
    config.smoke_warning_ppm = 200u;
    config.smoke_alarm_ppm = 500u;
    config.smoke_recovery_ppm = 150u;

    return config;
}

bool alarm_config_is_valid(const alarm_config_t *config)
{
    if (config == 0) {
        return false;
    }

    if (config->temperature_recovery_high_c_x10 > config->temperature_warning_high_c_x10 ||
        config->temperature_warning_high_c_x10 > config->temperature_alarm_high_c_x10) {
        return false;
    }

    if (config->humidity_recovery_high_rh_x10 > config->humidity_warning_high_rh_x10 ||
        config->humidity_warning_high_rh_x10 > config->humidity_alarm_high_rh_x10) {
        return false;
    }

    if (config->light_alarm_low_lux > config->light_warning_low_lux ||
        config->light_warning_low_lux > config->light_recovery_low_lux) {
        return false;
    }

    if (config->smoke_recovery_ppm > config->smoke_warning_ppm ||
        config->smoke_warning_ppm > config->smoke_alarm_ppm) {
        return false;
    }

    return true;
}

static bool is_alarm_sample(const alarm_config_t *config, const sensor_sample_t *sample)
{
    return sample->temperature_c_x10 >= config->temperature_alarm_high_c_x10 ||
           sample->humidity_rh_x10 >= config->humidity_alarm_high_rh_x10 ||
           sample->light_lux <= config->light_alarm_low_lux ||
           sample->smoke_ppm >= config->smoke_alarm_ppm;
}

static bool is_warning_sample(const alarm_config_t *config, const sensor_sample_t *sample)
{
    return sample->temperature_c_x10 >= config->temperature_warning_high_c_x10 ||
           sample->humidity_rh_x10 >= config->humidity_warning_high_rh_x10 ||
           sample->light_lux <= config->light_warning_low_lux ||
           sample->smoke_ppm >= config->smoke_warning_ppm;
}

static bool is_recovered_sample(const alarm_config_t *config, const sensor_sample_t *sample)
{
    return sample->temperature_c_x10 <= config->temperature_recovery_high_c_x10 &&
           sample->humidity_rh_x10 <= config->humidity_recovery_high_rh_x10 &&
           sample->light_lux >= config->light_recovery_low_lux &&
           sample->smoke_ppm <= config->smoke_recovery_ppm;
}

alarm_state_t alarm_state_update(
    alarm_state_t current_state,
    const alarm_config_t *config,
    const sensor_sample_t *sample)
{
    if (!alarm_config_is_valid(config) || !sensor_sample_is_valid(sample)) {
        return ALARM_STATE_SENSOR_FAULT;
    }

    if (is_alarm_sample(config, sample)) {
        return ALARM_STATE_ALARM;
    }

    if (current_state == ALARM_STATE_ALARM && !is_recovered_sample(config, sample)) {
        return ALARM_STATE_ALARM;
    }

    if (is_warning_sample(config, sample)) {
        return ALARM_STATE_WARNING;
    }

    if (current_state == ALARM_STATE_WARNING && !is_recovered_sample(config, sample)) {
        return ALARM_STATE_WARNING;
    }

    return ALARM_STATE_NORMAL;
}

const char *alarm_state_name(alarm_state_t state)
{
    switch (state) {
    case ALARM_STATE_NORMAL:
        return "normal";
    case ALARM_STATE_WARNING:
        return "warning";
    case ALARM_STATE_ALARM:
        return "alarm";
    case ALARM_STATE_SENSOR_FAULT:
        return "sensor-fault";
    default:
        return "unknown";
    }
}
