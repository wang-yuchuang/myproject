LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/inc

MODULE_SRCS += \
	$(LOCAL_DIR)/src/sdrv_g2dlite.c \
	$(LOCAL_DIR)/src/sdrv_g2dlite_core.c

include $(MKROOT)/module.mk
