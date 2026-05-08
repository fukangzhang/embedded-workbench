#include "embedded_workbench/app_info.h"

const char *app_info_project_name(void)
{
    return "Embedded Workbench";
}

const char *app_info_version(void)
{
    return "0.1.0";
}

const char *app_info_run_mode_name(app_run_mode_t mode)
{
    switch (mode) {
    case APP_RUN_MODE_HOST_SIM:
        return "host-simulation";
    case APP_RUN_MODE_TARGET_BOARD:
        return "target-board";
    default:
        return "unknown";
    }
}
