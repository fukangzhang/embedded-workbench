#include <stdbool.h>

#include "embedded_workbench/sensor_source.h"

typedef struct {
    sensor_sample_t sample;
    unsigned int read_count;
    bool fail_read;
} fake_sensor_source_t;

static bool fake_read(void *context, sensor_sample_t *sample_out)
{
    fake_sensor_source_t *fake = (fake_sensor_source_t *)context;

    if (fake == 0 || sample_out == 0 || fake->fail_read) {
        return false;
    }

    fake->read_count++;
    *sample_out = fake->sample;
    return true;
}

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

static int expect_uint(unsigned int actual, unsigned int expected)
{
    return actual == expected ? 0 : 1;
}

int main(void)
{
    fake_sensor_source_t fake = {0};
    sensor_source_t source = {fake_read, &fake};
    sensor_source_t invalid_source = {0, &fake};
    sensor_sample_t sample = sensor_sample_make(0, 0u, 0u, 0u);

    fake.sample = sensor_sample_make(253, 604u, 350u, 12u);

    if (expect_true(sensor_source_is_valid(&source)) != 0 ||
        expect_false(sensor_source_is_valid(&invalid_source)) != 0 ||
        expect_false(sensor_source_is_valid(0)) != 0) {
        return 1;
    }

    if (expect_true(sensor_source_read(&source, &sample)) != 0 ||
        expect_int(sample.temperature_c_x10, 253) != 0 ||
        expect_int(sample.humidity_rh_x10, 604) != 0 ||
        expect_int((int)sample.light_lux, 350) != 0 ||
        expect_int(sample.smoke_ppm, 12) != 0 ||
        expect_uint(fake.read_count, 1u) != 0) {
        return 2;
    }

    fake.sample = sensor_sample_make(SENSOR_TEMPERATURE_MAX_C_X10 + 1, 604u, 350u, 12u);
    if (expect_false(sensor_source_read(&source, &sample)) != 0 ||
        expect_uint(fake.read_count, 2u) != 0) {
        return 3;
    }

    fake.fail_read = true;
    if (expect_false(sensor_source_read(&source, &sample)) != 0 ||
        expect_uint(fake.read_count, 2u) != 0) {
        return 4;
    }

    if (expect_false(sensor_source_read(0, &sample)) != 0 ||
        expect_false(sensor_source_read(&source, 0)) != 0) {
        return 5;
    }

    return 0;
}
