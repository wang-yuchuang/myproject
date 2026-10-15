LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	include/driver/kunlun/mmc/ \
	lib/verified_boot/include/ \
	lib/libavb/ \
	include/lib/partition/

ifeq ($(CONFIG_KUNLUN_REBOOT), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/reboot.c
endif

ifeq ($(CONFIG_TAISHAN_REBOOT), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/reboot_taishan.c
endif

ifeq ($(CONFIG_TAISHAN_REBOOT_GLOBAL), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/reboot_taishan_global.c
endif

include $(MKROOT)/module.mk
