LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/rpmsg_service.c \
	$(LOCAL_DIR)/rpmsg_echo.c

ifeq ($(CONFIG_USE_IPCC_RPC), y)
	MODULE_SRCS += \
		$(LOCAL_DIR)/rpc/rpmsg_rpc.c \
		$(LOCAL_DIR)/rpc/dcf_rpc.c
endif

include $(MKROOT)/module.mk
