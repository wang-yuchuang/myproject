LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_AM_BOARD_X9_MS), y)
MODULES += $(LOCAL_DIR)/msa03
endif

ifeq ($(CONFIG_AM_BOARD_X9_REF), y)
MODULES += $(LOCAL_DIR)/refa03 \
		   $(LOCAL_DIR)/refa04
endif