LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

MODULE_SRCS += \
	$(LOCAL_DIR)/sem_monitor.c

include $(MKROOT)/module.mk
