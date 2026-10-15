LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/fastboot_app.c \
	$(LOCAL_DIR)/fastboot_common.c

include $(MKROOT)/module.mk

