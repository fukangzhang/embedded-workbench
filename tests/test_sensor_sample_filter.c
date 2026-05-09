#include <stdbool.h>

#include "embedded_workbench/sensor_sample_filter.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
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

static int test_first_sample_passes_through(void)
{
    sensor_sample_filter_t filter;
    sensor_sample_t input = sensor_sample_make(250, 500u, 300u, 20u);
    sensor_sample_t output = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(sensor_sample_filter_init(&filter, 1u, 2u)) != 0 ||
        expect_true(sensor_sample_filter_apply(&filter, &input, &output)) != 0 ||
        expect_sample(&output, &input) != 0) {
        return 1;
    }

    return 0;
}

static int test_half_weight_smooths_toward_next_sample(void)
{
    sensor_sample_filter_t filter;
    sensor_sample_t first = sensor_sample_make(200, 400u, 100u, 10u);
    sensor_sample_t next = sensor_sample_make(400, 800u, 500u, 50u);
    sensor_sample_t expected = sensor_sample_make(300, 600u, 300u, 30u);
    sensor_sample_t output = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(sensor_sample_filter_init(&filter, 1u, 2u)) != 0 ||
        expect_true(sensor_sample_filter_apply(&filter, &first, &output)) != 0 ||
        expect_true(sensor_sample_filter_apply(&filter, &next, &output)) != 0 ||
        expect_sample(&output, &expected) != 0) {
        return 1;
    }

    return 0;
}

static int test_quarter_weight_uses_previous_filtered_sample(void)
{
    sensor_sample_filter_t filter;
    sensor_sample_t first = sensor_sample_make(200, 400u, 100u, 10u);
    sensor_sample_t second = sensor_sample_make(400, 800u, 500u, 50u);
    sensor_sample_t third = sensor_sample_make(400, 800u, 500u, 50u);
    sensor_sample_t expected_second = sensor_sample_make(250, 500u, 200u, 20u);
    sensor_sample_t expected_third = sensor_sample_make(287, 575u, 275u, 27u);
    sensor_sample_t output = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(sensor_sample_filter_init(&filter, 1u, 4u)) != 0 ||
        expect_true(sensor_sample_filter_apply(&filter, &first, &output)) != 0 ||
        expect_true(sensor_sample_filter_apply(&filter, &second, &output)) != 0 ||
        expect_sample(&output, &expected_second) != 0 ||
        expect_true(sensor_sample_filter_apply(&filter, &third, &output)) != 0 ||
        expect_sample(&output, &expected_third) != 0) {
        return 1;
    }

    return 0;
}

static int test_reset_makes_next_sample_pass_through(void)
{
    sensor_sample_filter_t filter;
    sensor_sample_t first = sensor_sample_make(200, 400u, 100u, 10u);
    sensor_sample_t next = sensor_sample_make(400, 800u, 500u, 50u);
    sensor_sample_t output = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_true(sensor_sample_filter_init(&filter, 1u, 2u)) != 0 ||
        expect_true(sensor_sample_filter_apply(&filter, &first, &output)) != 0) {
        return 1;
    }

    sensor_sample_filter_reset(&filter);

    if (expect_true(sensor_sample_filter_apply(&filter, &next, &output)) != 0 ||
        expect_sample(&output, &next) != 0) {
        return 2;
    }

    return 0;
}

static int test_invalid_arguments_fail(void)
{
    sensor_sample_filter_t filter;
    sensor_sample_t input = sensor_sample_make(250, 500u, 300u, 20u);
    sensor_sample_t invalid = sensor_sample_make(SENSOR_TEMPERATURE_MAX_C_X10 + 1, 500u, 300u, 20u);
    sensor_sample_t output = sensor_sample_make(0, 0u, 0u, 0u);

    if (expect_false(sensor_sample_filter_init(0, 1u, 2u)) != 0 ||
        expect_false(sensor_sample_filter_init(&filter, 0u, 2u)) != 0 ||
        expect_false(sensor_sample_filter_init(&filter, 3u, 2u)) != 0 ||
        expect_false(sensor_sample_filter_init(&filter, 1u, 0u)) != 0) {
        return 1;
    }

    if (expect_true(sensor_sample_filter_init(&filter, 1u, 2u)) != 0 ||
        expect_false(sensor_sample_filter_apply(0, &input, &output)) != 0 ||
        expect_false(sensor_sample_filter_apply(&filter, 0, &output)) != 0 ||
        expect_false(sensor_sample_filter_apply(&filter, &input, 0)) != 0 ||
        expect_false(sensor_sample_filter_apply(&filter, &invalid, &output)) != 0) {
        return 2;
    }

    sensor_sample_filter_reset(0);

    return 0;
}

int main(void)
{
    if (test_first_sample_passes_through() != 0) {
        return 1;
    }
    if (test_half_weight_smooths_toward_next_sample() != 0) {
        return 2;
    }
    if (test_quarter_weight_uses_previous_filtered_sample() != 0) {
        return 3;
    }
    if (test_reset_makes_next_sample_pass_through() != 0) {
        return 4;
    }
    if (test_invalid_arguments_fail() != 0) {
        return 5;
    }

    return 0;
}
