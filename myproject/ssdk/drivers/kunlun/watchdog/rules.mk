LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_watchdog_drv.c \
	$(LOCAL_DIR)/sdrv_watchdog.c

include $(MKROOT)/module.mk
