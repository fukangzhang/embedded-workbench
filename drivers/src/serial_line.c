#include "embedded_workbench/serial_line.h"

static bool is_line_end(char byte)
{
    return byte == '\n' || byte == '\r';
}

bool serial_line_reader_init(serial_line_reader_t *reader, char *buffer, size_t capacity)
{
    if (reader == 0 || buffer == 0 || capacity == 0u) {
        return false;
    }

    reader->buffer = buffer;
    reader->capacity = capacity;
    reader->length = 0u;
    reader->previous_was_cr = false;
    reader->buffer[0] = '\0';

    return true;
}

void serial_line_reader_reset(serial_line_reader_t *reader)
{
    if (reader == 0 || reader->buffer == 0 || reader->capacity == 0u) {
        return;
    }

    reader->length = 0u;
    reader->previous_was_cr = false;
    reader->buffer[0] = '\0';
}

size_t serial_line_reader_length(const serial_line_reader_t *reader)
{
    if (reader == 0) {
        return 0u;
    }

    return reader->length;
}

static serial_line_status_t complete_line(
    serial_line_reader_t *reader,
    char *line_out,
    size_t line_out_size,
    size_t *line_length_out)
{
    size_t index = 0u;

    /* 空行不交给命令解析器，避免用户多按一次 Enter 就产生 invalid command。 */
    if (reader->length == 0u) {
        *line_length_out = 0u;
        return SERIAL_LINE_STATUS_IDLE;
    }

    /* 输出缓冲区必须能放下完整内容和末尾 '\0'。 */
    if (line_out_size <= reader->length) {
        serial_line_reader_reset(reader);
        *line_length_out = 0u;
        return SERIAL_LINE_STATUS_OVERFLOW;
    }

    for (index = 0u; index < reader->length; index++) {
        line_out[index] = reader->buffer[index];
    }
    line_out[reader->length] = '\0';
    *line_length_out = reader->length;
    serial_line_reader_reset(reader);

    return SERIAL_LINE_STATUS_LINE_READY;
}

serial_line_status_t serial_line_feed(
    serial_line_reader_t *reader,
    char byte,
    char *line_out,
    size_t line_out_size,
    size_t *line_length_out)
{
    if (reader == 0 ||
        reader->buffer == 0 ||
        reader->capacity == 0u ||
        line_out == 0 ||
        line_out_size == 0u ||
        line_length_out == 0) {
        return SERIAL_LINE_STATUS_ERROR;
    }

    *line_length_out = 0u;

    if (is_line_end(byte)) {
        if (byte == '\n' && reader->previous_was_cr && reader->length == 0u) {
            reader->previous_was_cr = false;
            return SERIAL_LINE_STATUS_IDLE;
        }

        reader->previous_was_cr = byte == '\r';
        return complete_line(reader, line_out, line_out_size, line_length_out);
    }

    reader->previous_was_cr = false;

    /* 内部缓冲区同样要预留 '\0'，这样调试时随时能把 buffer 当 C 字符串看。 */
    if (reader->length + 1u >= reader->capacity) {
        serial_line_reader_reset(reader);
        return SERIAL_LINE_STATUS_OVERFLOW;
    }

    reader->buffer[reader->length] = byte;
    reader->length++;
    reader->buffer[reader->length] = '\0';

    return SERIAL_LINE_STATUS_IDLE;
}
