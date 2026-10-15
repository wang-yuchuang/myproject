LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_PM), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/taishan_pm.c \
	$(LOCAL_DIR)/taishan_idle.c
endif

ifeq ($(CONFIG_SDRV_SMC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/lp_utils.c
endif

include $(MKROOT)/module.mk
