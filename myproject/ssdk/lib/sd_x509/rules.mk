LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)
ifeq ($(CONFIG_SD_X509), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sd_x509.c
endif
GLOBAL_INCLUDES += $(LOCAL_DIR) \
	lib/sd_x509 \
	include

MODULE_CFLAGS += -Wno-strict-prototypes

include $(MKROOT)/module.mk
