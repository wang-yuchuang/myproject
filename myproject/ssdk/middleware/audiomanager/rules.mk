LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_AUDIO_MANAGER), y)
MODULES += $(LOCAL_DIR)/am_stream \
           $(LOCAL_DIR)/am_codec
endif

ifeq ($(CONFIG_AUDIO_HUB), y)
MODULES += $(LOCAL_DIR)/am_stream
endif