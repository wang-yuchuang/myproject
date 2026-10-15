LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/mmc_test.c \
	$(LOCAL_DIR)/sdcard_test.c

include $(MKROOT)/module.mk

