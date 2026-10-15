LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_PWM_DEMO), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/app_pwm.c
endif

include $(MKROOT)/module.mk
