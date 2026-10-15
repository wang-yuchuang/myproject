LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/whetstone.c \
	$(LOCAL_DIR)/linpack.c \
	$(LOCAL_DIR)/flops.c \
	$(LOCAL_DIR)/flops_main.c

include $(MKROOT)/module.mk
