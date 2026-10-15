LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_PINCTRL), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_pinctrl.c
endif

include $(MKROOT)/module.mk
