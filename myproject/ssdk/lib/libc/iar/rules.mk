LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/io.c

# if use RTOS heap instead of IAR std heap
ifneq ($(CONFIG_STD_HEAP), y)
MODULE_SRCS += $(LOCAL_DIR)/malloc.c
endif

include $(MKROOT)/module.mk
