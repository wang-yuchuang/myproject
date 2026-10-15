LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/rpmsg.c \
	$(LOCAL_DIR)/rpmsg_ns.c \
	$(LOCAL_DIR)/rpmsg_channel.c

ifeq ($(CONFIG_RPMSG_VIRTIO), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/virtio/rpmsg_env.c \
	$(LOCAL_DIR)/virtio/rpmsg_platform.c \
	$(LOCAL_DIR)/virtio/rpmsg_virtio.c \
	$(LOCAL_DIR)/virtio/virtqueue.c
endif

ifeq ($(CONFIG_RPMSG_IPCC), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/ipcc/rpmsg_ipcc.c
endif

include $(MKROOT)/module.mk
