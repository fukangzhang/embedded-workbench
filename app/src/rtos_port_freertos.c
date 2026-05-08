#include "embedded_workbench/rtos_port_freertos.h"

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

static bool freertos_start(void *context)
{
    /* Scheduler launch is deferred until real task creation is wired in. */
    return context_is_ready((const freertos_rtos_port_context_t *)context);
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
