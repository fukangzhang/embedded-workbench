#include "embedded_workbench/sensor_source.h"

bool sensor_source_is_valid(const sensor_source_t *source)
{
    return source != 0 && source->read != 0;
}

bool sensor_source_read(const sensor_source_t *source, sensor_sample_t *sample_out)
{
    if (!sensor_source_is_valid(source) || sample_out == 0) {
        return false;
    }

    if (!source->read(source->context, sample_out)) {
        return false;
    }

    return sensor_sample_is_valid(sample_out);
}
