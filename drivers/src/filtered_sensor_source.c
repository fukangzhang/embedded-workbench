#include "embedded_workbench/filtered_sensor_source.h"

bool filtered_sensor_source_init(
    filtered_sensor_source_t *filtered,
    const sensor_source_t *inner_source,
    uint16_t alpha_numerator,
    uint16_t alpha_denominator)
{
    if (filtered == 0 || !sensor_source_is_valid(inner_source)) {
        return false;
    }

    if (!sensor_sample_filter_init(&filtered->filter, alpha_numerator, alpha_denominator)) {
        return false;
    }

    /* sensor_source_t 只有 read 函数指针和 context 指针，复制它等价于复制“怎么读”
     * 这个入口。真正的传感器状态仍在 context 指向的位置，不在这里复制。 */
    filtered->inner_source = *inner_source;
    return true;
}

sensor_source_t filtered_sensor_source_as_source(filtered_sensor_source_t *filtered)
{
    sensor_source_t source;

    source.read = filtered_sensor_source_read;
    source.context = filtered;
    return source;
}

void filtered_sensor_source_reset(filtered_sensor_source_t *filtered)
{
    if (filtered != 0) {
        sensor_sample_filter_reset(&filtered->filter);
    }
}

bool filtered_sensor_source_read(void *context, sensor_sample_t *sample_out)
{
    filtered_sensor_source_t *filtered = (filtered_sensor_source_t *)context;
    sensor_sample_t raw_sample;

    if (filtered == 0 || sample_out == 0) {
        return false;
    }

    /* 先通过统一 source 入口读取，这样非法 sample、空 read 函数和底层失败
     * 都会被 sensor_source_read 按同一套规则处理。 */
    if (!sensor_source_read(&filtered->inner_source, &raw_sample)) {
        return false;
    }

    return sensor_sample_filter_apply(&filtered->filter, &raw_sample, sample_out);
}
