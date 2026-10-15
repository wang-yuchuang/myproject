LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

ifeq ($(CONFIG_STR_STORE_SUPPORT), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/str_store.c
endif

ifeq ($(CONFIG_LOWER_POWER_OS), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/str.c
endif

ifeq ($(CONFIG_SAVE_DDR_INFO_IN_EMMC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/str_commu.c
endif

include $(MKROOT)/module.mk
