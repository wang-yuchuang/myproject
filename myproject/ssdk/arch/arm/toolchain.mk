ifndef ARCH_arm_TOOLCHAIN_INCLUDED
ARCH_arm_TOOLCHAIN_INCLUDED := 1

ifeq ($(SSDK_TOOLCHAIN),gcc)
# try to find the toolchain
ifndef ARCH_arm_TOOLCHAIN_PREFIX

FOUNDTOOL=

# if TOOLCHAIN_PREFIX is not empty and ARCH_arm64_TOOLCHAIN_PREFIX is empty,
# try to use TOOLCHAIN_PREFIX first
ifneq ($(TOOLCHAIN_PREFIX),)
ifeq ($(ARCH_arm64_TOOLCHAIN_PREFIX),)
ARCH_arm_TOOLCHAIN_PREFIX := $(TOOLCHAIN_PREFIX)
FOUNDTOOL=$(shell which $(ARCH_arm_TOOLCHAIN_PREFIX)gcc)
endif
endif

# try a series of common arm toolchain prefixes in the path
ifeq ($(FOUNDTOOL),)
ARCH_arm_TOOLCHAIN_PREFIX := arm-eabi-
FOUNDTOOL=$(shell which $(ARCH_arm_TOOLCHAIN_PREFIX)gcc)
endif
ifeq ($(FOUNDTOOL),)
ARCH_arm_TOOLCHAIN_PREFIX := arm-elf-
FOUNDTOOL=$(shell which $(ARCH_arm_TOOLCHAIN_PREFIX)gcc)
endif
ifeq ($(FOUNDTOOL),)
ARCH_arm_TOOLCHAIN_PREFIX := arm-none-eabi-
FOUNDTOOL=$(shell which $(ARCH_arm_TOOLCHAIN_PREFIX)gcc)
endif
ifeq ($(FOUNDTOOL),)
ARCH_arm_TOOLCHAIN_PREFIX := arm-linux-gnueabi-
FOUNDTOOL=$(shell which $(ARCH_arm_TOOLCHAIN_PREFIX)gcc)

# Set no stack protection if we found our gnueabi toolchain. We don't
# need it.
#
# Stack protection is default in this toolchain and we get such errors
# final linking stage:
#
# undefined reference to `__stack_chk_guard'
# undefined reference to `__stack_chk_fail'
# undefined reference to `__stack_chk_guard'
#
ifneq (,$(findstring arm-linux-gnueabi-,$(FOUNDTOOL)))
        ARCH_arm_COMPILEFLAGS += -fno-stack-protector
endif
endif # arm-linux-gnueabi-

else
FOUNDTOOL=$(shell which $(ARCH_arm_TOOLCHAIN_PREFIX)gcc)
endif # ARCH_arm_TOOLCHAIN_PREFIX

ifeq ($(FOUNDTOOL),)
$(error cannot find toolchain, please set ARCH_arm_TOOLCHAIN_PREFIX or add it to your path)
endif
endif

ifeq ($(CONFIG_ARCH_CORTEXR5), y)
ARCH_arm_COMPILEFLAGS += -mcpu=cortex-r5
ifeq ($(CONFIG_ARCH_USE_FPU), y)
ifeq ($(CONFIG_ARM_FPU_D16), y)
ARCH_arm_COMPILEFLAGS += -mfpu=vfpv3-d16 -mfloat-abi=hard
else
ARCH_arm_COMPILEFLAGS += -mfpu=vfpv3 -mfloat-abi=hard
endif
else
ARCH_arm_COMPILEFLAGS += -mfloat-abi=soft
endif
endif

endif
