LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_I2C), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_i2c_drv.c \
	$(LOCAL_DIR)/sdrv_i2c.c
endif

include $(MKROOT)/module.mk