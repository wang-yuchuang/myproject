LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)


MODULE_SRCS += $(wildcard $(LOCAL_DIR)/*.c)

MODULE_SRCS += $(wildcard $(LOCAL_DIR)/HID/*.c)


include $(MKROOT)/module.mk
