LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_LED_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/led_demo.c
endif

include $(MKROOT)/module.mk