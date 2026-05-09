#ifndef EMBEDDED_WORKBENCH_FILTERED_SENSOR_SOURCE_H
#define EMBEDDED_WORKBENCH_FILTERED_SENSOR_SOURCE_H

#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/sensor_sample_filter.h"
#include "embedded_workbench/sensor_source.h"

typedef struct {
    /* inner_source 是被包装的原始来源，例如 sequence source、未来的 I2C 温湿度传感器
     * 或 ADC 采样来源。这里保存的是 sensor_source_t 这层小接口副本，不复制真正的
     * 硬件/序列上下文；inner_source.context 指向的数据仍由调用者负责保证生命周期。 */
    sensor_source_t inner_source;
    sensor_sample_filter_t filter;
} filtered_sensor_source_t;

bool filtered_sensor_source_init(
    filtered_sensor_source_t *filtered,
    const sensor_source_t *inner_source,
    uint16_t alpha_numerator,
    uint16_t alpha_denominator);

sensor_source_t filtered_sensor_source_as_source(filtered_sensor_source_t *filtered);
void filtered_sensor_source_reset(filtered_sensor_source_t *filtered);
bool filtered_sensor_source_read(void *context, sensor_sample_t *sample_out);

#endif
