LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/sdrv_sem.c \
	$(LOCAL_DIR)/sdrv_sem_drv.c


include $(MKROOT)/module.mk