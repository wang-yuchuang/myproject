LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_LSD_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/lsd_demo.c
endif

include $(MKROOT)/module.mk