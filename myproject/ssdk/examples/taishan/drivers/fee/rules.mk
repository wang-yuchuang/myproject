LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
    $(LOCAL_DIR)

ifeq ($(CONFIG_FEE_EEPROM_DEMO), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/fee_eeprom_tst.c
endif

ifeq ($(CONFIG_FEE_BLOCK_DEMO), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/fee_block_tst.c
endif

include $(MKROOT)/module.mk
