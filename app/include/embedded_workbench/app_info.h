#ifndef EMBEDDED_WORKBENCH_APP_INFO_H
#define EMBEDDED_WORKBENCH_APP_INFO_H

typedef enum {
    APP_RUN_MODE_HOST_SIM = 0,
    APP_RUN_MODE_TARGET_BOARD = 1
} app_run_mode_t;

const char *app_info_project_name(void);
const char *app_info_version(void);
const char *app_info_run_mode_name(app_run_mode_t mode);

#endif
