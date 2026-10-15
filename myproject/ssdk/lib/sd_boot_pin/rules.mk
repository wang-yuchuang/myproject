LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SD_BOOT_PIN), y)

MODULE_SRCS += \
	$(LOCAL_DIR)/sd_boot_pin.c

endif
include $(MKROOT)/module.mk
