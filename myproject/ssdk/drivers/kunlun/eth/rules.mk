LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS := \
	middleware/lwip

ifeq ($(CONFIG_DWC_ETH), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/dwc_eth.c \
	$(LOCAL_DIR)/dw_mac_lld.c
endif

MODULE_SRCS += \
	$(LOCAL_DIR)/phy/phy.c \
	$(LOCAL_DIR)/phy/q21xx.c \
	$(LOCAL_DIR)/phy/dp83848.c \
	$(LOCAL_DIR)/phy/rtl9010.c \
	$(LOCAL_DIR)/phy/yt8521.c

include $(MKROOT)/module.mk
