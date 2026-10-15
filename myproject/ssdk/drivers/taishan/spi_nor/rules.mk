LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_XSPI), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/taishan_xspi.c

endif

include $(MKROOT)/module.mk
