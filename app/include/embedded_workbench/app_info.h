#ifndef EMBEDDED_WORKBENCH_APP_INFO_H
#define EMBEDDED_WORKBENCH_APP_INFO_H

typedef enum {
    /* 主机仿真模式：跑在 PC 上，用于本项目的测试和命令行 demo。 */
    APP_RUN_MODE_HOST_SIM = 0,
    /* 目标板模式：跑在真实 MCU 固件里。 */
    APP_RUN_MODE_TARGET_BOARD = 1
} app_run_mode_t;

/* 这些字符串会出现在日志、命令行或固件自检输出里，集中放这里避免各处硬编码。 */
const char *app_info_project_name(void);
const char *app_info_version(void);
const char *app_info_run_mode_name(app_run_mode_t mode);

#endif
