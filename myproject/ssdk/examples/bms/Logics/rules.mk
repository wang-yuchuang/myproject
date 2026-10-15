LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/Balance \
	$(LOCAL_DIR)/CanUser \
	$(LOCAL_DIR)/Tactics \

MODULE_SRCS += \
	$(LOCAL_DIR)/Balance/BalanTactic.c \
	$(LOCAL_DIR)/CanUser/SlaveMsg.c \
	$(LOCAL_DIR)/CanUser/AuxDevMsg.c \
	$(LOCAL_DIR)/Tactics/AuxDevTactic.c \
	$(LOCAL_DIR)/Tactics/FireTactic.c \
	$(LOCAL_DIR)/Tactics/GroupDebug.c \
	$(LOCAL_DIR)/Tactics/GroupMode.c \
	$(LOCAL_DIR)/Tactics/GroupProtect.c \
	$(LOCAL_DIR)/Tactics/GroupState.c \
	$(LOCAL_DIR)/Tactics/GroupTactic.c \
	$(LOCAL_DIR)/Tactics/TacInterface.c \
	$(LOCAL_DIR)/Tactics/TacUser.c \

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-unused-but-set-variable

include $(MKROOT)/module.mk
