#include <stdbool.h>

#include "embedded_workbench/sequence_sensor_source.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_int(int actual, int expected)
{
    return actual == expected ? 0 : 1;
}

static int test_reads_finite_sequence(void)
{
    sensor_sample_t samples[] = {
        {250, 500u, 300u, 20u},
        {360, 600u, 250u, 20u},
    };
    sequence_sensor_source_t sequence;
    sensor_source_t source;
    sensor_sample_t sample = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(sequence_sensor_source_init(&sequence, samples, 2u, false)) != 0) {
        return 1;
    }

    source = sequence_sensor_source_as_source(&sequence);

    if (expect_true(sensor_source_read(&source, &sample)) != 0 ||
        expect_int(sample.temperature_c_x10, 250) != 0 ||
        expect_int((int)sequence.index, 1) != 0) {
        return 2;
    }

    if (expect_true(sensor_source_read(&source, &sample)) != 0 ||
        expect_int(sample.temperature_c_x10, 360) != 0 ||
        expect_int((int)sequence.index, 2) != 0) {
        return 3;
    }

    if (expect_false(sensor_source_read(&source, &sample)) != 0 ||
        expect_int((int)sequence.index, 2) != 0) {
        return 4;
    }

    return 0;
}

static int test_can_repeat_last_sample(void)
{
    sensor_sample_t samples[] = {
        {250, 500u, 300u, 20u},
        {370, 640u, 280u, 650u},
    };
    sequence_sensor_source_t sequence;
    sensor_source_t source;
    sensor_sample_t sample = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(sequence_sensor_source_init(&sequence, samples, 2u, true)) != 0) {
        return 1;
    }

    source = sequence_sensor_source_as_source(&sequence);

    if (expect_true(sensor_source_read(&source, &sample)) != 0 ||
        expect_true(sensor_source_read(&source, &sample)) != 0 ||
        expect_true(sensor_source_read(&source, &sample)) != 0 ||
        expect_int(sample.temperature_c_x10, 370) != 0 ||
        expect_int(sample.smoke_ppm, 650) != 0 ||
        expect_int((int)sequence.index, 2) != 0) {
        return 2;
    }

    return 0;
}

static int test_rejects_invalid_setup_and_samples(void)
{
    sensor_sample_t invalid_samples[] = {
        {SENSOR_TEMPERATURE_MAX_C_X10 + 1, 500u, 300u, 20u},
    };
    sequence_sensor_source_t sequence;
    sensor_source_t source;
    sensor_sample_t sample = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_false(sequence_sensor_source_init(0, invalid_samples, 1u, false)) != 0 ||
        expect_false(sequence_sensor_source_init(&sequence, 0, 1u, false)) != 0 ||
        expect_false(sequence_sensor_source_init(&sequence, invalid_samples, 0u, false)) != 0) {
        return 1;
    }

    if (expect_true(sequence_sensor_source_init(&sequence, invalid_samples, 1u, false)) != 0) {
        return 2;
    }

    source = sequence_sensor_source_as_source(&sequence);
    if (expect_false(sensor_source_read(&source, &sample)) != 0 ||
        expect_int((int)sequence.index, 1) != 0) {
        return 3;
    }

    if (expect_false(sequence_sensor_source_read(0, &sample)) != 0 ||
        expect_false(sequence_sensor_source_read(&sequence, 0)) != 0) {
        return 4;
    }

    return 0;
}

int main(void)
{
    if (test_reads_finite_sequence() != 0) {
        return 1;
    }
    if (test_can_repeat_last_sample() != 0) {
        return 2;
    }
    if (test_rejects_invalid_setup_and_samples() != 0) {
        return 3;
    }

    return 0;
}
