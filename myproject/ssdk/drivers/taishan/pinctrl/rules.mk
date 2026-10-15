LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_PINCTRL), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/taishan_pinctrl.c \
	$(LOCAL_DIR)/taishan_pinctrl_is.c
endif

include $(MKROOT)/module.mk
