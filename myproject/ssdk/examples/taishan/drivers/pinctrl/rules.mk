LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/taishan_gpio_demo.c \
	$(LOCAL_DIR)/taishan_pinctrl_demo.c

include $(MKROOT)/module.mk
