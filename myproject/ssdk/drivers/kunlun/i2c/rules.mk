LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_DW_I2C), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dw_i2c.c
endif

include $(MKROOT)/module.mk
