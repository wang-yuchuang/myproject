DISPSS_DIR := $(GET_LOCAL_DIR)

MODULE := $(DISPSS_DIR)

include $(DISPSS_DIR)/lcm/rules.mk
include $(DISPSS_DIR)/libs/rules.mk

MODULE_SRCS += \
	$(LCM_DIR)/sdrv_panels.c \
	$(DISPSS_DIR)/sdrv_dispss.c \

include $(MKROOT)/module.mk
