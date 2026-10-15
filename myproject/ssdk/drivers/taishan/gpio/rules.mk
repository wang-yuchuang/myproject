LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_GPIO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/taishan_gpio.c
endif

include $(MKROOT)/module.mk
