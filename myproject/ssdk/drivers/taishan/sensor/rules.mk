LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_TEMPERATURE_SENSOR), y)
MODULE_SRCS += $(LOCAL_DIR)/taishan_pvt.c
endif

include $(MKROOT)/module.mk
