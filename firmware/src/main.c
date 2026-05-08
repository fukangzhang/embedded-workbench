#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/board_profile.h"
#include "embedded_workbench/command_handler.h"
#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/response_format.h"
#include "embedded_workbench/rtos_task_model.h"
#include "embedded_workbench/sensor_sample.h"

#if defined(EW_FIRMWARE_USE_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif

static volatile alarm_state_t firmware_last_state = ALARM_STATE_NORMAL;
static volatile int firmware_self_check = 0;
uint32_t SystemCoreClock = 16000000u;

int main(void)
{
    alarm_config_t config = alarm_config_default();
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_t command;
    command_handler_result_t result;
    char response[160];

    command_init(&command);
    command.type = COMMAND_TYPE_GET_STATUS;

    result = command_handler_handle(&command, &config);
    firmware_last_state = alarm_state_update(ALARM_STATE_NORMAL, &config, &sample);

    if (board_profile_is_valid(board_profile_default()) &&
        rtos_task_model_is_valid() &&
        result.status_requested &&
        response_format_status(response, sizeof(response), firmware_last_state, &sample)) {
        firmware_self_check = 1;
    } else {
        firmware_self_check = -1;
    }

#if defined(EW_FIRMWARE_USE_FREERTOS)
    (void)xTaskGetSchedulerState();
#endif

    while (1) {
    }
}
