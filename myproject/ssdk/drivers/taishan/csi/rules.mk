LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

MODULE_INCLUDES += \
	include/driver/pinctrl \
	include/driver/gpio \
	$(LOCAL_DIR)/excam \
	$(LOCAL_DIR)/csi_drv \
	$(LOCAL_DIR)/cam_hal

ifeq ($(CONFIG_VALIDATION), y)
MODULE_CFLAGS += -D_PLATFORM_SIMULATION_=1
endif

MODULE_SRCS += \
	$(LOCAL_DIR)/csi_drv/csi_hw.c \
	$(LOCAL_DIR)/csi_drv/csi_drv.c \
	$(LOCAL_DIR)/cam_hal/cam_hal.c

ifeq ($(CONFIG_OV5640), y)
MODULE_SRCS += $(LOCAL_DIR)/excam/ex_ov5640.c
endif

ifeq ($(CONFIG_MAX96706), y)
MODULE_SRCS += $(LOCAL_DIR)/excam/ex_max96706.c
endif

ifeq ($(CONFIG_TI926), y)
MODULE_SRCS += $(LOCAL_DIR)/excam/ex_ti926.c
endif

ifeq ($(CONFIG_VIRTUAL_CAMERA), y)
MODULE_SRCS += $(LOCAL_DIR)/excam/ex_vCamera.c
endif

MODULE_SRCS += $(LOCAL_DIR)/excam/exdev.c

include $(MKROOT)/module.mk
