#ifndef __APP_LOG_H__
#define __APP_LOG_H__
#include <debug.h>
#include <board.h>

#define APP_ERR(fmt, ...) \
        ssdk_printf(SSDK_EMERG, "[DEMO][%s:E][%s][%d]" fmt "\n", LOG_TAG, __func__, current_time(), ##__VA_ARGS__);
#define APP_WARN(fmt, ...) \
        ssdk_printf(SSDK_WARNING, "[DEMO][%s:W][%s][%d]" fmt "\n", LOG_TAG, __func__, current_time(), ##__VA_ARGS__);
#define APP_CRIT(fmt, ...) \
        ssdk_printf(SSDK_CRIT, "[DEMO][%s:C][%s][%d]" fmt "\n", LOG_TAG, __func__, current_time(), ##__VA_ARGS__);
#define APP_INFO(fmt, ...) \
        ssdk_printf(SSDK_NOTICE, "[DEMO][%s:I][%s][%d]" fmt "\n", LOG_TAG, __func__, current_time(), ##__VA_ARGS__);
#define APP_DBG(fmt, ...) \
        ssdk_printf(SSDK_DEBUG, "[DEMO][%s:D][%s][%d]" fmt "\n", LOG_TAG, __func__, current_time(), ##__VA_ARGS__);
#ifdef CONFIG_APP_LAUNCHER_DEBUG
#define APP_LOG(fmt, ...) \
        ssdk_printf(SSDK_EMERG, "[DEMO][%s:D][%s][%d]" fmt "\n", LOG_TAG, __func__, current_time(), ##__VA_ARGS__);
#else
#define APP_LOG(fmt, ...)
#endif
#endif
/* EOF */
