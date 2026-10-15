LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

ifeq ($(CONFIG_SDLOGC),y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdlog.c
endif

ifeq ($(CONFIG_SDLOGD),y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdlogd.c
endif

ifeq ($(CONFIG_SDSHELL_BACKEND),y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdshell_be.c
endif

include $(MKROOT)/module.mk
