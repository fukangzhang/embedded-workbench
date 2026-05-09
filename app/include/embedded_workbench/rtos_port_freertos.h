#ifndef EMBEDDED_WORKBENCH_RTOS_PORT_FREERTOS_H
#define EMBEDDED_WORKBENCH_RTOS_PORT_FREERTOS_H

#include <stdbool.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "embedded_workbench/alarm_output_sink.h"
#include "embedded_workbench/rtos_port.h"
#include "embedded_workbench/sensor_source.h"
#include "embedded_workbench/serial_command_ingress_pump.h"

typedef bool (*freertos_rtos_port_response_write_fn)(void *context, const char *data, size_t length);

typedef struct {
    /* FreeRTOS 的队列和任务句柄都集中放在 context 里。
     * 这样应用层只拿到 rtos_port_t，不需要知道底层到底用了几个 QueueHandle_t。 */
    QueueHandle_t sensor_sample_queue;
    QueueHandle_t command_queue;
    QueueHandle_t response_queue;
    QueueHandle_t alarm_event_queue;
    QueueHandle_t config_update_queue;

    /* 输出任务需要一个 sink，把“告警输出命令”真正落到 LED/蜂鸣器/执行器。
     * 这里存指针而不是直接存对象，是为了后续可以接真实 GPIO 后端或测试假后端。 */
    alarm_output_sink_t *alarm_output_sink;

    /* 采集任务的输入来源。为空时任务只保留周期骨架；非空时读取 sample 并送入队列。 */
    sensor_source_t *sensor_source;

    /* 可选命令字节来源。配置后 communication_task 会轮询 reader 并投递 command_queue。
     * 为空时保持原有行为：只消费外部已经放入 command_queue 的 command_t。 */
    serial_command_ingress_pump_read_fn command_read;
    void *command_read_context;

    /* 可选响应写出端。配置后 communication_task 会把 response_queue 的文本写到串口等后端。
     * 为空时保持测试/早期固件行为：响应只留在 response_queue，等待外部 receive_response。 */
    freertos_rtos_port_response_write_fn response_write;
    void *response_write_context;

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
