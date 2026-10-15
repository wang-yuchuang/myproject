LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_PMU_DEMO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/taishan_pmu_demo.c
endif

include $(MKROOT)/module.mk
