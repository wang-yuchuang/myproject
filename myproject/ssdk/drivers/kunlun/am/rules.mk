LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_AUDIO_BOARD), y)
MODULES += $(LOCAL_DIR)/am_board \
           $(LOCAL_DIR)/am_codec
endif