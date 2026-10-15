LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_CORE_DEBUG), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/taishan_coredbg.c
endif

include $(MKROOT)/module.mk
