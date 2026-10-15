#ifndef __ILOG4C_H__
#define __ILOG4C_H__

#include "base_defines/basetypes.h"


void log_message(int priority ,
        const char *file, int line, const char *fun,
        const char *fmt , ...);
void log_bmsCan_message(int priority, const char *fmt, ...);
void log_bmsAlmCan_message(int priority, const char *fmt, ...);
void log_filedebug(int priority, const char *fmt, ...);
int test_log_fini(void);
void log_bmshisalm_message(int priority, const char *fmt, ...);

#define LOG_FATAL(fmt,  args...) do{log_message(000, basename(__FILE__), __LINE__ , __FUNCTION__ , fmt, ##args);} while(0)
#define LOG_ERROR(fmt,  args...) do{log_message(300, basename(__FILE__), __LINE__ , __FUNCTION__ , fmt, ##args);} while(0)
#define LOG_WARN(fmt,   args...) do{log_message(400, basename(__FILE__), __LINE__ , __FUNCTION__ , fmt, ##args);} while(0)
#define LOG_NOTICE(fmt, args...) do{log_message(500, basename(__FILE__), __LINE__, __FUNCTION__ , fmt, ##args);}  while(0)
#define LOG_INFO(fmt,   args...) do{log_message(600, basename(__FILE__), __LINE__ , __FUNCTION__ , fmt, ##args);} while(0)
#define LOG_DEBUG(fmt , args...) do{log_message(700, basename(__FILE__), __LINE__ , __FUNCTION__ , fmt, ##args);} while(0)
#define LOG_TRACE(fmt,  args...) do{log_message(800, basename(__FILE__), __LINE__ , __FUNCTION__ , fmt, ##args);} while(0)
 
#define LOG_BMS_CAN_INFO(fmt,     args...) do{log_bmsCan_message(600, fmt, ##args);}     while(0)
#define LOG_BMS_ALM_CAN_INFO(fmt, args...) do{log_bmsAlmCan_message(600, fmt, ##args);}  while(0)
#define LOG_METER_FAULT_INFO(fmt, args...) do{log_meterFault_message(600, fmt, ##args);} while(0)
#define LOG_BMS_HIS_ALM_INFO(fmt,  args...) do{log_bmshisalm_message(600, fmt, ##args);}     while(0)

#define INTV_S_LOG_DEBUGFILE  (10*60)
// // unit test
// #define INTV_S_LOG_DEBUGFILE  (1)
#define LOG_FILEDEBUG
#define LOGFD(fmt, args...) do{log_filedebug(600, fmt, ##args);} while(0)

int test_log_init(void);
int Log4cOut(void);

#endif
