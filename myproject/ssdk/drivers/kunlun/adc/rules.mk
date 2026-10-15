LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_ADC), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/kunlun_adc.c
endif

include $(MKROOT)/module.mk
