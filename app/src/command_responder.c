#include "embedded_workbench/command_responder.h"

#include "embedded_workbench/command_handler.h"
#include "embedded_workbench/response_format.h"

typedef struct {
    char *buffer;
    size_t capacity;
    size_t used;
    bool ok;
} responder_writer_t;

typedef struct {
    alarm_state_t state;
    const sensor_sample_t *sample;
} status_format_context_t;

static size_t text_length(const char *text)
{
    size_t length = 0u;

    while (text[length] != '\0') {
        length++;
    }

    return length;
}

static void writer_init(responder_writer_t *writer, char *buffer, size_t capacity)
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
    responder_writer_t *writer,
    bool (*formatter)(char *, size_t, void *),
    void *context)
{
    size_t written = 0u;

    if (writer == 0 || !writer->ok || formatter == 0) {
        return false;
    }

    if (!formatter(&writer->buffer[writer->used], writer->capacity - writer->used, context)) {
        writer->ok = false;
        return false;
    }

    written = text_length(&writer->buffer[writer->used]);
    writer->used += written;

    return true;
}

static bool writer_append_text(responder_writer_t *writer, const char *text)
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
    return response_format_result(buffer, buffer_size, (const command_handler_result_t *)context);
}

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

static bool responder_is_ready(const command_responder_t *responder)
{
    return responder != 0 &&
           responder->config != 0 &&
           responder->state != 0 &&
           responder->sample != 0 &&
           alarm_config_is_valid(responder->config) &&
           sensor_sample_is_valid(responder->sample);
}

static void responder_status_clear(command_responder_status_t *status)
{
    if (status != 0) {
        status->config_changed = false;
        status->sample_changed = false;
    }
}

bool command_responder_init(
    command_responder_t *responder,
    alarm_config_t *config,
    alarm_state_t *state,
    sensor_sample_t *sample)
{
    if (responder == 0 || config == 0 || state == 0 || sample == 0) {
        return false;
    }
    if (!alarm_config_is_valid(config) || !sensor_sample_is_valid(sample)) {
        return false;
    }

    responder->config = config;
    responder->state = state;
    responder->sample = sample;

    return true;
}

bool command_responder_handle_command(
    command_responder_t *responder,
    const command_t *command,
    char *response,
    size_t response_size)
{
    return command_responder_handle_command_with_status(responder, command, response, response_size, 0);
}

bool command_responder_handle_command_with_status(
    command_responder_t *responder,
    const command_t *command,
    char *response,
    size_t response_size,
    command_responder_status_t *status)
{
    command_handler_result_t result;
    responder_writer_t writer;
    status_format_context_t status_context;

    responder_status_clear(status);

    if (!responder_is_ready(responder) || response == 0 || response_size == 0u) {
        return false;
    }

    writer_init(&writer, response, response_size);

    result = command_handler_handle(command, responder->config, responder->sample);
    if (status != 0) {
        status->config_changed = result.config_changed;
        status->sample_changed = result.sample_changed;
    }

    if (!writer_append_formatted(&writer, format_result_adapter, &result)) {
        return false;
    }

    if (result.config_changed || result.sample_changed) {
        *responder->state = alarm_state_update(*responder->state, responder->config, responder->sample);
    }

    if (result.config_changed) {
        if (!writer_append_formatted(&writer, format_config_adapter, responder->config)) {
            return false;
        }

        status_context.state = *responder->state;
        status_context.sample = responder->sample;
        if (!writer_append_formatted(&writer, format_status_adapter, &status_context)) {
            return false;
        }
    }

    if (result.sample_changed || result.status_requested) {
        status_context.state = *responder->state;
        status_context.sample = responder->sample;
        if (!writer_append_formatted(&writer, format_status_adapter, &status_context)) {
            return false;
        }
    }

    if (result.config_requested) {
        if (!writer_append_formatted(&writer, format_config_adapter, responder->config)) {
            return false;
        }
    }

    if (result.alarm_clear_requested) {
        if (!writer_append_text(&writer, "clear_alarm=requested\n")) {
            return false;
        }
    }

    return writer.ok;
}
