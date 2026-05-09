#ifndef EMBEDDED_WORKBENCH_SENSOR_SAMPLE_FILTER_H
#define EMBEDDED_WORKBENCH_SENSOR_SAMPLE_FILTER_H

#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/sensor_sample.h"

typedef struct {
    /* alpha_numerator / alpha_denominator 表示新样本权重。
     * 例如 1/4 表示输出更相信历史值，3/4 表示输出更快跟随新值。 */
    uint16_t alpha_numerator;
    uint16_t alpha_denominator;
    bool has_last_sample;
    sensor_sample_t last_sample;
} sensor_sample_filter_t;

bool sensor_sample_filter_init(
    sensor_sample_filter_t *filter,
    uint16_t alpha_numerator,
    uint16_t alpha_denominator);

void sensor_sample_filter_reset(sensor_sample_filter_t *filter);

bool sensor_sample_filter_apply(
    sensor_sample_filter_t *filter,
    const sensor_sample_t *input,
    sensor_sample_t *output);

#endif
