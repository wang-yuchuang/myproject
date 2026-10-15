LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDE += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/CLI.c \
	$(LOCAL_DIR)/CLI_console.c \
	$(LOCAL_DIR)/CLI_task.c

EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/CLI.ld

include $(MKROOT)/module.mk
