LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_MBOX_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_taishan_mbox_test.c
endif

ifeq ($(CONFIG_BAREMETAL_MBOX_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_mbox_test_baremetal.c
endif

include $(MKROOT)/module.mk
