LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_VIRT_UART), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/virt_uart.c \
	$(LOCAL_DIR)/logbuf.c
endif

include $(MKROOT)/module.mk
