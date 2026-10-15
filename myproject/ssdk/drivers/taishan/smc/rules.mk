LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/sdrv_smc.c \
    $(LOCAL_DIR)/sdrv_smc_drv.c

include $(MKROOT)/module.mk
