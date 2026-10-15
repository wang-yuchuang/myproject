LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_UART_LIN), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/uart_lin_drv.c
endif

include $(MKROOT)/module.mk
