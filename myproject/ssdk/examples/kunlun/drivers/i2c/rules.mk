LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_I2C_TOOLS_VALIDATION), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dw_i2c_validation.c
endif

ifeq ($(CONFIG_I2C_TOOLS_CMD), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/i2ctools.c
endif

include $(MKROOT)/module.mk
