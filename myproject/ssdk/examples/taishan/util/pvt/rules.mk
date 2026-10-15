LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_TEMPERATURE_EXAMPLE), y)
MODULE_SRCS += $(LOCAL_DIR)/taishan_pvt_example.c
endif

include $(MKROOT)/module.mk
