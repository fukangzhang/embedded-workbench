#ifndef EMBEDDED_WORKBENCH_ALARM_STATE_H
#define EMBEDDED_WORKBENCH_ALARM_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/sensor_sample.h"

typedef enum {
    ALARM_STATE_NORMAL = 0,
    ALARM_STATE_WARNING = 1,
    ALARM_STATE_ALARM = 2,
    ALARM_STATE_SENSOR_FAULT = 3
} alarm_state_t;

/* 告警阈值使用传感器原始单位：温湿度为 x10，光照为 lux，烟雾为 ppm。
 * recovery 阈值用于回滞，避免数值贴着边界抖动时状态频繁切换。 */
typedef struct {
    int16_t temperature_warning_high_c_x10;
    int16_t temperature_alarm_high_c_x10;
    int16_t temperature_recovery_high_c_x10;
    uint16_t humidity_warning_high_rh_x10;
    uint16_t humidity_alarm_high_rh_x10;
    uint16_t humidity_recovery_high_rh_x10;
    uint32_t light_warning_low_lux;
    uint32_t light_alarm_low_lux;
    uint32_t light_recovery_low_lux;
    uint16_t smoke_warning_ppm;
    uint16_t smoke_alarm_ppm;
    uint16_t smoke_recovery_ppm;
} alarm_config_t;

alarm_config_t alarm_config_default(void);
bool alarm_config_is_valid(const alarm_config_t *config);

/* 根据上一状态和新采样计算下一状态。
 * 输入无效时返回 SENSOR_FAULT；ALARM/WARNING 状态只有达到 recovery 条件才会退出。 */
alarm_state_t alarm_state_update(
    alarm_state_t current_state,
    const alarm_config_t *config,
    const sensor_sample_t *sample);
const char *alarm_state_name(alarm_state_t state);

#endif
