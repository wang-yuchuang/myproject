LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_VIRTIO_MMC_FRONTEND),y)
MODULE_DEPS += $(LOCAL_DIR)/mmc
endif

MODULE_COMPILEFLAGS += -Wall -Werror

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_SRCS += $(LOCAL_DIR)/virtio_common.c

ifeq ($(CONFIG_VIRTIO_BACKEND),y)
MODULE_CFLAGS += -D__HAS_BE=1
MODULE_SRCS += $(LOCAL_DIR)/virtio_backend.c
MODULE_SRCS += $(LOCAL_DIR)/virtio_backend_sa.c
else
MODULE_CFLAGS += -D__HAS_BE=0
endif

ifeq ($(CONFIG_VIRTIO_FRONTEND),y)
MODULE_CFLAGS += -D__HAS_FE=1
MODULE_SRCS += $(LOCAL_DIR)/virtio_frontend.c
else
MODULE_CFLAGS += -D__HAS_FE=0
endif

# keep it same as domain definition in virtio_common.h
ifeq ($(CORE),safety)
MODULE_CFLAGS += -D__MY_DOMAIN=0
MODULE_CFLAGS += -D__HAS_MMU=0
else ifeq ($(CORE),secure)
MODULE_CFLAGS += -D__MY_DOMAIN=1
MODULE_CFLAGS += -D__HAS_MMU=0
else ifeq ($(CORE),mp)
MODULE_CFLAGS += -D__MY_DOMAIN=2
MODULE_CFLAGS += -D__HAS_MMU=0
else ifeq ($(CORE),$(findstring $(CORE),ap ap1 ecockpit))
MODULE_CFLAGS += -D__MY_DOMAIN=3
MODULE_CFLAGS += -D__HAS_MMU=1
else ifeq ($(CORE),$(findstring $(CORE),ap2 cluster))
MODULE_CFLAGS += -D__MY_DOMAIN=4
MODULE_CFLAGS += -D__HAS_MMU=1
else

# other core such as sdpe should not enable CONFIG_VIRTIO_FRONTEND or
# CONFIG_VIRTIO_BACKEND, so __HAS_MMU will not be used
MODULE_CFLAGS += -D__MY_DOMAIN=5

ifeq ($(CONFIG_VIRTIO_BACKEND),y)
$(error virtio backend is not supported for core "$(CORE)")
endif
ifeq ($(CONFIG_VIRTIO_FRONTEND),y)
$(error virtio frontend is not supported for core "$(CORE)")
endif

endif

include $(MKROOT)/module.mk
