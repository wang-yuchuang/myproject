LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDE += \
	$(LOCAL_DIR)/v2.1

MODULE_SRCS += \
	$(LOCAL_DIR)/v2.1/dhry_1.c \
	$(LOCAL_DIR)/v2.1/dhry_2.c \
	$(LOCAL_DIR)/v2.1/dhry.c

include $(MKROOT)/module.mk
