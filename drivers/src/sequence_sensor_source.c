#include "embedded_workbench/sequence_sensor_source.h"

bool sequence_sensor_source_init(
    sequence_sensor_source_t *sequence,
    const sensor_sample_t *samples,
    size_t count,
    bool repeat_last)
{
    if (sequence == 0 || samples == 0 || count == 0u) {
        return false;
    }

    sequence->samples = samples;
    sequence->count = count;
    sequence->index = 0u;
    sequence->repeat_last = repeat_last;
    return true;
}

sensor_source_t sequence_sensor_source_as_source(sequence_sensor_source_t *sequence)
{
    sensor_source_t source;

    source.read = sequence_sensor_source_read;
    source.context = sequence;
    return source;
}

bool sequence_sensor_source_read(void *context, sensor_sample_t *sample_out)
{
    sequence_sensor_source_t *sequence = (sequence_sensor_source_t *)context;
    size_t read_index = 0u;

    if (sequence == 0 ||
        sample_out == 0 ||
        sequence->samples == 0 ||
        sequence->count == 0u) {
        return false;
    }

    if (sequence->index >= sequence->count) {
        if (!sequence->repeat_last) {
            return false;
        }
        read_index = sequence->count - 1u;
    } else {
        read_index = sequence->index;
        sequence->index++;
    }

    *sample_out = sequence->samples[read_index];
    return true;
}
