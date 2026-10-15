LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_MBOX), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_mbox_drv.c \
	$(LOCAL_DIR)/sdrv_mbox.c
endif

include $(MKROOT)/module.mk
