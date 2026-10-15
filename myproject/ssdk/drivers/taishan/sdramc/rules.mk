LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_SDRAMC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/taishan_sdramc.c
endif

include $(MKROOT)/module.mk
