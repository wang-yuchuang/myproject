LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_DW_RTC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dw_rtc_drv.c \
	$(LOCAL_DIR)/dw_rtc.c
endif


include $(MKROOT)/module.mk