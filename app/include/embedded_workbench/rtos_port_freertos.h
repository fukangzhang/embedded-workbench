#ifndef EMBEDDED_WORKBENCH_RTOS_PORT_FREERTOS_H
#define EMBEDDED_WORKBENCH_RTOS_PORT_FREERTOS_H

#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "embedded_workbench/alarm_output_sink.h"
#include "embedded_workbench/rtos_port.h"

typedef struct {
    QueueHandle_t sensor_sample_queue;
    QueueHandle_t command_queue;
    QueueHandle_t response_queue;
    QueueHandle_t alarm_event_queue;
    alarm_output_sink_t *alarm_output_sink;
    TaskHandle_t sensor_acquire_task;
    TaskHandle_t env_process_task;
    TaskHandle_t communication_task;
    TaskHandle_t alarm_output_task;
} freertos_rtos_port_context_t;

bool freertos_rtos_port_init(rtos_port_t *port, freertos_rtos_port_context_t *context);
bool freertos_rtos_port_send_response(freertos_rtos_port_context_t *context, const rtos_response_message_t *response);
bool freertos_rtos_port_start_scheduler(freertos_rtos_port_context_t *context);

#endif
