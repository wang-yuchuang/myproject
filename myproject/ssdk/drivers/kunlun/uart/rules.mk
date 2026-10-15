LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_DW_UART), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dw_uart_drv.c \
	$(LOCAL_DIR)/dw_uart.c
endif

include $(MKROOT)/module.mk
