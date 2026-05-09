#include "embedded_workbench/sensor_sample.h"

sensor_sample_t sensor_sample_make(
    int16_t temperature_c_x10,
    uint16_t humidity_rh_x10,
    uint32_t light_lux,
    uint16_t smoke_ppm)
{
    sensor_sample_t sample;

    /* 用工厂函数创建结构体，调用处就不用知道字段赋值顺序。 */
    sample.temperature_c_x10 = temperature_c_x10;
    sample.humidity_rh_x10 = humidity_rh_x10;
    sample.light_lux = light_lux;
    sample.smoke_ppm = smoke_ppm;

    return sample;
}

bool sensor_sample_is_valid(const sensor_sample_t *sample)
{
    if (sample == 0) {
        /* C 里 0 常用来表示空指针；空 sample 没有任何字段可检查。 */
        return false;
    }

    /* 这里验证的是项目当前支持的物理范围，不是传感器芯片的完整规格。 */
    if (sample->temperature_c_x10 < SENSOR_TEMPERATURE_MIN_C_X10 ||
        sample->temperature_c_x10 > SENSOR_TEMPERATURE_MAX_C_X10) {
        return false;
    }

    if (sample->humidity_rh_x10 < SENSOR_HUMIDITY_MIN_RH_X10 ||
        sample->humidity_rh_x10 > SENSOR_HUMIDITY_MAX_RH_X10) {
        /* uint16_t 不会小于 0，但保留最小值判断能让“范围检查”保持对称。 */
        return false;
    }

    if (sample->light_lux > SENSOR_LIGHT_MAX_LUX) {
        return false;
    }

    if (sample->smoke_ppm > SENSOR_SMOKE_MAX_PPM) {
        return false;
    }

    return true;
}
