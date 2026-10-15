LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/sdrv_etimer.c \
    $(LOCAL_DIR)/etimer_tmr_drv.c

ifeq ($(CONFIG_SDRV_ETIMER_PWM), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/etimer_pwm_drv.c
endif

include $(MKROOT)/module.mk