#include <stdio.h>
#include <string.h>

#include "embedded_workbench/alarm_output.h"
#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/app_info.h"
#include "embedded_workbench/sensor_sample.h"
#include "embedded_workbench/serial_command_service.h"

static void print_sample(const sensor_sample_t *sample)
{
    /* 项目内部用整数 x10 保存小数；打印给人看时再除以 10.0。 */
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

    /* host_sim 不是最终产品 UI，它是把核心状态链路用终端文字展示出来。 */
    print_sample(sample);
    printf("state=%s\n", alarm_state_name(state));

    if (alarm_output_command_for_state(state, &output)) {
        /* 这里复用 app 层输出策略，不直接写 LED/GPIO，方便在 PC 上验证。 */
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
    /* 这组样本故意覆盖 normal -> warning/alarm -> recovery，便于第一次运行就看到状态变化。 */
    sensor_sample_t samples[] = {
        {250, 520u, 300u, 20u},
        {360, 610u, 280u, 30u},
        {370, 640u, 280u, 650u},
        {260, 500u, 320u, 20u},
    };
    size_t index = 0;
    size_t count = sizeof(samples) / sizeof(samples[0]);

    for (index = 0; index < count; index++) {
        /* alarm_state_update 是纯逻辑函数，所以同一份代码可在主机和 MCU 上复用。 */
        state = alarm_state_update(state, config, &samples[index]);
        printf("demo[%lu] ", (unsigned long)index);
        print_status(state, &samples[index]);
    }

    /* 脚本命令 STATUS? 需要知道最近一次 sample，这里保存 demo 的最后一条。 */
    *last_sample = samples[count - 1u];
    return state;
}

static bool stdout_write(void *context, const char *data, size_t length)
{
    (void)context;

    return fwrite(data, 1u, length, stdout) == length;
}

static int run_script_mode(alarm_config_t *config, alarm_state_t *state, const sensor_sample_t *sample)
{
    serial_command_service_t service;
    char rx_buffer[128];
    char line_buffer[128];
    char response_buffer[512];
    int input = 0;

    if (!serial_command_service_init(
            &service,
            rx_buffer,
            sizeof(rx_buffer),
            line_buffer,
            sizeof(line_buffer),
            response_buffer,
            sizeof(response_buffer),
            config,
            state,
            sample,
            stdout_write,
            0)) {
        return 1;
    }

    while ((input = getchar()) != EOF) {
        serial_command_service_status_t status = serial_command_service_feed(&service, (char)input);

        if (status == SERIAL_COMMAND_SERVICE_STATUS_OVERFLOW) {
            printf("ERR result=line-overflow\n");
        } else if (status == SERIAL_COMMAND_SERVICE_STATUS_ERROR) {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    alarm_config_t config = alarm_config_default();
    sensor_sample_t last_sample = sensor_sample_make(250, 500u, 300u, 20u);
    alarm_state_t state = ALARM_STATE_NORMAL;

    printf("%s %s\n", app_info_project_name(), app_info_version());
    printf("run_mode=%s\n", app_info_run_mode_name(APP_RUN_MODE_HOST_SIM));

    state = run_demo_samples(&config, &last_sample);

    /* --script 模式从 stdin 逐字节读命令，更接近真实串口收 byte 的方式。 */
    if (argc > 1 && strcmp(argv[1], "--script") == 0) {
        return run_script_mode(&config, &state, &last_sample);
    }

    return 0;
}
