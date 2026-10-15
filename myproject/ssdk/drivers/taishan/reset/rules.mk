LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_RESETGEN), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/taishan_reset_drv.c \
	$(LOCAL_DIR)/taishan_rstgen.c
endif

include $(MKROOT)/module.mk
