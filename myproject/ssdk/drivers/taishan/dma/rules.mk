LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_DW_PCIE_DMA), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dw_pcie_dma.c
endif

ifeq ($(CONFIG_SDRV_DMA), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sdrv_dma.c \
	$(LOCAL_DIR)/sdrv_dmac.c
endif

include $(MKROOT)/module.mk
