LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

#MODULE_DEPS += \
	lib/debug \
	lib/console

MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

MODULE_SRCS += \
	$(LOCAL_DIR)/debugcommands.c

include $(MKROOT)/module.mk
