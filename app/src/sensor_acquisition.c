#include "embedded_workbench/sensor_acquisition.h"

bool sensor_acquisition_init(
    sensor_acquisition_t *acquisition,
    const sensor_source_t *source,
    sensor_acquisition_submit_fn submit,
    void *submit_context)
{
    if (acquisition == 0 || source == 0 || submit == 0) {
        return false;
    }

    acquisition->source = source;
    acquisition->submit = submit;
    acquisition->submit_context = submit_context;

    return true;
}

bool sensor_acquisition_poll(sensor_acquisition_t *acquisition)
{
    sensor_sample_t sample;

    if (acquisition == 0 || acquisition->source == 0 || acquisition->submit == 0) {
        return false;
    }

    /* sensor_source_read already rejects null callbacks and out-of-range sensor values. */
    if (!sensor_source_read(acquisition->source, &sample)) {
        return false;
    }

    return acquisition->submit(acquisition->submit_context, &sample);
}
