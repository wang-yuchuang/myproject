LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_UART_LIN_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_uart_demo.c
endif

include $(MKROOT)/module.mk
