LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc

MODULE_SRCS += \
	$(LOCAL_DIR)/src/animation.c \
	$(LOCAL_DIR)/src/ani_decoder.c \
	$(LOCAL_DIR)/src/ani_player.c \
	$(LOCAL_DIR)/src/ani_res.c

ifeq ($(CONFIG_AUDIO_MANAGER),y)
MODULE_SRCS += $(LOCAL_DIR)/src/ani_audio.c
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable

include $(MKROOT)/module.mk
