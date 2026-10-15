LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

PROJECT := {}
BOARD := {}
ARCH := {}

include $(LOCAL_DIR)/.config

MODULE_SRCS +={}

GLOBAL_INCLUDES += \
    $(LOCAL_DIR)

# gcc specialized scripts
ifeq ($(SSDK_TOOLCHAIN),gcc)
GLOBAL_LDFLAGS += \
    --defsym MEMBASE=$(CONFIG_MEM_BASE) \
    --defsym MEMSIZE=$(CONFIG_MEM_SIZE)
LINKER_SCRIPT += $(LOCAL_DIR)/$(SSDK_TOOLCHAIN)/$(PROJECT).ld
endif

# iar specialized scripts
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_LINKER_OPTIONS += \
    --config_def MEMBASE=$(CONFIG_MEM_BASE) \
    --config_def MEMSIZE=$(CONFIG_MEM_SIZE)
IAR_LINKCFG := $(LOCAL_DIR)/$(SSDK_TOOLCHAIN)/$(PROJECT).icf
IAR_DEBUG_MACRO := $(LOCAL_DIR)/$(SSDK_TOOLCHAIN)/$(PROJECT).mac
endif

include $(MKROOT)/module.mk
