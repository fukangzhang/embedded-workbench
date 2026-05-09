#include "embedded_workbench/command_session.h"

#include <stddef.h>

#include "embedded_workbench/command_handler.h"
#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/response_format.h"

typedef struct {
    /* response 可能需要由多段文本组成，例如 SET 成功后同时回：
     * OK、CONFIG、STATUS。session_writer_t 记录当前已经写到哪里。 */
    char *buffer;
    size_t capacity;
    size_t used;
    /* ok 一旦变成 false，表示缓冲区不足或 formatter 失败，后续追加会停止。 */
    bool ok;
} session_writer_t;

static size_t text_length(const char *text)
{
    size_t length = 0u;

    while (text[length] != '\0') {
        length++;
    }

    return length;
}

static void writer_init(session_writer_t *writer, char *buffer, size_t capacity)
{
    writer->buffer = buffer;
    writer->capacity = capacity;
    writer->used = 0u;
    writer->ok = buffer != 0 && capacity > 0u;

    if (writer->ok) {
        writer->buffer[0] = '\0';
    }
}

static bool writer_append_formatted(
    session_writer_t *writer,
    bool (*formatter)(char *, size_t, void *),
    void *context)
{
    size_t written = 0u;

    if (writer == 0 || !writer->ok || formatter == 0) {
        return false;
    }

    /* response_format_* 每次都会从当前写入位置开始生成一个完整 C 字符串。
     * 成功后再用 text_length 统计本次追加了多少字节。 */
    if (!formatter(&writer->buffer[writer->used], writer->capacity - writer->used, context)) {
        writer->ok = false;
        return false;
    }

    written = text_length(&writer->buffer[writer->used]);
    writer->used += written;
    return true;
}

static bool writer_append_text(session_writer_t *writer, const char *text)
{
    size_t length = 0u;
    size_t index = 0u;

    if (writer == 0 || !writer->ok || text == 0) {
        return false;
    }

    length = text_length(text);
    if (writer->used + length >= writer->capacity) {
        writer->ok = false;
        return false;
    }

    for (index = 0u; index < length; index++) {
        writer->buffer[writer->used + index] = text[index];
    }
    writer->used += length;
    writer->buffer[writer->used] = '\0';
    return true;
}

static bool format_result_adapter(char *buffer, size_t buffer_size, void *context)
{
    /* writer_append_formatted 需要统一的 void* formatter 形状。
     * adapter 只做类型转换，把真实 formatter 接到通用 writer 上。 */
    return response_format_result(buffer, buffer_size, (const command_handler_result_t *)context);
}

typedef struct {
    alarm_state_t state;
    const sensor_sample_t *sample;
} status_format_context_t;

static bool format_status_adapter(char *buffer, size_t buffer_size, void *context)
{
    status_format_context_t *status_context = (status_format_context_t *)context;

    if (status_context == 0) {
        return false;
    }

    return response_format_status(buffer, buffer_size, status_context->state, status_context->sample);
}

static bool format_config_adapter(char *buffer, size_t buffer_size, void *context)
{
    return response_format_config(buffer, buffer_size, (const alarm_config_t *)context);
}

static bool session_is_valid(const command_session_t *session)
{
    return session != 0 &&
           session->config != 0 &&
           session->state != 0 &&
           session->sample != 0;
}

bool command_session_handle_line(
    command_session_t *session,
    const char *line,
    char *response,
    size_t response_size)
{
    command_t command;
    command_handler_result_t result;
    session_writer_t writer;
    status_format_context_t status_context;

    if (!session_is_valid(session) || line == 0 || response == 0 || response_size == 0u) {
        return false;
    }

    writer_init(&writer, response, response_size);

    if (!command_parse(line, &command)) {
        /* 解析失败也走统一的响应格式，让串口上始终有可读反馈。 */
        result.result = COMMAND_RESULT_INVALID_COMMAND;
        result.config_changed = false;
        result.status_requested = false;
        result.config_requested = false;
        result.alarm_clear_requested = false;
        return writer_append_formatted(&writer, format_result_adapter, &result);
    }

    result = command_handler_handle(&command, session->config);
    if (!writer_append_formatted(&writer, format_result_adapter, &result)) {
        return false;
    }

    if (result.config_changed) {
        /* 配置改变后，当前样本对应的状态可能也会改变，所以立即重新计算状态。 */
        *session->state = alarm_state_update(*session->state, session->config, session->sample);
        if (!writer_append_formatted(&writer, format_config_adapter, session->config)) {
            return false;
        }

        status_context.state = *session->state;
        status_context.sample = session->sample;
        if (!writer_append_formatted(&writer, format_status_adapter, &status_context)) {
            return false;
        }
    }

    if (result.status_requested) {
        status_context.state = *session->state;
        status_context.sample = session->sample;
        if (!writer_append_formatted(&writer, format_status_adapter, &status_context)) {
            return false;
        }
    }

    if (result.config_requested) {
        if (!writer_append_formatted(&writer, format_config_adapter, session->config)) {
            return false;
        }
    }

    if (result.alarm_clear_requested) {
        /* 目前 CLEAR_ALARM 还没有真实锁存告警状态可清除。
         * 先保留明确响应，后续接入人工确认逻辑时可以在这里扩展。 */
        if (!writer_append_text(&writer, "clear_alarm=requested\n")) {
            return false;
        }
    }

    return writer.ok;
}
