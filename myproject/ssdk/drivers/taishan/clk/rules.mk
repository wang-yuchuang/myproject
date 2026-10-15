LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
GLOBAL_INCLUDES += $(LOCAL_DIR)

ifeq ($(CONFIG_TAISHAN_CKGEN), y)
MODULE_SRCS += $(LOCAL_DIR)/taishan_ckgen_drv.c $(LOCAL_DIR)/taishan_ckgen.c
endif

ifeq ($(CONFIG_TAISHAN_PLL), y)
MODULE_SRCS += $(LOCAL_DIR)/taishan_pll_drv.c $(LOCAL_DIR)/taishan_pll.c
endif

include $(MKROOT)/module.mk