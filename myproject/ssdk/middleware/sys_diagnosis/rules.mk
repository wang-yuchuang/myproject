LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \

MODULE_SRCS += \
	$(LOCAL_DIR)/sys_diagnosis.c

include $(MKROOT)/module.mk