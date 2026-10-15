LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_SENT), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_sent_drv.c
endif

include $(MKROOT)/module.mk
