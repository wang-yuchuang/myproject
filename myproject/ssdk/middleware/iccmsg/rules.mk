LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/iccmsg_dpr.c  \
	$(LOCAL_DIR)/iccmsg.c \
	$(LOCAL_DIR)/sysmgr_default.c


include $(MKROOT)/module.mk
