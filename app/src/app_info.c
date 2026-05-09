#include "embedded_workbench/app_info.h"

const char *app_info_project_name(void)
{
    /* 返回字符串字面量：它存放在只读区域，调用者不需要也不能释放。 */
    return "Embedded Workbench";
}

const char *app_info_version(void)
{
    /* 先固定一个简单版本号，后续可以再接入构建号或 Git commit。 */
    return "0.1.0";
}

const char *app_info_run_mode_name(app_run_mode_t mode)
{
    /* switch 让枚举值到文本的映射集中、清晰，也方便测试固定输出。 */
    switch (mode) {
    case APP_RUN_MODE_HOST_SIM:
        return "host-simulation";
    case APP_RUN_MODE_TARGET_BOARD:
        return "target-board";
    default:
        /* 防御性回退：如果上层传入未定义枚举值，也返回稳定字符串。 */
        return "unknown";
    }
}
