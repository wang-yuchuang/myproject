LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
    $(LOCAL_DIR)

ifeq ($(CONFIG_DISK_RAM_DEMO), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_ram_test.c
endif

ifeq ($(CONFIG_DISK_MMC_DEMO), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_mmc_test.c
endif

ifeq ($(CONFIG_DISK_NOR_FLASH_DEMO), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/disk_norflash_test.c
endif

include $(MKROOT)/module.mk
