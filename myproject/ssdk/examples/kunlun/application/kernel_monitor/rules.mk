LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

MODULE_SRCS += \
	$(LOCAL_DIR)/km.c \
	$(LOCAL_DIR)/km_dump.c

include $(MKROOT)/module.mk
