#include <stdio.h>

#include "embedded_workbench/app_info.h"

int main(void)
{
    printf("%s %s\n", app_info_project_name(), app_info_version());
    printf("run_mode=%s\n", app_info_run_mode_name(APP_RUN_MODE_HOST_SIM));
    return 0;
}
