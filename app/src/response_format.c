#include "embedded_workbench/response_format.h"

#include <stdint.h>

typedef struct {
    char *buffer;
    size_t buffer_size;
    size_t used;
    bool ok;
} response_writer_t;

static void writer_init(response_writer_t *writer, char *buffer, size_t buffer_size)
{
    writer->buffer = buffer;
    writer->buffer_size = buffer_size;
    writer->used = 0u;
    writer->ok = buffer != 0 && buffer_size > 0u;

    if (writer->ok) {
        writer->buffer[0] = '\0';
    }
}

static void writer_append_char(response_writer_t *writer, char value)
{
    if (!writer->ok) {
        return;
    }

    if (writer->used + 1u >= writer->buffer_size) {
        writer->ok = false;
        return;
    }

    writer->buffer[writer->used] = value;
    writer->used++;
    writer->buffer[writer->used] = '\0';
}

static void writer_append_string(response_writer_t *writer, const char *text)
{
    if (text == 0) {
        writer->ok = false;
        return;
    }

    while (*text != '\0') {
        writer_append_char(writer, *text);
        text++;
    }
}

static void writer_append_u32(response_writer_t *writer, uint32_t value)
{
    char digits[10];
    size_t count = 0u;

    if (value == 0u) {
        writer_append_char(writer, '0');
        return;
    }

    while (value > 0u && count < sizeof(digits)) {
        digits[count] = (char)('0' + (value % 10u));
        value /= 10u;
        count++;
    }

    while (count > 0u) {
        count--;
        writer_append_char(writer, digits[count]);
    }
}

static void writer_append_i32(response_writer_t *writer, int32_t value)
{
    uint32_t magnitude = 0u;

    if (value < 0) {
        writer_append_char(writer, '-');
        magnitude = (uint32_t)(-(value + 1)) + 1u;
    } else {
        magnitude = (uint32_t)value;
    }

    writer_append_u32(writer, magnitude);
}

bool response_format_result(
    char *buffer,
    size_t buffer_size,
    const command_handler_result_t *result)
{
    const char *prefix = "ERR";
    response_writer_t writer;

    if (result == 0) {
        return false;
    }

    writer_init(&writer, buffer, buffer_size);

    if (result->result == COMMAND_RESULT_OK) {
        prefix = "OK";
    }

    writer_append_string(&writer, prefix);
    writer_append_string(&writer, " result=");
    writer_append_string(&writer, command_result_name(result->result));
    writer_append_char(&writer, '\n');

    return writer.ok;
}

bool response_format_status(
    char *buffer,
    size_t buffer_size,
    alarm_state_t state,
    const sensor_sample_t *sample)
{
    response_writer_t writer;

    if (!sensor_sample_is_valid(sample)) {
        return false;
    }

    writer_init(&writer, buffer, buffer_size);
    writer_append_string(&writer, "STATUS state=");
    writer_append_string(&writer, alarm_state_name(state));
    writer_append_string(&writer, " temp_c_x10=");
    writer_append_i32(&writer, sample->temperature_c_x10);
    writer_append_string(&writer, " humidity_rh_x10=");
    writer_append_u32(&writer, sample->humidity_rh_x10);
    writer_append_string(&writer, " light_lux=");
    writer_append_u32(&writer, sample->light_lux);
    writer_append_string(&writer, " smoke_ppm=");
    writer_append_u32(&writer, sample->smoke_ppm);
    writer_append_char(&writer, '\n');

    return writer.ok;
}

bool response_format_config(
    char *buffer,
    size_t buffer_size,
    const alarm_config_t *config)
{
    response_writer_t writer;

    if (!alarm_config_is_valid(config)) {
        return false;
    }

    writer_init(&writer, buffer, buffer_size);
    writer_append_string(&writer, "CONFIG temp_warn=");
    writer_append_i32(&writer, config->temperature_warning_high_c_x10);
    writer_append_string(&writer, " temp_alarm=");
    writer_append_i32(&writer, config->temperature_alarm_high_c_x10);
    writer_append_string(&writer, " temp_recover=");
    writer_append_i32(&writer, config->temperature_recovery_high_c_x10);
    writer_append_string(&writer, " hum_warn=");
    writer_append_u32(&writer, config->humidity_warning_high_rh_x10);
    writer_append_string(&writer, " hum_alarm=");
    writer_append_u32(&writer, config->humidity_alarm_high_rh_x10);
    writer_append_string(&writer, " hum_recover=");
    writer_append_u32(&writer, config->humidity_recovery_high_rh_x10);
    writer_append_string(&writer, " light_warn_low=");
    writer_append_u32(&writer, config->light_warning_low_lux);
    writer_append_string(&writer, " light_alarm_low=");
    writer_append_u32(&writer, config->light_alarm_low_lux);
    writer_append_string(&writer, " light_recover_low=");
    writer_append_u32(&writer, config->light_recovery_low_lux);
    writer_append_string(&writer, " smoke_warn=");
    writer_append_u32(&writer, config->smoke_warning_ppm);
    writer_append_string(&writer, " smoke_alarm=");
    writer_append_u32(&writer, config->smoke_alarm_ppm);
    writer_append_string(&writer, " smoke_recover=");
    writer_append_u32(&writer, config->smoke_recovery_ppm);
    writer_append_char(&writer, '\n');

    return writer.ok;
}
