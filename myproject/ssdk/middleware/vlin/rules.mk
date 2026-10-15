LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

ifeq ($(CONFIG_SDPE_RPC_CLIENT), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/vlin_client.c
else
MODULE_SRCS += \
    $(LOCAL_DIR)/vlin_server.c
endif

include $(MKROOT)/module.mk
