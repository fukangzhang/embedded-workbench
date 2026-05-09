#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/rtos_task_model.h"
#include "embedded_workbench/sensor_sample.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_size(size_t actual, size_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_u32(uint32_t actual, uint32_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

int main(void)
{
    const rtos_task_descriptor_t *task = 0;
    const rtos_queue_descriptor_t *queue = 0;

    /* 模型自检保证任务和队列描述表满足创建 FreeRTOS 资源的最低条件。 */
    if (expect_true(rtos_task_model_is_valid()) != 0) {
        return 1;
    }

    /* 当前架构规划了 4 个任务：采集、处理、通信、输出。 */
    if (expect_size(rtos_task_model_task_count(), 4u) != 0) {
        return 2;
    }

    /* env_process 是核心处理任务，测试固定它的名字、优先级和周期。 */
    task = rtos_task_model_find_task(RTOS_TASK_ENV_PROCESS);
    if (task == 0 ||
        expect_string(task->name, "env_process") != 0 ||
        expect_u32(task->priority, 4u) != 0 ||
        expect_u32(task->period_ms, 200u) != 0) {
        return 3;
    }

    task = rtos_task_model_task_at(99u);
    if (task != 0) {
        return 4;
    }

    if (expect_size(rtos_task_model_queue_count(), 4u) != 0) {
        return 5;
    }

    /* 队列元素大小必须和实际传输结构体一致，否则 FreeRTOS 复制消息时会出错。 */
    queue = rtos_task_model_find_queue(RTOS_QUEUE_SENSOR_SAMPLE);
    if (queue == 0 ||
        expect_string(queue->name, "sensor_sample_queue") != 0 ||
        expect_size(queue->item_size, sizeof(sensor_sample_t)) != 0) {
        return 6;
    }

    queue = rtos_task_model_find_queue(RTOS_QUEUE_COMMAND);
    if (queue == 0 ||
        expect_size(queue->item_size, sizeof(command_t)) != 0) {
        return 7;
    }

    queue = rtos_task_model_queue_at(99u);
    if (queue != 0) {
        return 8;
    }

    /* 越界访问返回 0，调用者可以安全判断“没找到”。 */
    return 0;
}
