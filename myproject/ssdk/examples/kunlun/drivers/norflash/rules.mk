LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_SNOR_TEST), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/norflash_async_test.c \
	$(LOCAL_DIR)/norflash_sync_test.c
endif

ifeq ($(CONFIG_SPI_NOR_VALIDATION), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/validation.c
endif

include $(MKROOT)/module.mk

