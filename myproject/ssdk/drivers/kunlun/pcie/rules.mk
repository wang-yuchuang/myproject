LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_DWC_PCIE), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dwc_pcie.c
endif

include $(MKROOT)/module.mk
