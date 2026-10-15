LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
GLOBAL_INCLUDES += $(LOCAL_DIR)

ifeq ($(CONFIG_SD_SPI), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/sdrv_spi.c
MODULE_SRCS += \
    $(LOCAL_DIR)/spi_lld.c\
	$(LOCAL_DIR)/taishan_ip_spi.c
endif

MODULE_INCLUDES += \
    include/driver/taishan/spi/

include $(MKROOT)/module.mk
