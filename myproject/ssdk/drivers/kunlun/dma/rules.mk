LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_DW_DMA), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dw_dma.c \
	$(LOCAL_DIR)/dw_dmac.c
endif

ifeq ($(CONFIG_DW_PCIE_DMA), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dw_pcie_dma.c
endif

include $(MKROOT)/module.mk
