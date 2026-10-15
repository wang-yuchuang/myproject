LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/UserDef \
	$(LOCAL_DIR)/OperAuth \
	

MODULE_SRCS += \
	$(LOCAL_DIR)/OperAuth/Authority.c \
	$(LOCAL_DIR)/UserDef/UserTactic.c \
	

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-pointer-cast -Wno-unused-but-set-variable

include $(MKROOT)/module.mk