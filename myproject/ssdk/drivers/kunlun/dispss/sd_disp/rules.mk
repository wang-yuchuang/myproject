LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/inc
GLOBAL_INCLUDES += $(LOCAL_DIR)/dsi/inc

ifeq ($(CONFIG_DISPSS_KUNLUN), y)
GLOBAL_INCLUDES += $(LOCAL_DIR)/inc/kunlun
endif

ifeq ($(CONFIG_DISPSS_TAIHANG), y)
GLOBAL_INCLUDES += $(LOCAL_DIR)/inc/taihang
endif

MODULE_SRCS += \
	$(LOCAL_DIR)/dsi/dwc_mipi_dsi_host.c \
	$(LOCAL_DIR)/dsi/disp_dsi_api.c \
	$(LOCAL_DIR)/dsi/disp_dsi.c \
	$(LOCAL_DIR)/core/disp_drv.c \
	$(LOCAL_DIR)/core/disp_interface.c \
	$(LOCAL_DIR)/core/disp_panel.c \
	$(LOCAL_DIR)/core/disp_parallel.c

ifeq ($(CONFIG_DISPSS_KUNLUN), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/core/kunlun/disp_link.c \
	$(LOCAL_DIR)/core/kunlun/disp_ctrl.c \
	$(LOCAL_DIR)/core/kunlun/disp_lvds.c
endif

ifeq ($(CONFIG_DISPSS_TAIHANG), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/core/taihang/disp_link.c \
	$(LOCAL_DIR)/core/taihang/disp_ctrl.c \
	$(LOCAL_DIR)/core/taihang/disp_lvds.c
endif
