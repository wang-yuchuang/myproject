LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SPI_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/spi_test.c
endif

include $(MKROOT)/module.mk
