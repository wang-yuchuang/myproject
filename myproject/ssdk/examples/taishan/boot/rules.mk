LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_BOOT_CORE_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/boot_core
endif

ifeq ($(CONFIG_GLOBAL_REBOOT_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/global_reboot
endif

ifeq ($(CONFIG_LOAD_IMAGE_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/load_image
endif
