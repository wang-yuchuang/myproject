LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_XSPI_SLV), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/taishan_xspi_slv.c
endif

ifeq ($(CONFIG_XSPI_MST), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/taishan_xspi_mst.c
endif

include $(MKROOT)/module.mk
