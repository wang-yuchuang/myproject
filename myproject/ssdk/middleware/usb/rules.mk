LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/lib_mem.c

ifeq ($(CONFIG_USB_DEVICE), y)
MODULE_DEPS += middleware/usb/uC-USBD
endif

ifeq ($(CONFIG_USB_HOST), y)
MODULE_DEPS += middleware/usb/uC-USBH
endif

ifeq ($(CONFIG_USB_OTG), y)
MODULE_SRCS += $(LOCAL_DIR)/usb_otg_e3.c
endif

ifneq ($(CONFIG_USB_DEVICE)$(CONFIG_USB_HOST), )
MODULE_SRCS += $(LOCAL_DIR)/usb_bsp_e3.c

GLOBAL_INCLUDES += $(LOCAL_DIR)/include
endif

include $(MKROOT)/module.mk
