#include "embedded_workbench/rtos_task_model.h"

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/rtos_port.h"
#include "embedded_workbench/sensor_sample.h"

/* rtos_task_model 是 RTOS 设计的“清单”。
 * 它不创建真实 FreeRTOS 任务，只集中记录要创建哪些任务、队列参数是什么。 */

typedef struct {
    /* 告警事件当前只携带状态，后续可以扩展时间戳或样本摘要。 */
    alarm_state_t state;
} alarm_event_t;

static const rtos_task_descriptor_t tasks[] = {
    /* 优先级数值越大通常表示越高优先级；这里让环境处理略高于通信和输出骨架。 */
    {RTOS_TASK_SENSOR_ACQUIRE, "sensor_acquire", 3u, 384u, 1000u},
    {RTOS_TASK_ENV_PROCESS, "env_process", 4u, 512u, 200u},
    {RTOS_TASK_COMMUNICATION, "communication", 2u, 768u, 20u},
    {RTOS_TASK_ALARM_OUTPUT, "alarm_output", 3u, 384u, 100u},
};

static const rtos_queue_descriptor_t queues[] = {
    /* 队列长度先保持很小，便于早期测试暴露生产/消费节奏问题。 */
    {RTOS_QUEUE_SENSOR_SAMPLE, "sensor_sample_queue", sizeof(sensor_sample_t), 4u},
    {RTOS_QUEUE_COMMAND, "command_queue", sizeof(command_t), 4u},
    {RTOS_QUEUE_RESPONSE, "response_queue", sizeof(rtos_response_message_t), 4u},
    {RTOS_QUEUE_ALARM_EVENT, "alarm_event_queue", sizeof(alarm_event_t), 4u},
    {RTOS_QUEUE_CONFIG_UPDATE, "config_update_queue", sizeof(alarm_config_t), 2u},
};

size_t rtos_task_model_task_count(void)
{
    return sizeof(tasks) / sizeof(tasks[0]);
}

const rtos_task_descriptor_t *rtos_task_model_task_at(size_t index)
{
    if (index >= rtos_task_model_task_count()) {
        return 0;
    }

    return &tasks[index];
}

const rtos_task_descriptor_t *rtos_task_model_find_task(rtos_task_id_t id)
{
    size_t index = 0;

    /* 用线性查找足够：任务数量很少，代码简单比过早优化更重要。 */
    for (index = 0; index < rtos_task_model_task_count(); index++) {
        if (tasks[index].id == id) {
            return &tasks[index];
        }
    }

    return 0;
}

size_t rtos_task_model_queue_count(void)
{
    return sizeof(queues) / sizeof(queues[0]);
}

const rtos_queue_descriptor_t *rtos_task_model_queue_at(size_t index)
{
    if (index >= rtos_task_model_queue_count()) {
        return 0;
    }

    return &queues[index];
}

const rtos_queue_descriptor_t *rtos_task_model_find_queue(rtos_queue_id_t id)
{
    size_t index = 0;

    /* 队列数量同样很少，保持直观线性查找即可。 */
    for (index = 0; index < rtos_task_model_queue_count(); index++) {
        if (queues[index].id == id) {
            return &queues[index];
        }
    }

    return 0;
}

bool rtos_task_model_is_valid(void)
{
    size_t index = 0;

    for (index = 0; index < rtos_task_model_task_count(); index++) {
        /* 模型自检关注“能否创建任务”的最低要求，不判断业务参数是否最优。 */
        if (tasks[index].name == 0 ||
            tasks[index].priority == 0u ||
            tasks[index].stack_words == 0u ||
            tasks[index].period_ms == 0u) {
            return false;
        }
    }

    for (index = 0; index < rtos_task_model_queue_count(); index++) {
        /* 队列至少要有名字、元素大小和长度，FreeRTOS 创建时才有明确参数。 */
        if (queues[index].name == 0 ||
            queues[index].item_size == 0u ||
            queues[index].length == 0u) {
            return false;
        }
    }

    return true;
}
