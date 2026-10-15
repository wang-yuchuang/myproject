LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_KUNLUN_PMU), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_kunlun_pmu_drv.c \
	$(LOCAL_DIR)/sdrv_kunlun_pmu.c \
	$(LOCAL_DIR)/pmu.c
endif


include $(MKROOT)/module.mk
