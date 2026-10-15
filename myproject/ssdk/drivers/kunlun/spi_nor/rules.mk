LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_CDNS_OSPI), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/cdns_ospi.c \
	$(LOCAL_DIR)/cdns_ospi_drv.c
endif

include $(MKROOT)/module.mk
