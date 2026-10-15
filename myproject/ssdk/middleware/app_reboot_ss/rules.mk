LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \

MODULE_SRCS += \
	$(LOCAL_DIR)/app_reboot_ss.c \
	$(LOCAL_DIR)/str_api.c

include $(MKROOT)/module.mk