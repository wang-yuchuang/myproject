LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/fee.c

ifeq ($(CONFIG_FEE_BLOCK), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/fee_block.c
endif

ifeq ($(CONFIG_FEE_EEPROM), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/fee_eeprom.c
endif

include $(MKROOT)/module.mk
