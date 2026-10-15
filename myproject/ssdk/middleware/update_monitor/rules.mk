LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_UPDATE_MONITOR), y)
GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/update_monitor.c  \
	$(LOCAL_DIR)/update_monitor_diskio.c  \

endif
include $(MKROOT)/module.mk
