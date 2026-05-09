#ifndef EMBEDDED_WORKBENCH_COMMAND_PARSER_H
#define EMBEDDED_WORKBENCH_COMMAND_PARSER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    /* 解析失败或尚未填充时使用 INVALID，避免后续误把脏数据当命令执行。 */
    COMMAND_TYPE_INVALID = 0,
    /* STATUS?：请求当前告警状态和最新传感器样本。 */
    COMMAND_TYPE_GET_STATUS = 1,
    /* CONFIG?：请求当前阈值配置快照。 */
    COMMAND_TYPE_GET_CONFIG = 2,
    /* SET <threshold> <value>：修改一个告警阈值。 */
    COMMAND_TYPE_SET_THRESHOLD = 3,
    /* CLEAR_ALARM：预留给后续“人工清除/确认告警”的命令。 */
    COMMAND_TYPE_CLEAR_ALARM = 4,
    /* SAMPLE <temp> <hum> <light> <smoke>：在无真实传感器时注入一帧样本。 */
    COMMAND_TYPE_SET_SAMPLE = 5
} command_type_t;

typedef enum {
    /* NONE 表示当前命令没有携带阈值名，或阈值名解析失败。 */
    COMMAND_THRESHOLD_NONE = 0,
    COMMAND_THRESHOLD_TEMP_WARNING_HIGH = 1,
    COMMAND_THRESHOLD_TEMP_ALARM_HIGH = 2,
    COMMAND_THRESHOLD_HUMIDITY_WARNING_HIGH = 3,
    COMMAND_THRESHOLD_HUMIDITY_ALARM_HIGH = 4,
    COMMAND_THRESHOLD_LIGHT_WARNING_LOW = 5,
    COMMAND_THRESHOLD_LIGHT_ALARM_LOW = 6,
    COMMAND_THRESHOLD_SMOKE_WARNING = 7,
    COMMAND_THRESHOLD_SMOKE_ALARM = 8
} command_threshold_t;

typedef struct {
    /* type 决定 command_handler 后续走哪条业务分支。 */
    command_type_t type;
    /* 只有 SET 命令会使用 threshold。 */
    command_threshold_t threshold;
    /* SET 命令的数值参数，先用 int32_t 接住，再由 handler 检查具体字段范围。 */
    int32_t value;
    /* SAMPLE 命令的 4 个字段也先用 int32_t 接住。
     * handler 再判断它们能否安全落到 sensor_sample_t 的具体类型和物理范围。 */
    int32_t sample_temperature_c_x10;
    int32_t sample_humidity_rh_x10;
    int32_t sample_light_lux;
    int32_t sample_smoke_ppm;
} command_t;

/* 初始化命令对象。
 * C 结构体不会自动带安全业务默认值，解析前先调用它能避免读到旧内容。 */
void command_init(command_t *command);

/* 解析面向串口/控制台的简单文本命令。
 * 输入示例：
 * - STATUS?
 * - CONFIG?
 * - CLEAR_ALARM
 * - SET TEMP_WARN 360
 * - SAMPLE 360 600 250 20
 *
 * 解析器只负责“文本 -> command_t”，不会修改配置，也不会生成响应。
 * 解析失败时保持 INVALID 语义，避免半解析命令被后续逻辑误用。 */
bool command_parse(const char *line, command_t *command);

/* 下面两个 name 函数只用于日志、测试和响应文本，不参与业务判断。 */
const char *command_type_name(command_type_t type);
const char *command_threshold_name(command_threshold_t threshold);

#endif
