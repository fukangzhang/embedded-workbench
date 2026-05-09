#include "embedded_workbench/sensor_sample_filter.h"

static int32_t filter_i32(int32_t previous, int32_t next, uint16_t numerator, uint16_t denominator)
{
    int32_t delta = next - previous;

    return previous + ((delta * (int32_t)numerator) / (int32_t)denominator);
}

static uint32_t filter_u32(uint32_t previous, uint32_t next, uint16_t numerator, uint16_t denominator)
{
    int64_t delta = (int64_t)next - (int64_t)previous;

    return (uint32_t)((int64_t)previous + ((delta * (int64_t)numerator) / (int64_t)denominator));
}

bool sensor_sample_filter_init(
    sensor_sample_filter_t *filter,
    uint16_t alpha_numerator,
    uint16_t alpha_denominator)
{
    if (filter == 0 ||
        alpha_denominator == 0u ||
        alpha_numerator == 0u ||
        alpha_numerator > alpha_denominator) {
        return false;
    }

    filter->alpha_numerator = alpha_numerator;
    filter->alpha_denominator = alpha_denominator;
    filter->has_last_sample = false;
    filter->last_sample = sensor_sample_make(0, 0u, 0u, 0u);

    return true;
}

void sensor_sample_filter_reset(sensor_sample_filter_t *filter)
{
    if (filter != 0) {
        filter->has_last_sample = false;
    }
}

bool sensor_sample_filter_apply(
    sensor_sample_filter_t *filter,
    const sensor_sample_t *input,
    sensor_sample_t *output)
{
    sensor_sample_t filtered;

    if (filter == 0 ||
        output == 0 ||
        !sensor_sample_is_valid(input)) {
        return false;
    }

    if (!filter->has_last_sample) {
        filter->last_sample = *input;
        filter->has_last_sample = true;
        *output = *input;
        return true;
    }

    filtered.temperature_c_x10 = (int16_t)filter_i32(
        filter->last_sample.temperature_c_x10,
        input->temperature_c_x10,
        filter->alpha_numerator,
        filter->alpha_denominator);
    filtered.humidity_rh_x10 = (uint16_t)filter_u32(
        filter->last_sample.humidity_rh_x10,
        input->humidity_rh_x10,
        filter->alpha_numerator,
        filter->alpha_denominator);
    filtered.light_lux = filter_u32(
        filter->last_sample.light_lux,
        input->light_lux,
        filter->alpha_numerator,
        filter->alpha_denominator);
    filtered.smoke_ppm = (uint16_t)filter_u32(
        filter->last_sample.smoke_ppm,
        input->smoke_ppm,
        filter->alpha_numerator,
        filter->alpha_denominator);

    if (!sensor_sample_is_valid(&filtered)) {
        return false;
    }

    filter->last_sample = filtered;
    *output = filtered;
    return true;
}
