LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_KUNLUN_CKGEN), y)
MODULE_SRCS += $(LOCAL_DIR)/kunlun_ckgen_drv.c $(LOCAL_DIR)/kunlun_ckgen.c
endif

ifeq ($(CONFIG_SC_PLL), y)
MODULE_SRCS += $(LOCAL_DIR)/sc_pfpll_drv.c $(LOCAL_DIR)/sc_pfpll.c
endif

include $(MKROOT)/module.mk