LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SD_BOOT_IMG), y)

MODULE_SRCS += \
	$(LOCAL_DIR)/sd_boot_img.c

endif
include $(MKROOT)/module.mk
