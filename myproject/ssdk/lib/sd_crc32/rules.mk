LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)
ifeq ($(CONFIG_SD_CRC32), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sd_crc32.c
endif
GLOBAL_INCLUDES += $(LOCAL_DIR) \
	include/lib/sd_crc32 \
	include

MODULE_CFLAGS += -Wno-strict-prototypes

include $(MKROOT)/module.mk
