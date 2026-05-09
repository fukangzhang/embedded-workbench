#include <stdio.h>
#include <string.h>

#include "embedded_workbench/alarm_output.h"
#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/app_info.h"
#include "embedded_workbench/command_session.h"
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

static void print_status(alarm_state_t state, const sensor_sample_t *sample)
{
    alarm_output_command_t output;

    print_sample(sample);
    printf("state=%s\n", alarm_state_name(state));

    if (alarm_output_command_for_state(state, &output)) {
        printf(
            "output indicator=%s buzzer=%s actuator=%s period_ms=%u\n",
            alarm_output_indicator_name(output.indicator),
            output.buzzer_enabled ? "on" : "off",
            output.actuator_enabled ? "on" : "off",
            (unsigned int)output.period_ms);
    }
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
    command_session_t session;
    char response[512];

    session.config = config;
    session.state = state;
    session.sample = sample;

    if (command_session_handle_line(&session, line, response, sizeof(response))) {
        printf("%s", response);
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
