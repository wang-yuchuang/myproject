LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/dloader.c \
	$(LOCAL_DIR)/dloader_disk_io.c \
	$(LOCAL_DIR)/dloader_test.c

ifeq ($(CONFIG_DLOADER_WITH_USB), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dloader_usb_fastboot.c

endif

include $(MKROOT)/module.mk
