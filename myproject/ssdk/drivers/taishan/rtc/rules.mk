LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SEC_RTC),y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sec_rtc.c \
	$(LOCAL_DIR)/sec_rtcd.c
endif

include $(MKROOT)/module.mk