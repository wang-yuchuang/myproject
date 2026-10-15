LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/mjpeg_test.c
MODULE_SRCS += $(LOCAL_DIR)/cluster_bin.c

include $(MKROOT)/module.mk
