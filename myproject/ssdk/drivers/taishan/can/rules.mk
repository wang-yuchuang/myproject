LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_FLEXCAN), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/flexcan_driver.c \
	$(LOCAL_DIR)/flexcan_lld.c
endif

include $(MKROOT)/module.mk