LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_ADC), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/taishan_adc.c
endif

include $(MKROOT)/module.mk
