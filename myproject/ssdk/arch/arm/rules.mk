LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
    $(LOCAL_DIR)/include

ARCH_OPTFLAGS := -O2

ifeq ($(CONFIG_ARM_THUMB),y)
THUMBCFLAGS := -mthumb -D__thumb__
endif

TOOLCHAIN_PREFIX := $(abspath $(LOCAL_DIR)/../../prebuilts/$(OS)/gcc/arm/bin/arm-none-eabi-)

# try to find toolchain
include $(LOCAL_DIR)/toolchain.mk
$(info TOOLCHAIN_PREFIX = $(TOOLCHAIN_PREFIX))

ARCH_COMPILEFLAGS += $(ARCH_$(ARCH)_COMPILEFLAGS)

#include $(MKROOT)/module.mk

ifeq ($(CONFIG_ARCH_FAMILY),"armv7-r")
MODULE_DEPS += arch/arm/armv7-r
endif

ifeq ($(CONFIG_ARCH_CHIP_G9),y)
MODULE_DEPS += arch/arm/g9
endif

ifeq ($(CONFIG_ARCH_CHIP_X9),y)
MODULE_DEPS += arch/arm/x9
endif

ifeq ($(CONFIG_ARCH_CHIP_D9),y)
MODULE_DEPS += arch/arm/d9
endif

ifeq ($(CONFIG_ARCH_CHIP_E3),y)
MODULE_DEPS += arch/arm/e3
endif

ifeq ($(CONFIG_ARCH_CHIP_D3),y)
MODULE_DEPS += arch/arm/d3
endif