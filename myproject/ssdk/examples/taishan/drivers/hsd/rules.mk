LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_HSD_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/hsd_demo.c
endif

include $(MKROOT)/module.mk