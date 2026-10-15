#ifndef __THERMAL_SERVICE_H__
#define __THERMAL_SERVICE_H__

#define THERMAL_SERVICE_NAME        "thermal-service"
#define THERMAL_SERVICE_STACK_SIZE       (1536)
#define SYS_SHUTDOWN 1
#define SYS_REBOOT 0

void thermal_service_init(int type);
#endif /* __THERMAL_SERVICE_H__ */
