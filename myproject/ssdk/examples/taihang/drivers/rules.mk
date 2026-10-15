LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_SDRV_LIN_DEMO), y)
MODULES += $(LOCAL_DIR)/sdrv_lin
endif

ifeq ($(CONFIG_SDRV_UART_DEMO), y)
MODULES += $(LOCAL_DIR)/sdrv_uart
endif