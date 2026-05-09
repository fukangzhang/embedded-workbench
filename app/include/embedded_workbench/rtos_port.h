#ifndef EMBEDDED_WORKBENCH_RTOS_PORT_H
#define EMBEDDED_WORKBENCH_RTOS_PORT_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/sensor_sample.h"

typedef struct {
    /* 固定长度响应消息适合放进 RTOS queue，避免在任务之间传递堆内存。 */
    char text[128];
} rtos_response_message_t;

/* RTOS 端口抽象：主机测试可以替换成 fake，目标板可以接 FreeRTOS 队列。 */
typedef struct {
    /* start 在 FreeRTOS 后端里表示创建任务；是否启动调度器由更明确的 API 控制。 */
    bool (*start)(void *context);
    /* 把新传感器样本送入 RTOS 数据流。 */
    bool (*send_sensor_sample)(void *context, const sensor_sample_t *sample);
    /* 把已解析命令送入通信/命令处理数据流。 */
    bool (*send_command)(void *context, const command_t *command);
    /* 从响应队列中取出一条响应。 */
    bool (*receive_response)(void *context, rtos_response_message_t *response);
} rtos_port_ops_t;

typedef struct {
    /* ops 是函数表，context 是具体实现的私有状态；这是 C 里常用的接口写法。 */
    const rtos_port_ops_t *ops;
    void *context;
} rtos_port_t;

/* 包装函数在调用底层 ops 前统一做空指针和数据有效性检查。 */
bool rtos_port_is_valid(const rtos_port_t *port);
bool rtos_port_start(const rtos_port_t *port);
bool rtos_port_send_sensor_sample(const rtos_port_t *port, const sensor_sample_t *sample);
bool rtos_port_send_command(const rtos_port_t *port, const command_t *command);
bool rtos_port_receive_response(const rtos_port_t *port, rtos_response_message_t *response);

#endif
