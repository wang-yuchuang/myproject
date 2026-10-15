DISPSS_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_DISPSS), y)
include $(DISPSS_DIR)/lcm/rules.mk
include $(DISPSS_DIR)/display/rules.mk
include $(DISPSS_DIR)/disp_hal/rules.mk
include $(DISPSS_DIR)/sd_disp/rules.mk

ifeq ($(CONFIG_DISP_VLDN), y)
include $(DISPSS_DIR)/disp_vldn/rules.mk
endif

endif
