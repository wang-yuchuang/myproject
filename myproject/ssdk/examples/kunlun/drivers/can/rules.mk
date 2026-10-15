LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/can_test.c \
	$(LOCAL_DIR)/can_demo.c

include $(MKROOT)/module.mk