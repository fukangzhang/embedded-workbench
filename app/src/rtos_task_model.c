#include "embedded_workbench/rtos_task_model.h"

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/sensor_sample.h"

typedef struct {
    alarm_state_t state;
} alarm_event_t;

typedef struct {
    char text[128];
} response_message_t;

static const rtos_task_descriptor_t tasks[] = {
    {RTOS_TASK_SENSOR_ACQUIRE, "sensor_acquire", 3u, 384u, 1000u},
    {RTOS_TASK_ENV_PROCESS, "env_process", 4u, 512u, 200u},
    {RTOS_TASK_COMMUNICATION, "communication", 2u, 768u, 20u},
    {RTOS_TASK_ALARM_OUTPUT, "alarm_output", 3u, 384u, 100u},
};

static const rtos_queue_descriptor_t queues[] = {
    {RTOS_QUEUE_SENSOR_SAMPLE, "sensor_sample_queue", sizeof(sensor_sample_t), 4u},
    {RTOS_QUEUE_COMMAND, "command_queue", sizeof(command_t), 4u},
    {RTOS_QUEUE_RESPONSE, "response_queue", sizeof(response_message_t), 4u},
    {RTOS_QUEUE_ALARM_EVENT, "alarm_event_queue", sizeof(alarm_event_t), 4u},
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
        if (tasks[index].name == 0 ||
            tasks[index].priority == 0u ||
            tasks[index].stack_words == 0u ||
            tasks[index].period_ms == 0u) {
            return false;
        }
    }

    for (index = 0; index < rtos_task_model_queue_count(); index++) {
        if (queues[index].name == 0 ||
            queues[index].item_size == 0u ||
            queues[index].length == 0u) {
            return false;
        }
    }

    return true;
}
