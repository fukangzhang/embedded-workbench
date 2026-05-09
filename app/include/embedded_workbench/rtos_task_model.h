#ifndef EMBEDDED_WORKBENCH_RTOS_TASK_MODEL_H
#define EMBEDDED_WORKBENCH_RTOS_TASK_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    /* 周期性采集传感器，后续会把 sample 发到 sensor_sample_queue。 */
    RTOS_TASK_SENSOR_ACQUIRE = 0,
    /* 处理传感器样本，更新告警状态，并发出告警事件。 */
    RTOS_TASK_ENV_PROCESS = 1,
    /* 处理外部命令和响应，例如串口命令链路。 */
    RTOS_TASK_COMMUNICATION = 2,
    /* 根据告警事件刷新 LED/蜂鸣器/执行器输出。 */
    RTOS_TASK_ALARM_OUTPUT = 3
} rtos_task_id_t;

typedef enum {
    /* sensor_acquire -> env_process。 */
    RTOS_QUEUE_SENSOR_SAMPLE = 0,
    /* communication task 收到的命令。 */
    RTOS_QUEUE_COMMAND = 1,
    /* communication task 输出给外部接口的响应文本。 */
    RTOS_QUEUE_RESPONSE = 2,
    /* env_process -> alarm_output。 */
    RTOS_QUEUE_ALARM_EVENT = 3,
    /* communication -> env_process：同步 SET 命令产生的新告警配置快照。 */
    RTOS_QUEUE_CONFIG_UPDATE = 4
} rtos_queue_id_t;

typedef struct {
    /* id 是稳定枚举，代码用它查找任务。 */
    rtos_task_id_t id;
    /* name 会传给 FreeRTOS，方便调试器或 trace 工具显示。 */
    const char *name;
    /* FreeRTOS 中通常数值越大优先级越高，本项目用集中表避免散落魔法数字。 */
    uint8_t priority;
    /* 栈大小以 word 为单位，不是字节；这是 FreeRTOS API 的常见约定。 */
    uint16_t stack_words;
    /* 周期任务的目标节拍，后续会转换成 RTOS tick。 */
    uint32_t period_ms;
} rtos_task_descriptor_t;

typedef struct {
    /* id 是稳定枚举，FreeRTOS port 用它找到具体 queue 参数。 */
    rtos_queue_id_t id;
    /* name 主要用于文档、测试和后续调试输出。 */
    const char *name;
    /* 每个队列元素的字节大小，必须和发送/接收的结构体一致。 */
    size_t item_size;
    /* 队列能缓存多少个元素。早期设小一些，便于发现生产/消费节奏问题。 */
    uint8_t length;
} rtos_queue_descriptor_t;

/* 任务/队列模型集中描述 FreeRTOS 资源，便于测试和后续移植时统一调整。
 * 查找函数返回 const 指针，调用者只能读取模型，不应该在运行时改这些描述。 */
size_t rtos_task_model_task_count(void);
const rtos_task_descriptor_t *rtos_task_model_task_at(size_t index);
const rtos_task_descriptor_t *rtos_task_model_find_task(rtos_task_id_t id);
size_t rtos_task_model_queue_count(void);
const rtos_queue_descriptor_t *rtos_task_model_queue_at(size_t index);
const rtos_queue_descriptor_t *rtos_task_model_find_queue(rtos_queue_id_t id);
bool rtos_task_model_is_valid(void);

#endif
