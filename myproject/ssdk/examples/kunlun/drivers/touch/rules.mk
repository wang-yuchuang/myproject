LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	${LOCAL_DIR}/touch_test.c

include $(MKROOT)/module.mk
