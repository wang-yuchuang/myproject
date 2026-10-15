LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/inc

MODULE_SRCS += \
	$(LOCAL_DIR)/src/maxim_rw.c \
	$(LOCAL_DIR)/src/maxim_967xx.c