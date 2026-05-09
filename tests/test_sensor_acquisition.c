#include <stdbool.h>

#include "embedded_workbench/sensor_acquisition.h"
#include "embedded_workbench/sequence_sensor_source.h"

typedef struct {
    unsigned int count;
    sensor_sample_t last_sample;
    bool fail_next_submit;
} fake_submit_context_t;

static bool fake_submit(void *context, const sensor_sample_t *sample)
{
    fake_submit_context_t *fake = (fake_submit_context_t *)context;

    if (fake == 0 || sample == 0 || fake->fail_next_submit) {
        return false;
    }

    fake->last_sample = *sample;
    fake->count++;

    return true;
}

static bool null_context_submit(void *context, const sensor_sample_t *sample)
{
    (void)context;

    return sample != 0;
}

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_uint(unsigned int actual, unsigned int expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_int(int actual, int expected)
{
    return actual == expected ? 0 : 1;
}

int main(void)
{
    sensor_sample_t samples[] = {
        sensor_sample_make(250, 500u, 300u, 20u),
        sensor_sample_make(360, 650u, 120u, 80u),
    };
    sensor_sample_t invalid_samples[] = {
        sensor_sample_make(SENSOR_TEMPERATURE_MAX_C_X10 + 1, 500u, 300u, 20u),
    };
    sequence_sensor_source_t sequence;
    sequence_sensor_source_t invalid_sequence;
    sensor_source_t source;
    sensor_source_t invalid_source;
    sensor_acquisition_t acquisition;
    fake_submit_context_t submit_context = {0u, {0}, false};

    if (expect_true(sequence_sensor_source_init(&sequence, samples, 2u, false)) != 0) {
        return 1;
    }
    source = sequence_sensor_source_as_source(&sequence);

    if (expect_true(sensor_acquisition_init(&acquisition, &source, fake_submit, &submit_context)) != 0) {
        return 2;
    }

    if (expect_true(sensor_acquisition_poll(&acquisition)) != 0 ||
        expect_uint(submit_context.count, 1u) != 0 ||
        expect_int(submit_context.last_sample.temperature_c_x10, 250) != 0) {
        return 3;
    }

    if (expect_true(sensor_acquisition_poll(&acquisition)) != 0 ||
        expect_uint(submit_context.count, 2u) != 0 ||
        expect_int(submit_context.last_sample.temperature_c_x10, 360) != 0) {
        return 4;
    }

    if (expect_false(sensor_acquisition_poll(&acquisition)) != 0 ||
        expect_uint(submit_context.count, 2u) != 0) {
        return 5;
    }

    if (expect_true(sequence_sensor_source_init(&sequence, samples, 2u, false)) != 0) {
        return 6;
    }
    source = sequence_sensor_source_as_source(&sequence);
    submit_context.count = 0u;
    submit_context.fail_next_submit = true;

    if (expect_true(sensor_acquisition_init(&acquisition, &source, fake_submit, &submit_context)) != 0 ||
        expect_false(sensor_acquisition_poll(&acquisition)) != 0 ||
        expect_uint(submit_context.count, 0u) != 0) {
        return 7;
    }

    if (expect_true(sequence_sensor_source_init(&invalid_sequence, invalid_samples, 1u, false)) != 0) {
        return 8;
    }
    invalid_source = sequence_sensor_source_as_source(&invalid_sequence);
    submit_context.fail_next_submit = false;

    if (expect_true(sensor_acquisition_init(&acquisition, &invalid_source, fake_submit, &submit_context)) != 0 ||
        expect_false(sensor_acquisition_poll(&acquisition)) != 0 ||
        expect_uint(submit_context.count, 0u) != 0) {
        return 9;
    }

    if (expect_false(sensor_acquisition_init(0, &source, fake_submit, &submit_context)) != 0 ||
        expect_false(sensor_acquisition_init(&acquisition, 0, fake_submit, &submit_context)) != 0 ||
        expect_false(sensor_acquisition_init(&acquisition, &source, 0, &submit_context)) != 0 ||
        expect_false(sensor_acquisition_poll(0)) != 0) {
        return 10;
    }

    if (expect_true(sensor_acquisition_init(&acquisition, &source, null_context_submit, 0)) != 0) {
        return 11;
    }

    return 0;
}
