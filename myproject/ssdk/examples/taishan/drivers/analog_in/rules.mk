LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_ANALOG_IN_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/analog_in_demo.c
endif

include $(MKROOT)/module.mk