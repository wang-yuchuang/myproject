LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/disk_api.c

ifeq ($(CONFIG_DISK_CACHE), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_cache.c
endif

ifeq ($(CONFIG_DISK_CACHE_NOOP), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_cache_noop.c
endif

ifeq ($(CONFIG_DISK_RAM), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_ram.c
endif

ifeq ($(CONFIG_DISK_MMC), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_mmc.c
endif

ifeq ($(CONFIG_DISK_NOR_FLASH), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_norflash.c
endif

ifeq ($(CONFIG_DISK_USB), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_usb.c
endif

ifeq ($(CONFIG_DISK_MAPPED), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_mapped.c
endif

include $(MKROOT)/module.mk
