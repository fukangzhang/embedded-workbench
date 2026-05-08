#include <string.h>

#include "embedded_workbench/app_info.h"

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0;
}

int main(void)
{
    if (!expect_string(app_info_project_name(), "Embedded Workbench")) {
        return 1;
    }

    if (!expect_string(app_info_version(), "0.1.0")) {
        return 2;
    }

    if (!expect_string(app_info_run_mode_name(APP_RUN_MODE_HOST_SIM), "host-simulation")) {
        return 3;
    }

    if (!expect_string(app_info_run_mode_name(APP_RUN_MODE_TARGET_BOARD), "target-board")) {
        return 4;
    }

    if (!expect_string(app_info_run_mode_name((app_run_mode_t)99), "unknown")) {
        return 5;
    }

    return 0;
}
