LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc

MODULE_SRCS += \
	$(LOCAL_DIR)/src/boot_splash.c

include $(MKROOT)/module.mk