LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_PWM), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/pwm_drv.c
endif

include $(MKROOT)/module.mk
