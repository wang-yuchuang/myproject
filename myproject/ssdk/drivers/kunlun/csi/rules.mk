LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_BOARD_X9SP), y)
GLOBAL_INCLUDES += $(LOCAL_DIR)/csi_drv_v3/inc
else
GLOBAL_INCLUDES += $(LOCAL_DIR)/csi_drv/inc
endif

GLOBAL_INCLUDES += $(LOCAL_DIR)/cam_hal

MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

MODULE_INCLUDES += \
	$(LOCAL_DIR)/excam/inc \
	$(LOCAL_DIR)/cam_hal

ifeq ($(CONFIG_BOARD_X9SP), y)
MODULE_INCLUDES += $(LOCAL_DIR)/csi_drv_v3/inc
else
MODULE_INCLUDES += $(LOCAL_DIR)/csi_drv/inc
endif

ifeq ($(CONFIG_BOARD_X9SP), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/csi_drv_v3/src/csi_hw.c \
	$(LOCAL_DIR)/csi_drv_v3/src/csi_drv.c \
	$(LOCAL_DIR)/csi_drv_v3/src/mipicsi-ip.c \
	$(LOCAL_DIR)/csi_drv_v3/src/mipicsi-os.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/csi_drv/src/csi_hw.c \
	$(LOCAL_DIR)/csi_drv/src/csi_drv.c \
	$(LOCAL_DIR)/csi_drv/src/mipicsi-ip.c \
	$(LOCAL_DIR)/csi_drv/src/mipicsi-os.c
endif

MODULE_SRCS += $(LOCAL_DIR)/cam_hal/cam_hal.c

ifeq ($(CONFIG_OV5640), y)
MODULE_SRCS += $(LOCAL_DIR)/excam/src/ex_ov5640.c
endif

ifeq ($(CONFIG_VIRTUAL_CAMERA), y)
MODULE_SRCS += $(LOCAL_DIR)/excam/src/ex_vCamera.c
endif

ifeq ($(CONFIG_DESER), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/excam/src/poc-max2008x.c \
	$(LOCAL_DIR)/excam/src/sdrv_serdes_core.c
endif

ifeq ($(CONFIG_mipi_max9286_fengjing_MC163_720P_MAX96705), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/excam/src/mipi_max9286_fengjing_MC163_720P_MAX96705.c
endif

ifeq ($(CONFIG_dvp_max96706_fengjing_MC163_MAX96705), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/excam/src/dvp_max96706_fengjing_MC163_MAX96705.c
endif

ifeq ($(CONFIG_mipi_max96722_sengyun_IMX390C_5200_MAX9295), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/excam/src/mipi_max96722_sengyun_IMX390C_5200_MAX9295.c
endif

ifeq ($(CONFIG_mipi_max96722_fengjing_MC163_MAX96705), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/excam/src/mipi_max96722_fengjing_MC163_MAX96705.c
endif

MODULE_SRCS += $(LOCAL_DIR)/excam/src/exdev.c

include $(MKROOT)/module.mk
