#include "embedded_workbench/response_format.h"

#include <stdio.h>

static bool format_written(int written, size_t buffer_size)
{
    return written >= 0 && (size_t)written < buffer_size;
}

bool response_format_result(
    char *buffer,
    size_t buffer_size,
    const command_handler_result_t *result)
{
    const char *prefix = "ERR";
    int written = 0;

    if (buffer == 0 || buffer_size == 0u || result == 0) {
        return false;
    }

    if (result->result == COMMAND_RESULT_OK) {
        prefix = "OK";
    }

    written = snprintf(
        buffer,
        buffer_size,
        "%s result=%s\n",
        prefix,
        command_result_name(result->result));

    return format_written(written, buffer_size);
}

bool response_format_status(
    char *buffer,
    size_t buffer_size,
    alarm_state_t state,
    const sensor_sample_t *sample)
{
    int written = 0;

    if (buffer == 0 || buffer_size == 0u || !sensor_sample_is_valid(sample)) {
        return false;
    }

    written = snprintf(
        buffer,
        buffer_size,
        "STATUS state=%s temp_c_x10=%d humidity_rh_x10=%u light_lux=%lu smoke_ppm=%u\n",
        alarm_state_name(state),
        (int)sample->temperature_c_x10,
        (unsigned int)sample->humidity_rh_x10,
        (unsigned long)sample->light_lux,
        (unsigned int)sample->smoke_ppm);

    return format_written(written, buffer_size);
}

bool response_format_config(
    char *buffer,
    size_t buffer_size,
    const alarm_config_t *config)
{
    int written = 0;

    if (buffer == 0 || buffer_size == 0u || !alarm_config_is_valid(config)) {
        return false;
    }

    written = snprintf(
        buffer,
        buffer_size,
        "CONFIG temp_warn=%d temp_alarm=%d temp_recover=%d hum_warn=%u hum_alarm=%u hum_recover=%u light_warn_low=%lu light_alarm_low=%lu light_recover_low=%lu smoke_warn=%u smoke_alarm=%u smoke_recover=%u\n",
        (int)config->temperature_warning_high_c_x10,
        (int)config->temperature_alarm_high_c_x10,
        (int)config->temperature_recovery_high_c_x10,
        (unsigned int)config->humidity_warning_high_rh_x10,
        (unsigned int)config->humidity_alarm_high_rh_x10,
        (unsigned int)config->humidity_recovery_high_rh_x10,
        (unsigned long)config->light_warning_low_lux,
        (unsigned long)config->light_alarm_low_lux,
        (unsigned long)config->light_recovery_low_lux,
        (unsigned int)config->smoke_warning_ppm,
        (unsigned int)config->smoke_alarm_ppm,
        (unsigned int)config->smoke_recovery_ppm);

    return format_written(written, buffer_size);
}
