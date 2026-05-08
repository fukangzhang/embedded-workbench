#ifndef EMBEDDED_WORKBENCH_RTOS_TASK_MODEL_H
#define EMBEDDED_WORKBENCH_RTOS_TASK_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    RTOS_TASK_SENSOR_ACQUIRE = 0,
    RTOS_TASK_ENV_PROCESS = 1,
    RTOS_TASK_COMMUNICATION = 2,
    RTOS_TASK_ALARM_OUTPUT = 3
} rtos_task_id_t;

typedef enum {
    RTOS_QUEUE_SENSOR_SAMPLE = 0,
    RTOS_QUEUE_COMMAND = 1,
    RTOS_QUEUE_RESPONSE = 2,
    RTOS_QUEUE_ALARM_EVENT = 3
} rtos_queue_id_t;

typedef struct {
    rtos_task_id_t id;
    const char *name;
    uint8_t priority;
    uint16_t stack_words;
    uint32_t period_ms;
} rtos_task_descriptor_t;

typedef struct {
    rtos_queue_id_t id;
    const char *name;
    size_t item_size;
    uint8_t length;
} rtos_queue_descriptor_t;

size_t rtos_task_model_task_count(void);
const rtos_task_descriptor_t *rtos_task_model_task_at(size_t index);
const rtos_task_descriptor_t *rtos_task_model_find_task(rtos_task_id_t id);
size_t rtos_task_model_queue_count(void);
const rtos_queue_descriptor_t *rtos_task_model_queue_at(size_t index);
const rtos_queue_descriptor_t *rtos_task_model_find_queue(rtos_queue_id_t id);
bool rtos_task_model_is_valid(void);

#endif
