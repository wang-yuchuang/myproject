LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/disp_share/disp_share.c \
	$(LOCAL_DIR)/safety_disp.c
#MODULE_DEPS += framework/communication

ifeq ($(SUPPORT_DISPLAY_UNILINK),true)
MODULE_SRCS += \
	$(LOCAL_DIR)/safety_ulink.c
MODULE_DEPS += framework/service/ulink
endif

include $(MKROOT)/module.mk
