LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_ADC_DEMO), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/adc_test.c
endif

ifeq ($(CONFIG_TAISHAN_ADC_DEMO), y)
ifeq ($(BOARD), e3_evb)
MODULE_SRCS += \
    $(LOCAL_DIR)/taishan_adc_demo.c
else
MODULE_SRCS += \
    $(LOCAL_DIR)/taishan_adc_test.c
endif
endif

include $(MKROOT)/module.mk
