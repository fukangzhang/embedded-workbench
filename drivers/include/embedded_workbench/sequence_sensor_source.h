#ifndef EMBEDDED_WORKBENCH_SEQUENCE_SENSOR_SOURCE_H
#define EMBEDDED_WORKBENCH_SEQUENCE_SENSOR_SOURCE_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/sensor_sample.h"
#include "embedded_workbench/sensor_source.h"

typedef struct {
    /* samples 由调用者持有，sequence source 只按索引读取，不复制整段数组。 */
    const sensor_sample_t *samples;
    size_t count;
    size_t index;
    /* true 表示读到末尾后继续返回最后一帧，适合模拟“环境保持在最后状态”。 */
    bool repeat_last;
} sequence_sensor_source_t;

bool sequence_sensor_source_init(
    sequence_sensor_source_t *sequence,
    const sensor_sample_t *samples,
    size_t count,
    bool repeat_last);

sensor_source_t sequence_sensor_source_as_source(sequence_sensor_source_t *sequence);
bool sequence_sensor_source_read(void *context, sensor_sample_t *sample_out);

#endif
