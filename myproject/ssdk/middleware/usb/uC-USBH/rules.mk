LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)


MODULE_SRCS += $(wildcard $(LOCAL_DIR)/Source/usbh_*.c)
MODULE_SRCS += $(LOCAL_DIR)/OS/cmsis_os/usbh_os.c

ifeq ($(CONFIG_RNSS_USB), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/HCD/EHCI/usbh_hcd_ehci.c \
	$(LOCAL_DIR)/HCD/OHCI/usbh_hcd_ohci.c
endif

ifeq ($(CONFIG_USB_HOST_HID), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/Class/HID/usbh_hid.c \
	$(LOCAL_DIR)/Class/HID/usbh_hidparser.c
endif

ifeq ($(CONFIG_USB_HOST_MSC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/Class/MSC/usbh_msc.c
endif


include $(MKROOT)/module.mk
