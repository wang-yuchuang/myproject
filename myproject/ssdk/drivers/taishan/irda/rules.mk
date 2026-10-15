LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_IRDA), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_irda_drv.c
endif

include $(MKROOT)/module.mk
