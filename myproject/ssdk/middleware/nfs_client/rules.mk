LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += middleware/lwip

GLOBAL_INCLUDES += \
    $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/nfs.c  \
    $(LOCAL_DIR)/nfs_api.c

include mk/module.mk
