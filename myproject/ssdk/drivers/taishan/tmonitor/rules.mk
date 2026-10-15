LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAMPER_MONITOR), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/tamper_monitor.c
endif

include $(MKROOT)/module.mk
