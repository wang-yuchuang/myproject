LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SUPPORT_LOG_TIMESTAMP), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/timestamp.c
endif

include $(MKROOT)/module.mk
