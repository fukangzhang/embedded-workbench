#ifndef EMBEDDED_WORKBENCH_RTOS_PORT_FREERTOS_H
#define EMBEDDED_WORKBENCH_RTOS_PORT_FREERTOS_H

#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "embedded_workbench/alarm_output_sink.h"
#include "embedded_workbench/rtos_port.h"

typedef struct {
    /* FreeRTOS 的队列和任务句柄都集中放在 context 里。
     * 这样应用层只拿到 rtos_port_t，不需要知道底层到底用了几个 QueueHandle_t。 */
    QueueHandle_t sensor_sample_queue;
    QueueHandle_t command_queue;
    QueueHandle_t response_queue;
    QueueHandle_t alarm_event_queue;

    /* 输出任务需要一个 sink，把“告警输出命令”真正落到 LED/蜂鸣器/执行器。
     * 这里存指针而不是直接存对象，是为了后续可以接真实 GPIO 后端或测试假后端。 */
    alarm_output_sink_t *alarm_output_sink;

    /* xTaskCreate 会把创建出来的任务句柄写到这些字段。
     * 后续判断任务是否已经创建、是否可以启动调度器，都靠这些字段。 */
    TaskHandle_t sensor_acquire_task;
    TaskHandle_t env_process_task;
    TaskHandle_t communication_task;
    TaskHandle_t alarm_output_task;
} freertos_rtos_port_context_t;

bool freertos_rtos_port_init(rtos_port_t *port, freertos_rtos_port_context_t *context);
bool freertos_rtos_port_send_response(freertos_rtos_port_context_t *context, const rtos_response_message_t *response);
bool freertos_rtos_port_start_scheduler(freertos_rtos_port_context_t *context);

#endif
