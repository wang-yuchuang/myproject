LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_HWMUTEX), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/taishan_hwmutex.c
endif

include $(MKROOT)/module.mk
