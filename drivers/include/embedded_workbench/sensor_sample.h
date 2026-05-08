#ifndef EMBEDDED_WORKBENCH_SENSOR_SAMPLE_H
#define EMBEDDED_WORKBENCH_SENSOR_SAMPLE_H

#include <stdbool.h>
#include <stdint.h>

#define SENSOR_TEMPERATURE_MIN_C_X10 (-400)
#define SENSOR_TEMPERATURE_MAX_C_X10 (850)
#define SENSOR_HUMIDITY_MIN_RH_X10 (0)
#define SENSOR_HUMIDITY_MAX_RH_X10 (1000)
#define SENSOR_LIGHT_MAX_LUX (100000u)
#define SENSOR_SMOKE_MAX_PPM (10000u)

/* 传感器采样统一使用整数，适合 MCU 上避免浮点依赖。
 * temperature_c_x10 表示摄氏度乘以 10，例如 253 表示 25.3 C。 */
typedef struct {
    int16_t temperature_c_x10;
    uint16_t humidity_rh_x10;
    uint32_t light_lux;
    uint16_t smoke_ppm;
} sensor_sample_t;

sensor_sample_t sensor_sample_make(
    int16_t temperature_c_x10,
    uint16_t humidity_rh_x10,
    uint32_t light_lux,
    uint16_t smoke_ppm);

bool sensor_sample_is_valid(const sensor_sample_t *sample);

#endif
