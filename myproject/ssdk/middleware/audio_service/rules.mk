LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc \

ifdef CONFIG_AUDIO_MANAGER
MODULE_SRCS += \
	$(LOCAL_DIR)/au_rpc_svc.c
endif

ifdef CONFIG_AUDIO_HUB_SVC
MODULE_SRCS += \
	$(LOCAL_DIR)/ahub_stream_svc.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/au_stream_svc.c
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable

include $(MKROOT)/module.mk
