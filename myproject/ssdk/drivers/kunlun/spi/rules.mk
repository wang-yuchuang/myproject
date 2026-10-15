LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_DW_SPI), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/dw_spi_drv.c
endif

MODULE_INCLUDES += \
    include/driver/kunlun/spi/

include $(MKROOT)/module.mk
