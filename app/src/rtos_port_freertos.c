#include "embedded_workbench/rtos_port_freertos.h"

#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/rtos_task_model.h"

static QueueHandle_t create_queue(rtos_queue_id_t id, size_t expected_item_size)
{
    const rtos_queue_descriptor_t *descriptor = rtos_task_model_find_queue(id);

    if (descriptor == 0 ||
        descriptor->length == 0u ||
        descriptor->item_size != expected_item_size) {
        return 0;
    }

    return xQueueCreate((UBaseType_t)descriptor->length, (UBaseType_t)descriptor->item_size);
}

static bool context_is_ready(const freertos_rtos_port_context_t *context)
{
    return context != 0 &&
           context->sensor_sample_queue != 0 &&
           context->command_queue != 0 &&
           context->response_queue != 0;
}

static bool tasks_are_created(const freertos_rtos_port_context_t *context)
{
    return context != 0 &&
           context->sensor_acquire_task != 0 &&
           context->env_process_task != 0 &&
           context->communication_task != 0 &&
           context->alarm_output_task != 0;
}

static void delay_for_descriptor(rtos_task_id_t id)
{
    const rtos_task_descriptor_t *descriptor = rtos_task_model_find_task(id);
    uint32_t period_ms = 1000u;

    if (descriptor != 0 && descriptor->period_ms > 0u) {
        period_ms = descriptor->period_ms;
    }

    vTaskDelay((TickType_t)period_ms);
}

static void sensor_acquire_task(void *parameter)
{
    freertos_rtos_port_context_t *context = (freertos_rtos_port_context_t *)parameter;

    for (;;) {
        (void)context;
        delay_for_descriptor(RTOS_TASK_SENSOR_ACQUIRE);
    }
}

static void env_process_task(void *parameter)
{
    freertos_rtos_port_context_t *context = (freertos_rtos_port_context_t *)parameter;
    sensor_sample_t sample;
    alarm_state_t state = ALARM_STATE_NORMAL;
    alarm_config_t config = alarm_config_default();

    for (;;) {
        if (context != 0 &&
            context->sensor_sample_queue != 0 &&
            xQueueReceive(context->sensor_sample_queue, &sample, portMAX_DELAY) == pdPASS) {
            state = alarm_state_update(state, &config, &sample);
        }
    }
}

static void communication_task(void *parameter)
{
    freertos_rtos_port_context_t *context = (freertos_rtos_port_context_t *)parameter;
    command_t command;
    rtos_response_message_t response = {{0}};

    for (;;) {
        if (context != 0 &&
            context->command_queue != 0 &&
            context->response_queue != 0 &&
            xQueueReceive(context->command_queue, &command, portMAX_DELAY) == pdPASS) {
            response.text[0] = 'O';
            response.text[1] = 'K';
            response.text[2] = '\0';
            (void)xQueueSend(context->response_queue, &response, (TickType_t)0);
        }
    }
}

static void alarm_output_task(void *parameter)
{
    freertos_rtos_port_context_t *context = (freertos_rtos_port_context_t *)parameter;

    for (;;) {
        (void)context;
        delay_for_descriptor(RTOS_TASK_ALARM_OUTPUT);
    }
}

static bool create_task_from_model(
    freertos_rtos_port_context_t *context,
    rtos_task_id_t id,
    TaskFunction_t function,
    TaskHandle_t *handle)
{
    const rtos_task_descriptor_t *descriptor = rtos_task_model_find_task(id);

    if (context == 0 || function == 0 || handle == 0) {
        return false;
    }
    if (*handle != 0) {
        return true;
    }
    if (descriptor == 0 ||
        descriptor->name == 0 ||
        descriptor->stack_words == 0u ||
        descriptor->priority == 0u) {
        return false;
    }

    return xTaskCreate(
               function,
               descriptor->name,
               (configSTACK_DEPTH_TYPE)descriptor->stack_words,
               context,
               (UBaseType_t)descriptor->priority,
               handle) == pdPASS;
}

static bool create_tasks(freertos_rtos_port_context_t *context)
{
    if (!context_is_ready(context)) {
        return false;
    }
    if (tasks_are_created(context)) {
        return true;
    }

    return create_task_from_model(context, RTOS_TASK_SENSOR_ACQUIRE, sensor_acquire_task, &context->sensor_acquire_task) &&
           create_task_from_model(context, RTOS_TASK_ENV_PROCESS, env_process_task, &context->env_process_task) &&
           create_task_from_model(context, RTOS_TASK_COMMUNICATION, communication_task, &context->communication_task) &&
           create_task_from_model(context, RTOS_TASK_ALARM_OUTPUT, alarm_output_task, &context->alarm_output_task);
}

static bool freertos_start(void *context)
{
    return create_tasks((freertos_rtos_port_context_t *)context);
}

static bool freertos_send_sensor_sample(void *context, const sensor_sample_t *sample)
{
    freertos_rtos_port_context_t *freertos_context = (freertos_rtos_port_context_t *)context;

    if (!context_is_ready(freertos_context) || sample == 0) {
        return false;
    }

    return xQueueSend(freertos_context->sensor_sample_queue, sample, (TickType_t)0) == pdPASS;
}

static bool freertos_send_command(void *context, const command_t *command)
{
    freertos_rtos_port_context_t *freertos_context = (freertos_rtos_port_context_t *)context;

    if (!context_is_ready(freertos_context) || command == 0) {
        return false;
    }

    return xQueueSend(freertos_context->command_queue, command, (TickType_t)0) == pdPASS;
}

static bool freertos_receive_response(void *context, rtos_response_message_t *response)
{
    freertos_rtos_port_context_t *freertos_context = (freertos_rtos_port_context_t *)context;

    if (!context_is_ready(freertos_context) || response == 0) {
        return false;
    }

    return xQueueReceive(freertos_context->response_queue, response, (TickType_t)0) == pdPASS;
}

bool freertos_rtos_port_init(rtos_port_t *port, freertos_rtos_port_context_t *context)
{
    static const rtos_port_ops_t freertos_ops = {
        freertos_start,
        freertos_send_sensor_sample,
        freertos_send_command,
        freertos_receive_response,
    };

    if (port == 0 || context == 0) {
        return false;
    }

    if (context->sensor_sample_queue == 0) {
        context->sensor_sample_queue = create_queue(RTOS_QUEUE_SENSOR_SAMPLE, sizeof(sensor_sample_t));
    }
    if (context->command_queue == 0) {
        context->command_queue = create_queue(RTOS_QUEUE_COMMAND, sizeof(command_t));
    }
    if (context->response_queue == 0) {
        context->response_queue = create_queue(RTOS_QUEUE_RESPONSE, sizeof(rtos_response_message_t));
    }

    if (!context_is_ready(context)) {
        return false;
    }

    port->ops = &freertos_ops;
    port->context = context;
    return true;
}

bool freertos_rtos_port_send_response(freertos_rtos_port_context_t *context, const rtos_response_message_t *response)
{
    if (!context_is_ready(context) || response == 0) {
        return false;
    }

    return xQueueSend(context->response_queue, response, (TickType_t)0) == pdPASS;
}
