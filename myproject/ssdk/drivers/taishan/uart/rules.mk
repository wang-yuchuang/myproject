LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
GLOBAL_INCLUDES += $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_uart_drv.c \
	$(LOCAL_DIR)/sdrv_uart.c

include $(MKROOT)/module.mk
