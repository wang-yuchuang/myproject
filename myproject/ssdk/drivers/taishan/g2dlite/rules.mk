LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

include $(LOCAL_DIR)/libs/rules.mk

MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_g2dlite.c

include $(MKROOT)/module.mk
