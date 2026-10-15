LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAIHANG_XSPI), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/taihang_xspi.c

endif

include $(MKROOT)/module.mk
