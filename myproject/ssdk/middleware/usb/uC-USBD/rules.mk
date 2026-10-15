LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)


MODULE_SRCS += $(wildcard $(LOCAL_DIR)/Source/usbd_*.c)
MODULE_SRCS += $(LOCAL_DIR)/OS/cmsis_os/usbd_os.c

ifeq ($(CONFIG_RNSS_USB), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/Drivers/Renesas_USBHS/usbd_drv_renesas_usbhs.c \
	$(LOCAL_DIR)/Drivers/drv_lib/usbd_drv_lib.c
endif

ifeq ($(CONFIG_USB_DEVICE_HID), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/Class/HID/usbd_hid.c \
	$(LOCAL_DIR)/Class/HID/usbd_hid_report.c \
	$(LOCAL_DIR)/Class/HID/OS/cmsis_os/usbd_hid_os.c
endif

ifeq ($(CONFIG_USB_DEVICE_MSC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/Class/MSC/usbd_msc.c \
	$(LOCAL_DIR)/Class/MSC/usbd_scsi.c \
	$(LOCAL_DIR)/Class/MSC/OS/cmsis_os/usbd_msc_os.c \
	$(LOCAL_DIR)/Class/MSC/Storage/RAMDisk/usbd_storage.c
endif

ifeq ($(CONFIG_USB_DEVICE_FB), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/Class/FastBoot/usbd_fb.c \
	$(LOCAL_DIR)/Class/FastBoot/OS/cmsis_os/usbd_fb_os.c
endif

ifeq ($(CONFIG_USB_DEVICE_CDC_ACM), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/Class/CDC/usbd_cdc.c \
	$(LOCAL_DIR)/Class/CDC/ACM/usbd_acm_serial.c
endif


include $(MKROOT)/module.mk
