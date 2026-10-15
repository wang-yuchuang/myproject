LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_PART_LOAD_UTIL), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/partition_load.c
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_CFLAGS += -Wno-strict-prototypes

include $(MKROOT)/module.mk
