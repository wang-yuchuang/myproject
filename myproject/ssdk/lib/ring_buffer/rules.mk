LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/ring_buffer.c

ifeq ($(CONFIG_ERING_BUFFER), y)
MODULE_SRCS += $(LOCAL_DIR)/ering_buffer.c
endif

include $(MKROOT)/module.mk
