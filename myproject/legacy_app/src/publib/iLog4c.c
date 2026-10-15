#include "std_sys.h"
#include "base_defines/iLog.h"
#include "app_incs.h"
#include "base_defines/iLog4c.h"
#include "../include/3rd_party/log4c/include/log4c.h"

/*
日志级别排序：fatal error warn notice info debug

*/
 
static log4c_category_t *log_category = NULL;
static log4c_category_t *log_bms_category = NULL;
static log4c_category_t *log_bmsalm_category = NULL;
static log4c_category_t *log_meterfault_category = NULL;
static log4c_category_t *log_filedebug_category = NULL;
static log4c_category_t *log_bmshisalm_category = NULL;
/*******************************************************************************/
static const char* dated_format(const log4c_layout_t *a_layout,
        const log4c_logging_event_t *a_event)
{
    static char buffer[1024];
    struct tm tm;

    gmtime_r(&a_event->evt_timestamp.tv_sec, &tm);
    snprintf(buffer, sizeof(buffer), "%04d%02d%02d %02d:%02d:%02d.%06ld %-5s %s\n",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             (tm.tm_hour + 8) % 24, tm.tm_min, tm.tm_sec,
             a_event->evt_timestamp.tv_usec,
             log4c_priority_to_string(a_event->evt_priority),
             a_event->evt_msg);

    return buffer;
}
 
/*******************************************************************************/
const log4c_layout_type_t log4c_layout_type_spice = {
    "dated_format",
    dated_format,
};
 
static int init_dated_format(void)
{
    log4c_layout_type_set(&log4c_layout_type_spice);//自定义格式类型函数指针
    return 0;
}
 
// ===  FUNCTION  ======================================================================
//  Name:  log_message
//  Description:  记录日志信息
//  @param priority [in]: 日志类别
//  @param file [in]: 文件
//  @param line [in]: 行
//  @param fun [in]: 函数
//  @param fmt [in]: 格式化参数
// =====================================================================================
#define MAX_TOTAL_LOG_LEN   2048    // 最大一次性格式化长度
#define MAX_SINGLE_LOG_LEN  950     // 单段输出最大长度（含前缀）

void log_message(int priority, const char *file, int line, const char *fun, const char *fmt, ...)
{
    if (!log_category) return;

     // 1. 构建前缀 "[file:line][tid] "
     char prefix[128];
     const char *bfile = basename((char*)file);
     int prefix_len = snprintf(prefix, sizeof(prefix),
                               "[%s:%d]",
                               bfile, line);
     if (prefix_len < 0 || prefix_len >= (int)sizeof(prefix)) 
     {
         prefix_len = 0;
         prefix[0] = '\0';
     }

    // 2. 格式化正文到栈上缓冲
    char new_fmt[MAX_TOTAL_LOG_LEN];
    int avail = MAX_TOTAL_LOG_LEN - prefix_len;
    memcpy(new_fmt, prefix, prefix_len);

    va_list ap;
    va_start(ap, fmt);
    int body_len = vsnprintf(new_fmt + prefix_len, avail, fmt, ap);
    va_end(ap);
    if (body_len < 0) return;
    if (body_len >= avail) body_len = avail - 1;

    int total_len = prefix_len + body_len;
    new_fmt[total_len] = '\0';

    // 3. 分段输出，chunk 缓冲放外
    char chunk[MAX_SINGLE_LOG_LEN + 1];
    int offset = 0;
    while (offset < total_len) 
    {
        int remain = total_len - offset;
        int copy_len = remain;
        if (copy_len + 1 > MAX_SINGLE_LOG_LEN) 
        {
            copy_len = MAX_SINGLE_LOG_LEN - 1;
        }
        memcpy(chunk, new_fmt + offset, copy_len);
        chunk[copy_len] = '\0';
        log4c_category_log(log_category, priority, "%s", chunk);
        offset += copy_len;
    }
}

// void log_message(int priority, const char *file, int line, const char *fun, const char *fmt, ...)
// {
//     const int len_buf = 2048;
//     char new_fmt[len_buf];
//     int n;
//     if (log_category == NULL)
//         return;
 
//     n = snprintf(new_fmt, len_buf, "[%s %d %s] ", file, line, fun);
//     if (n < 0 || n >= len_buf)
//         return;
 
//     va_list ap;
//     va_start(ap, fmt);
//     //vsnprintf(new_fmt + n, len_buf - n, fmt, ap);  // 将日志信息追加到 new_fmt 后面
//     vsnprintf(new_fmt, len_buf, fmt, ap);   //没有文件名、行号和函数名
//     va_end(ap);
//     log4c_category_log(log_category, priority, new_fmt);
// }
 
// ===  FUNCTION  ======================================================================
//  Name:  log_fini
//  Description:  清理所有申请的内存，关闭它们打开的文件
// =====================================================================================
int test_log_fini(void)
{
    return log4c_fini();
}
 
static void set_log4c_envirment(void)
{
    // 优先使用可写目录 /opt/appdata/config，兜底 /etc
    const char* log4crc_path = "/opt/appdata/config";
    if (access("/opt/appdata/config/log4crc", F_OK) != 0) {
        log4crc_path = "/etc";
    }
    setenv("LOG4C_RCPATH", log4crc_path, 1);
    printf("[BOOT] log4c: LOG4C_RCPATH=%s\n", log4crc_path);
}
 
int test_log_init(void)
{
    set_log4c_envirment();
    if (log4c_init() == 1) {
       printf("[BOOT] log4c: log4c_init() FAILED (rcpath=%s)\n", getenv("LOG4C_RCPATH"));
       return FALSE;
    }
    printf("[BOOT] log4c: log4c_init() OK\n");
 
    //log_category = log4c_category_get("log4c.mytest");
    log_category = log4c_category_get("fileLog");
    log_bms_category = log4c_category_get("bmsCanLog");
    log_bmsalm_category = log4c_category_get("bmsAlmCanLog");
    log_meterfault_category = log4c_category_get("meterFaultLog");
    log_filedebug_category = log4c_category_get("logfd");
    log_bmshisalm_category = log4c_category_get("bmsAlmHisLog");
    

    return TRUE;
}


int Log4cOut()
{
	int i = 10;
	float n = 11.11;
	//printf("log4ctest...\n");

    /* 输出日志 */
    LOG_FATAL("LOG_FATAL i=[%d] n=[%0.2f]",i,n);
    LOG_ERROR("LOG_ERROR i=[%d] n=[%0.2f]",i,n);
    LOG_WARN("LOG_WARN i=[%d] n=[%0.2f]",i,n);
    LOG_NOTICE("LOG_NOTICE i=[%d] n=[%0.2f]",i,n);
    LOG_INFO("LOG_INFO i=[%d] n=[%0.2f]",i,n);
    LOG_DEBUG("LOG_DEBUG i=[%d] n=[%0.2f]",i,n);
    LOG_TRACE("LOG_TRACE i=[%d] n=[%0.2f]",i,n);

    LOG_FATAL("LOG_FATAL");
    LOG_ERROR("LOG_ERROR");
    LOG_WARN("LOG_WARN");
    LOG_NOTICE("LOG_NOTICE");
    LOG_INFO("LOG_INFO");
    LOG_DEBUG("LOG_DEBUG");
    LOG_TRACE("LOG_TRACE");

	//printf("log4ctest...finish\n");

    return 0;

}

// log bms can
void log_bmsCan_message(int priority, const char *fmt, ...)
{
    const int len_buf = 2048;
    char new_fmt[len_buf];

    if (log_bms_category == NULL)
        return;
 
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(new_fmt, len_buf, fmt, ap);
    va_end(ap);
    log4c_category_log(log_bms_category, priority, new_fmt);
}

void log_bmsAlmCan_message(int priority, const char *fmt, ...)
{
    const int len_buf = 2048;
    char new_fmt[len_buf];

    if (log_bmsalm_category == NULL)
        return;
 
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(new_fmt, len_buf, fmt, ap);
    va_end(ap);
    log4c_category_log(log_bmsalm_category, priority, new_fmt);
}

void log_meterFault_message(int priority, const char *fmt, ...)
{
    const int len_buf = 2048;
    char new_fmt[len_buf];

    if (log_meterfault_category == NULL)
        return;
 
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(new_fmt, len_buf, fmt, ap);
    va_end(ap);
    log4c_category_log(log_meterfault_category, priority, new_fmt);
}

#include <sys/syscall.h>
static pid_t gettid(void)
{
	return syscall(SYS_gettid);
}

void log_filedebug(int priority, const char *fmt, ...)
{
    if (log_filedebug_category == NULL) return;

    const int len_buf = 2500;
    char new_fmt[len_buf];
    int prefix_len = snprintf(new_fmt, len_buf, "[%06x] ", (unsigned int)gettid());

    int avail = len_buf - prefix_len;
    va_list ap;
    va_start(ap, fmt);
    int body_len = vsnprintf(new_fmt + prefix_len, avail, fmt, ap);
    va_end(ap);
    if (body_len < 0) return;
    if (body_len >= avail) body_len = avail - 1;

    int total_len = prefix_len + body_len;
    new_fmt[total_len] = '\0';

    // 3. 分段输出，chunk 缓冲放外
    char chunk[MAX_SINGLE_LOG_LEN + 1];
    int offset = 0;
    while (offset < total_len) 
    {
        int remain = total_len - offset;
        int copy_len = remain;
        if (copy_len + 1 > MAX_SINGLE_LOG_LEN) 
        {
            copy_len = MAX_SINGLE_LOG_LEN - 1;
        }
        memcpy(chunk, new_fmt + offset, copy_len);
        chunk[copy_len] = '\0';
        log4c_category_log(log_filedebug_category, priority, "%s", chunk);
        offset += copy_len;
    }
}

void log_bmshisalm_message(int priority, const char *fmt, ...)
{
    const int len_buf = 2048;
    char new_fmt[len_buf];

    if (log_bmshisalm_category == NULL)
        return;
 
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(new_fmt, len_buf, fmt, ap);
    va_end(ap);
    log4c_category_log(log_bmshisalm_category, priority, new_fmt);
}
