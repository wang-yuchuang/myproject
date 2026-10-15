LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/pm_demo.c \
	$(LOCAL_DIR)/lp_demo.c

include $(MKROOT)/module.mk
