LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_XSPI), y)
MODULES += $(LOCAL_DIR)/spi_nor
endif

ifeq ($(CONFIG_SDRV_LIN), y)
MODULES += $(LOCAL_DIR)/sdrv_lin
endif

ifeq ($(CONFIG_SDRV_UART), y)
MODULES += $(LOCAL_DIR)/sdrv_uart
endif
