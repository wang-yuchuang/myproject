LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/app_usbd.c \
	$(LOCAL_DIR)/app_usbd_hid.c

include $(MKROOT)/module.mk

