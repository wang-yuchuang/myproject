LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/xli_ttc.c \
    $(LOCAL_DIR)/xli_ttc_drv.c

include $(MKROOT)/module.mk