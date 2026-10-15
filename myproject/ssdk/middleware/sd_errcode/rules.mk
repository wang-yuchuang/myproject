LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/sd_errcode.c \
	$(LOCAL_DIR)/sd_errcode_cmd.c

ifeq ($(CONFIG_SLIMFS), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sd_errcode_emmc.c
endif

ifeq ($(CONFIG_FEE_EEPROM), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sd_errcode_ospi.c
endif
#MODULE_COMPILEFLAGS += -Wall
MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

ifeq ($(CONFIG_LWIP), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/sd_errcode_net.c
GLOBAL_INCLUDES += middleware/lwip/src/include/compat/posix
MODULE_DEPS += middleware/lwip
endif

include $(MKROOT)/module.mk
