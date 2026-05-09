#include <stdbool.h>

#include "embedded_workbench/filtered_sensor_source.h"
#include "embedded_workbench/sequence_sensor_source.h"

typedef struct {
    bool should_succeed;
    sensor_sample_t sample;
    unsigned int read_count;
} fake_sensor_source_t;

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

static int expect_sample(const sensor_sample_t *actual, const sensor_sample_t *expected)
{
    if (actual == 0 || expected == 0) {
        return 1;
    }

    return actual->temperature_c_x10 == expected->temperature_c_x10 &&
                   actual->humidity_rh_x10 == expected->humidity_rh_x10 &&
                   actual->light_lux == expected->light_lux &&
                   actual->smoke_ppm == expected->smoke_ppm
               ? 0
               : 1;
}

static bool fake_sensor_source_read(void *context, sensor_sample_t *sample_out)
{
    fake_sensor_source_t *fake = (fake_sensor_source_t *)context;

    if (fake == 0 || sample_out == 0) {
        return false;
    }

    fake->read_count++;
    if (!fake->should_succeed) {
        return false;
    }

    *sample_out = fake->sample;
    return true;
}

static int test_rejects_invalid_setup(void)
{
    filtered_sensor_source_t filtered;
    fake_sensor_source_t fake = {true, {250, 500u, 300u, 20u}, 0u};
    sensor_source_t inner = {fake_sensor_source_read, &fake};

    if (expect_false(filtered_sensor_source_init(0, &inner, 1u, 2u)) != 0 ||
        expect_false(filtered_sensor_source_init(&filtered, 0, 1u, 2u)) != 0 ||
        expect_false(filtered_sensor_source_init(&filtered, &inner, 0u, 2u)) != 0 ||
        expect_false(filtered_sensor_source_init(&filtered, &inner, 3u, 2u)) != 0) {
        return 1;
    }

    inner.read = 0;
    if (expect_false(filtered_sensor_source_init(&filtered, &inner, 1u, 2u)) != 0) {
        return 2;
    }

    return 0;
}

static int test_first_sample_passes_through_sequence_source(void)
{
    sensor_sample_t samples[] = {
        {250, 500u, 300u, 20u},
        {450, 700u, 500u, 60u},
    };
    sequence_sensor_source_t sequence;
    sensor_source_t inner;
    filtered_sensor_source_t filtered;
    sensor_source_t source;
    sensor_sample_t output = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(sequence_sensor_source_init(&sequence, samples, 2u, false)) != 0) {
        return 1;
    }

    inner = sequence_sensor_source_as_source(&sequence);
    if (expect_true(filtered_sensor_source_init(&filtered, &inner, 1u, 2u)) != 0) {
        return 2;
    }

    source = filtered_sensor_source_as_source(&filtered);
    if (expect_true(sensor_source_read(&source, &output)) != 0 ||
        expect_sample(&output, &samples[0]) != 0 ||
        expect_int((int)sequence.index, 1) != 0) {
        return 3;
    }

    return 0;
}

static int test_second_sample_is_filtered(void)
{
    sensor_sample_t samples[] = {
        {200, 400u, 100u, 10u},
        {400, 800u, 500u, 50u},
    };
    sensor_sample_t expected = {300, 600u, 300u, 30u};
    sequence_sensor_source_t sequence;
    sensor_source_t inner;
    filtered_sensor_source_t filtered;
    sensor_source_t source;
    sensor_sample_t output = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(sequence_sensor_source_init(&sequence, samples, 2u, false)) != 0) {
        return 1;
    }

    inner = sequence_sensor_source_as_source(&sequence);
    if (expect_true(filtered_sensor_source_init(&filtered, &inner, 1u, 2u)) != 0) {
        return 2;
    }

    source = filtered_sensor_source_as_source(&filtered);
    if (expect_true(sensor_source_read(&source, &output)) != 0 ||
        expect_true(sensor_source_read(&source, &output)) != 0 ||
        expect_sample(&output, &expected) != 0) {
        return 3;
    }

    return 0;
}

static int test_reset_makes_next_sample_pass_through(void)
{
    sensor_sample_t samples[] = {
        {200, 400u, 100u, 10u},
        {400, 800u, 500u, 50u},
        {360, 700u, 900u, 90u},
    };
    sequence_sensor_source_t sequence;
    sensor_source_t inner;
    filtered_sensor_source_t filtered;
    sensor_source_t source;
    sensor_sample_t output = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(sequence_sensor_source_init(&sequence, samples, 3u, false)) != 0) {
        return 1;
    }

    inner = sequence_sensor_source_as_source(&sequence);
    if (expect_true(filtered_sensor_source_init(&filtered, &inner, 1u, 2u)) != 0) {
        return 2;
    }

    source = filtered_sensor_source_as_source(&filtered);
    if (expect_true(sensor_source_read(&source, &output)) != 0 ||
        expect_true(sensor_source_read(&source, &output)) != 0) {
        return 3;
    }

    filtered_sensor_source_reset(&filtered);

    if (expect_true(sensor_source_read(&source, &output)) != 0 ||
        expect_sample(&output, &samples[2]) != 0) {
        return 4;
    }

    filtered_sensor_source_reset(0);
    return 0;
}

static int test_inner_read_failure_is_preserved(void)
{
    filtered_sensor_source_t filtered;
    fake_sensor_source_t fake = {false, {250, 500u, 300u, 20u}, 0u};
    sensor_source_t inner = {fake_sensor_source_read, &fake};
    sensor_source_t source;
    sensor_sample_t output = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(filtered_sensor_source_init(&filtered, &inner, 1u, 2u)) != 0) {
        return 1;
    }

    source = filtered_sensor_source_as_source(&filtered);
    if (expect_false(sensor_source_read(&source, &output)) != 0 ||
        expect_int((int)fake.read_count, 1) != 0) {
        return 2;
    }

    if (expect_false(filtered_sensor_source_read(0, &output)) != 0 ||
        expect_false(filtered_sensor_source_read(&filtered, 0)) != 0) {
        return 3;
    }

    return 0;
}

int main(void)
{
    if (test_rejects_invalid_setup() != 0) {
        return 1;
    }
    if (test_first_sample_passes_through_sequence_source() != 0) {
        return 2;
    }
    if (test_second_sample_is_filtered() != 0) {
        return 3;
    }
    if (test_reset_makes_next_sample_pass_through() != 0) {
        return 4;
    }
    if (test_inner_read_failure_is_preserved() != 0) {
        return 5;
    }

    return 0;
}
