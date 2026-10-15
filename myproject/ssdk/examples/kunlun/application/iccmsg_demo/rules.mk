LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc \
	$(LOCAL_DIR)/../../../../middleware/iccmsg \

MODULE_SRCS += \
	$(LOCAL_DIR)/src/iccmsg_demo.c

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-unused-but-set-variable

include $(MKROOT)/module.mk
