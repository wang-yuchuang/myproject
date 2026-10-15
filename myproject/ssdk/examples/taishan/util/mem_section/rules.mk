LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_MEM_SECTION_EXAMPLE), y)
MODULE_SRCS += $(LOCAL_DIR)/mem_section_example.c
endif

include $(MKROOT)/module.mk
