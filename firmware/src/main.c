#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/board_profile.h"
#include "embedded_workbench/command_handler.h"
#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/response_format.h"
#include "embedded_workbench/rtos_port.h"
#include "embedded_workbench/rtos_task_model.h"
#include "embedded_workbench/sensor_sample.h"

#if defined(EW_FIRMWARE_USE_FREERTOS)
#include "embedded_workbench/rtos_port_freertos.h"
#endif

static volatile alarm_state_t firmware_last_state = ALARM_STATE_NORMAL;
static volatile int firmware_self_check = 0;
uint32_t SystemCoreClock = 16000000u;

#if defined(EW_FIRMWARE_USE_FREERTOS)
static freertos_rtos_port_context_t firmware_rtos_context = {0};
static rtos_port_t firmware_rtos_port = {0};

static bool firmware_freertos_queue_self_check(const sensor_sample_t *sample, const command_t *command)
{
    rtos_response_message_t queued_response = {{0}};
    rtos_response_message_t received_response = {{0}};

    queued_response.text[0] = 'O';
    queued_response.text[1] = 'K';
    queued_response.text[2] = '\0';

    return freertos_rtos_port_init(&firmware_rtos_port, &firmware_rtos_context) &&
           rtos_port_start(&firmware_rtos_port) &&
           rtos_port_send_sensor_sample(&firmware_rtos_port, sample) &&
           rtos_port_send_command(&firmware_rtos_port, command) &&
           freertos_rtos_port_send_response(&firmware_rtos_context, &queued_response) &&
           rtos_port_receive_response(&firmware_rtos_port, &received_response) &&
           received_response.text[0] == 'O' &&
           received_response.text[1] == 'K' &&
           received_response.text[2] == '\0';
}
#endif

int main(void)
{
    alarm_config_t config = alarm_config_default();
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_t command;
    command_handler_result_t result;
    char response[160];
    bool freertos_ready = true;

    command_init(&command);
    command.type = COMMAND_TYPE_GET_STATUS;

    result = command_handler_handle(&command, &config);
    firmware_last_state = alarm_state_update(ALARM_STATE_NORMAL, &config, &sample);

#if defined(EW_FIRMWARE_USE_FREERTOS)
    freertos_ready = firmware_freertos_queue_self_check(&sample, &command);
#endif

    if (board_profile_is_valid(board_profile_default()) &&
        rtos_task_model_is_valid() &&
        result.status_requested &&
        response_format_status(response, sizeof(response), firmware_last_state, &sample) &&
        freertos_ready) {
        firmware_self_check = 1;
    } else {
        firmware_self_check = -1;
    }

    while (1) {
    }
}
