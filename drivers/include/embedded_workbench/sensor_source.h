#ifndef EMBEDDED_WORKBENCH_SENSOR_SOURCE_H
#define EMBEDDED_WORKBENCH_SENSOR_SOURCE_H

#include <stdbool.h>

#include "embedded_workbench/sensor_sample.h"

typedef bool (*sensor_source_read_fn)(void *context, sensor_sample_t *sample_out);

typedef struct {
    /* read 从某个来源读取一帧 sample。
     * context 由具体实现解释：可以是测试 fake、模拟序列，也可以是真实 I2C/ADC 驱动状态。 */
    sensor_source_read_fn read;
    void *context;
} sensor_source_t;

bool sensor_source_is_valid(const sensor_source_t *source);

/* 从 source 读取一帧 sample，并统一检查输出是否落在项目支持的物理范围内。
 * 返回 false 表示 source 不可用、读取失败或读取出的 sample 非法。 */
bool sensor_source_read(const sensor_source_t *source, sensor_sample_t *sample_out);

#endif
