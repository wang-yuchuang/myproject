LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_I2C_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_i2c_demo.c
endif

include $(MKROOT)/module.mk
