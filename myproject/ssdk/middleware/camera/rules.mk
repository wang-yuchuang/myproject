LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
MODULE_CFLAGS += -D_CONFIG_OS_SSDK_

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/cam_service.c

MODULE_DEFINE += CAM_BUFFER_COPY=1

ifeq ($(CONFIG_CSI_SVC_VIA_UNILINK), y)
GLOBAL_INCLUDES += middleware/lwip/src/include/compat/posix
MODULE_DEPS += middleware/lwip
endif


include $(MKROOT)/module.mk

