LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)


MODULE_SRCS += $(LOCAL_DIR)/app_usbh.c

ifeq ($(CONFIG_USB_HOST_HID), y)
MODULE_SRCS += $(wildcard $(LOCAL_DIR)/HID/*.c)
endif

ifeq ($(CONFIG_USB_HOST_MSC), y)
ifeq ($(CONFIG_FATFS)$(CONFIG_DISK), yy)
MODULE_SRCS += $(wildcard $(LOCAL_DIR)/MSC/FatFs/*.c)
else
MODULE_SRCS += $(wildcard $(LOCAL_DIR)/MSC/noFS/*.c)
endif
endif


include $(MKROOT)/module.mk
