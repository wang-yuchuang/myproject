LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc

MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

MODULE_CFLAGS += -I$(SSDK_ROOT)/include/driver/taishan/dispss/
MODULE_CFLAGS += -I$(SSDK_ROOT)/drivers/taishan/csi/excam/
MODULE_CFLAGS += -I$(SSDK_ROOT)/drivers/taishan/csi/csi_drv/
MODULE_CFLAGS += -I$(SSDK_ROOT)/drivers/taishan/csi/cam_hal/

MODULE_SRCS += \
	$(LOCAL_DIR)/src/emirror_lvgl.c \
	$(LOCAL_DIR)/src/emirror_app.c \
	$(LOCAL_DIR)/src/on.c \
	$(LOCAL_DIR)/src/off.c

ifeq ($(CONFIG_ASW), y)
MODULE_SRCS += $(LOCAL_DIR)/src/disp_demo_api.c
endif

include $(MKROOT)/module.mk
