LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc \

MODULE_SRCS += \
	$(LOCAL_DIR)/src/am_stream.c \
	$(LOCAL_DIR)/src/au_agent.c \
	$(LOCAL_DIR)/src/au_capture.c \
	$(LOCAL_DIR)/src/au_hw_device.c \
	$(LOCAL_DIR)/src/au_hw_pcm.c \
	$(LOCAL_DIR)/src/au_mixer.c \
	$(LOCAL_DIR)/src/au_pcm_stream.c \
	$(LOCAL_DIR)/src/au_source.c \
	$(LOCAL_DIR)/src/au_swvol.c \
	$(LOCAL_DIR)/src/ringbuffer.c \
	$(LOCAL_DIR)/src/stream_source.c

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast -Wno-unused-but-set-variable

include $(MKROOT)/module.mk