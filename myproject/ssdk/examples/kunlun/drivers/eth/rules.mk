LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/eth_recv.c \
	$(LOCAL_DIR)/eth_send.c

include $(MKROOT)/module.mk

