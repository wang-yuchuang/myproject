ASW_DIR := $(GET_LOCAL_DIR)

MODULE := $(ASW_DIR)

include $(ASW_DIR)/libs/rules.mk

MODULE_SRCS += \
	$(ASW_DIR)/sdrv_asw.c \
	$(ASW_DIR)/sdrv_asw_lut_hvk.c

include $(MKROOT)/module.mk
