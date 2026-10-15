LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

MODULE_CFLAGS += -I$(SSDK_ROOT)/drivers/taishan/csi/excam/
MODULE_CFLAGS += -I$(SSDK_ROOT)/drivers/taishan/csi/csi_drv/
MODULE_CFLAGS += -I$(SSDK_ROOT)/drivers/taishan/csi/cam_hal/

MODULE_SRCS += \
	$(LOCAL_DIR)/cam_pre.c

include $(MKROOT)/module.mk