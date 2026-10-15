LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/inc

MODULE_SRCS += \
	$(LOCAL_DIR)/ota.c

ifeq ($(CONFIG_OTA_FROM_ETH_TFTP), y)
MODULE_DEPS += middleware/lwip
MODULE_SRCS += $(LOCAL_DIR)/ota_eth.c
endif

ifeq ($(CONFIG_OTA_FROM_UDISK), y)
MODULE_SRCS += $(LOCAL_DIR)/ota_usbh.c
endif

include $(MKROOT)/module.mk
