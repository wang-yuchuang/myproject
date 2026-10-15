LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
GLOBAL_INCLUDES += $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_ACMP), y)
MODULE_SRCS += $(LOCAL_DIR)/sdrv_acmp_drv.c
endif

include $(MKROOT)/module.mk