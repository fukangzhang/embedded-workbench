#include "embedded_workbench/rtos_port_freertos.h"

#include "embedded_workbench/alarm_output.h"
#include "embedded_workbench/alarm_output_timing.h"
#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/command_responder.h"
#include "embedded_workbench/environment_processor.h"
#include "embedded_workbench/rtos_task_model.h"
#include "embedded_workbench/sensor_acquisition.h"

/* rtos_port_freertos 把项目自己的 rtos_port_t 接到 FreeRTOS。
 * 这个文件是“真实 RTOS 后端”，负责创建队列、创建任务、在任务间转发消息。 */

#define ALARM_OUTPUT_REFRESH_PERIOD_MS 50u

typedef struct {
    /* 目前输出任务只需要知道告警状态。
     * 之后如果要带时间戳、传感器摘要，也可以从这个事件结构扩展。 */
    alarm_state_t state;
} freertos_alarm_event_t;

static QueueHandle_t create_queue(rtos_queue_id_t id, size_t expected_item_size)
{
    const rtos_queue_descriptor_t *descriptor = rtos_task_model_find_queue(id);

    /* 创建队列前核对模型中的 item_size，避免发送方和接收方对消息大小理解不一致。 */
    if (descriptor == 0 ||
        descriptor->length == 0u ||
        descriptor->item_size != expected_item_size) {
        return 0;
    }

    return xQueueCreate((UBaseType_t)descriptor->length, (UBaseType_t)descriptor->item_size);
}

static bool context_is_ready(const freertos_rtos_port_context_t *context)
{
    /* FreeRTOS port 的最小可用条件：四条队列都已经创建好。
     * alarm_output_sink 可以为空，因为早期构建阶段允许只验证任务/队列链路。 */
    return context != 0 &&
           context->sensor_sample_queue != 0 &&
           context->command_queue != 0 &&
           context->response_queue != 0 &&
           context->alarm_event_queue != 0;
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

    /* 当前 tick 换算还很粗糙，先用毫秒数直接构建骨架；
     * 后续接真实板级 tick 配置时再引入 pdMS_TO_TICKS。 */
    vTaskDelay((TickType_t)period_ms);
}

static bool freertos_submit_sensor_sample(void *context, const sensor_sample_t *sample)
{
    freertos_rtos_port_context_t *freertos_context = (freertos_rtos_port_context_t *)context;

    if (freertos_context == 0 ||
        freertos_context->sensor_sample_queue == 0 ||
        sample == 0) {
        return false;
    }

    return xQueueSend(freertos_context->sensor_sample_queue, sample, (TickType_t)0) == pdPASS;
}

static void sensor_acquire_task(void *parameter)
{
    freertos_rtos_port_context_t *context = (freertos_rtos_port_context_t *)parameter;
    sensor_acquisition_t acquisition;
    bool acquisition_ready = false;

    if (context != 0) {
        acquisition_ready = sensor_acquisition_init(
            &acquisition,
            context->sensor_source,
            freertos_submit_sensor_sample,
            context);
    }

    for (;;) {
        /* source 是可选的：早期固件可以只验证任务创建；配置 source 后才真正产生 sample。 */
        if (acquisition_ready) {
            (void)sensor_acquisition_poll(&acquisition);
        }
        delay_for_descriptor(RTOS_TASK_SENSOR_ACQUIRE);
    }
}

static bool freertos_publish_alarm_event(void *context, alarm_state_t state)
{
    freertos_rtos_port_context_t *freertos_context = (freertos_rtos_port_context_t *)context;
    freertos_alarm_event_t event;

    if (freertos_context == 0 || freertos_context->alarm_event_queue == 0) {
        return false;
    }

    event.state = state;
    return xQueueSend(freertos_context->alarm_event_queue, &event, (TickType_t)0) == pdPASS;
}

static void env_process_task(void *parameter)
{
    freertos_rtos_port_context_t *context = (freertos_rtos_port_context_t *)parameter;
    sensor_sample_t sample;
    alarm_config_t config = alarm_config_default();
    environment_processor_t processor;
    bool processor_ready = false;

    if (context != 0) {
        processor_ready = environment_processor_init(
            &processor,
            &config,
            ALARM_STATE_NORMAL,
            freertos_publish_alarm_event,
            context);
    }

    for (;;) {
        /* 环境处理任务阻塞等待传感器采样，收到后更新状态并把状态事件发给输出任务。 */
        if (processor_ready &&
            context != 0 &&
            context->sensor_sample_queue != 0 &&
            xQueueReceive(context->sensor_sample_queue, &sample, portMAX_DELAY) == pdPASS) {
            (void)environment_processor_process_sample(&processor, &sample);
        } else if (!processor_ready) {
            delay_for_descriptor(RTOS_TASK_ENV_PROCESS);
        }
    }
}

static void communication_task(void *parameter)
{
    freertos_rtos_port_context_t *context = (freertos_rtos_port_context_t *)parameter;
    command_t command;
    rtos_response_message_t response = {{0}};
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_NORMAL;
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_responder_t responder;
    bool responder_ready = false;

    if (context != 0) {
        responder_ready = command_responder_init(&responder, &config, &state, &sample);
    }

    for (;;) {
        /* 通信任务处理已经解析好的 command_t，并把响应文本送回 response_queue。 */
        if (responder_ready &&
            context != 0 &&
            context->command_queue != 0 &&
            context->response_queue != 0 &&
            xQueueReceive(context->command_queue, &command, portMAX_DELAY) == pdPASS) {
            if (command_responder_handle_command(
                    &responder,
                    &command,
                    response.text,
                    sizeof(response.text))) {
                (void)xQueueSend(context->response_queue, &response, (TickType_t)0);
            }
        } else if (!responder_ready) {
            delay_for_descriptor(RTOS_TASK_COMMUNICATION);
        }
    }
}

static void alarm_output_task(void *parameter)
{
    freertos_rtos_port_context_t *context = (freertos_rtos_port_context_t *)parameter;
    freertos_alarm_event_t event;
    alarm_output_command_t command;
    alarm_output_command_t rendered_command;
    bool has_command = false;
    uint32_t elapsed_ms = 0u;
    bool indicator_is_on = false;
    BaseType_t receive_result = pdFAIL;

    for (;;) {
        if (context != 0 &&
            context->alarm_event_queue != 0) {
            /* 第一次没有命令时永久等待；已有命令后周期性超时，用超时机会刷新闪烁相位。 */
            receive_result = xQueueReceive(
                context->alarm_event_queue,
                &event,
                has_command ? (TickType_t)ALARM_OUTPUT_REFRESH_PERIOD_MS : portMAX_DELAY);

            if (receive_result == pdPASS) {
                /* 新状态事件会重新计算输出命令，并从 0 开始计时一个新的闪烁周期。 */
                has_command = alarm_output_command_for_state(event.state, &command);
                elapsed_ms = 0u;
            } else if (has_command) {
                elapsed_ms += ALARM_OUTPUT_REFRESH_PERIOD_MS;
            }

            if (context->alarm_output_sink != 0 &&
                has_command &&
                alarm_output_indicator_is_on(&command, elapsed_ms, &indicator_is_on)) {
                rendered_command = command;
                if (!indicator_is_on) {
                    /* 闪烁的“灭”阶段只关闭指示灯，不改变蜂鸣器和执行器策略。 */
                    rendered_command.indicator = ALARM_OUTPUT_INDICATOR_OFF;
                }
                (void)alarm_output_sink_apply(context->alarm_output_sink, &rendered_command);
            }
        }
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
    /* 任务参数来自集中模型；这里拒绝不完整的描述，避免创建出难以调试的 RTOS 任务。 */
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
    /* 对 rtos_port 来说 start 表示“任务创建完成”。
     * 是否真的启动调度器由 freertos_rtos_port_start_scheduler 显式控制。 */
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

    /* init 允许 context 中预先放入队列句柄，便于测试或后续接入静态分配队列。 */
    if (context->sensor_sample_queue == 0) {
        context->sensor_sample_queue = create_queue(RTOS_QUEUE_SENSOR_SAMPLE, sizeof(sensor_sample_t));
    }
    if (context->command_queue == 0) {
        context->command_queue = create_queue(RTOS_QUEUE_COMMAND, sizeof(command_t));
    }
    if (context->response_queue == 0) {
        context->response_queue = create_queue(RTOS_QUEUE_RESPONSE, sizeof(rtos_response_message_t));
    }
    if (context->alarm_event_queue == 0) {
        context->alarm_event_queue = create_queue(RTOS_QUEUE_ALARM_EVENT, sizeof(freertos_alarm_event_t));
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

bool freertos_rtos_port_start_scheduler(freertos_rtos_port_context_t *context)
{
    if (!tasks_are_created(context)) {
        return false;
    }

    vTaskStartScheduler();

    /* FreeRTOS 调度器正常启动后通常不会返回；返回到这里按失败处理。 */
    return false;
}
