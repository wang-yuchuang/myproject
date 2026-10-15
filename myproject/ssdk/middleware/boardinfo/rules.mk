LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/boardinfo.c \
	$(LOCAL_DIR)/sdrv_extra_gpr.c

include $(MKROOT)/module.mk
