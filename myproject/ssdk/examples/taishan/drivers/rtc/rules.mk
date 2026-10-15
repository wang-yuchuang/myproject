LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_RTC_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/rtc_test.c
endif

include $(MKROOT)/module.mk
