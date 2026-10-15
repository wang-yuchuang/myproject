LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_BOARD_X9SP), y)
GLOBAL_INCLUDES += $(LOCAL_DIR)/thang
else
GLOBAL_INCLUDES += $(LOCAL_DIR)/kunlun
endif

ifeq ($(CONFIG_KUNLUN_RESETGEN), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_reset_drv.c \
	$(LOCAL_DIR)/sdrv_rstgen.c
endif

include $(MKROOT)/module.mk
