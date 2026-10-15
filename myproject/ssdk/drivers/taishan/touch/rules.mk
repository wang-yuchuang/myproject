LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/touch_core.c \
	$(LOCAL_DIR)/touch_event.c

ifeq ($(CONFIG_TOUCH_GOODIX), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/goodix.c
endif

include $(MKROOT)/module.mk
