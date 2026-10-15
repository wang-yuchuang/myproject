LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/sd_ddr_pfmon.c \
	$(LOCAL_DIR)/pfm_axi_master_name.c \

include $(MKROOT)/module.mk
