LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

MODULE_SRCS += \
	$(LOCAL_DIR)/ext_rst_req.c

include $(MKROOT)/module.mk
