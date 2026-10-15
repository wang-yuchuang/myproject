LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_LIN), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_lin_drv.c
endif

include $(MKROOT)/module.mk
