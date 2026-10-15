LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/pvt_app.c \

ifeq ($(CONFIG_PVT_APPLICATION_PRINT), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/pvt_print.c
endif

ifeq ($(CONFIG_PVT_APPLICATION_MONITOR), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/pvt_monitor.c
endif

include $(MKROOT)/module.mk
