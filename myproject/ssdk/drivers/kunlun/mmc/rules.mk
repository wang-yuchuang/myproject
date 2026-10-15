LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/core/mmc_sdhci.c \
    $(LOCAL_DIR)/host/sdhci.c

ifeq ($(CONFIG_DWCMSHC), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/host/mmc_dwcmshc.c
endif

ifeq ($(CONFIG_SEHC), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/host/mmc_sehc.c
endif

include $(MKROOT)/module.mk
