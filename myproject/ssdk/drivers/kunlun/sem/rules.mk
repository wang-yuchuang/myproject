LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_SEM), y)
GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/kunlun_sem_lld.c \
	$(LOCAL_DIR)/sdrv_sem_drv_kunlun.c
endif
include $(MKROOT)/module.mk
