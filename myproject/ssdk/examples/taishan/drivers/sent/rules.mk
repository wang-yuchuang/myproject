LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_SENT_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_sent_test.c
endif

include $(MKROOT)/module.mk
