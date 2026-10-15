LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_MBOX_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_mbox_test.c
endif

include $(MKROOT)/module.mk
