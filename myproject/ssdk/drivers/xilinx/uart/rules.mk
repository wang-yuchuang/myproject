LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/xli_uart.c \
    $(LOCAL_DIR)/xli_uart_drv.c

include $(MKROOT)/module.mk