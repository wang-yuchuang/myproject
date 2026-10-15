LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/lock.c \
	$(LOCAL_DIR)/syscalls.c \
	$(LOCAL_DIR)/assert.c \
	$(LOCAL_DIR)/aeabi_atexit.c
ifeq ($(ARCH),arm64)
MODULE_SRCS += \
	$(LOCAL_DIR)/tiny-printf.c
endif
ifneq ($(CONFIG_STD_HEAP), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/mallocr.c
endif

include $(MKROOT)/module.mk
