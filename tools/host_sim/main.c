#include <stdio.h>
#include <string.h>

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/app_info.h"
#include "embedded_workbench/command_handler.h"
#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/sensor_sample.h"

static void print_sample(const sensor_sample_t *sample)
{
    printf(
        "sample temp=%.1fC humidity=%.1f%%RH light=%lulux smoke=%uppm\n",
        sample->temperature_c_x10 / 10.0,
        sample->humidity_rh_x10 / 10.0,
        (unsigned long)sample->light_lux,
        (unsigned int)sample->smoke_ppm);
}

static void print_config(const alarm_config_t *config)
{
    printf(
        "config temp_warn=%d temp_alarm=%d hum_warn=%u hum_alarm=%u light_warn_low=%lu light_alarm_low=%lu smoke_warn=%u smoke_alarm=%u\n",
        (int)config->temperature_warning_high_c_x10,
        (int)config->temperature_alarm_high_c_x10,
        (unsigned int)config->humidity_warning_high_rh_x10,
        (unsigned int)config->humidity_alarm_high_rh_x10,
        (unsigned long)config->light_warning_low_lux,
        (unsigned long)config->light_alarm_low_lux,
        (unsigned int)config->smoke_warning_ppm,
        (unsigned int)config->smoke_alarm_ppm);
}

static void print_status(alarm_state_t state, const sensor_sample_t *sample)
{
    print_sample(sample);
    printf("state=%s\n", alarm_state_name(state));
}

static alarm_state_t run_demo_samples(const alarm_config_t *config, sensor_sample_t *last_sample)
{
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t samples[] = {
        {250, 520u, 300u, 20u},
        {360, 610u, 280u, 30u},
        {370, 640u, 280u, 650u},
        {260, 500u, 320u, 20u},
    };
    size_t index = 0;
    size_t count = sizeof(samples) / sizeof(samples[0]);

    for (index = 0; index < count; index++) {
        state = alarm_state_update(state, config, &samples[index]);
        printf("demo[%lu] ", (unsigned long)index);
        print_status(state, &samples[index]);
    }

    *last_sample = samples[count - 1u];
    return state;
}

static void handle_script_command(
    const char *line,
    alarm_config_t *config,
    alarm_state_t *state,
    const sensor_sample_t *sample)
{
    command_t command;
    command_handler_result_t result;

    if (!command_parse(line, &command)) {
        printf("response error=invalid-command\n");
        return;
    }

    result = command_handler_handle(&command, config);
    printf("response result=%s\n", command_result_name(result.result));

    if (result.config_changed) {
        *state = alarm_state_update(*state, config, sample);
        print_config(config);
        printf("state=%s\n", alarm_state_name(*state));
    }

    if (result.status_requested) {
        print_status(*state, sample);
    }

    if (result.config_requested) {
        print_config(config);
    }

    if (result.alarm_clear_requested) {
        printf("clear_alarm=requested\n");
    }
}

int main(int argc, char **argv)
{
    alarm_config_t config = alarm_config_default();
    sensor_sample_t last_sample = sensor_sample_make(250, 500u, 300u, 20u);
    alarm_state_t state = ALARM_STATE_NORMAL;

    printf("%s %s\n", app_info_project_name(), app_info_version());
    printf("run_mode=%s\n", app_info_run_mode_name(APP_RUN_MODE_HOST_SIM));

    state = run_demo_samples(&config, &last_sample);

    if (argc > 1 && strcmp(argv[1], "--script") == 0) {
        char line[128];

        while (fgets(line, sizeof(line), stdin) != 0) {
            handle_script_command(line, &config, &state, &last_sample);
        }
    }

    return 0;
}
