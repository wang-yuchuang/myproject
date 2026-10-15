LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_SBC), y)
MODULE_SRCS += $(LOCAL_DIR)/sbc_demo.c
MODULE_SRCS += $(LOCAL_DIR)/sbc_sys.c
MODULE_SRCS += $(LOCAL_DIR)/TLE94x1.c
MODULE_SRCS += $(LOCAL_DIR)/TLE94x1_SPI.c
endif

include $(MKROOT)/module.mk
