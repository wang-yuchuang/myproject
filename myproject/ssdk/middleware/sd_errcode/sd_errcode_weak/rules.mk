LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/../include

MODULE_SRCS += \
	$(LOCAL_DIR)/sd_errcode_weak.c

include $(MKROOT)/module.mk
