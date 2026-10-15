LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)

ifeq ($(CONFIG_SDRV_PMU), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_pmu_drv.c \
	$(LOCAL_DIR)/sdrv_pmu.c
endif

include $(MKROOT)/module.mk
