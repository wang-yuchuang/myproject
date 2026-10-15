LOCAL_DIR := $(GET_LOCAL_DIR)


GLOBAL_INCLUDES += \
    $(LOCAL_DIR)/include \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN) \
    $(LOCAL_DIR)/d9

ifneq ($(wildcard $(LOCAL_DIR)/*.c),)

ifeq ($(wildcard $(COREDIR)/libcpu.a),)
MODULE_STATIC_LIB := true
endif

MODULE := $(LOCAL_DIR)
MODULE_SRCS += \
	$(LOCAL_DIR)/$(SSDK_TOOLCHAIN)/asm.S \
	$(LOCAL_DIR)/$(SSDK_TOOLCHAIN)/spinlock.S \
	$(LOCAL_DIR)/$(SSDK_TOOLCHAIN)/start.S \
	$(LOCAL_DIR)/$(SSDK_TOOLCHAIN)/cache-ops.S \
	$(LOCAL_DIR)/$(SSDK_TOOLCHAIN)/atomic.S \
	$(LOCAL_DIR)/arch.c \
	$(LOCAL_DIR)/mmu_setup.c \
	$(LOCAL_DIR)/interrupts.c

ifeq ($(CONFIG_ARCH_WITH_SMP), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/smp.c
endif
else
GLOBAL_COMPILEFLAGS += -DLIBCPU_VERSION
endif


ARCH_OPTFLAGS := -O2


TOOLCHAIN_PREFIX := $(abspath $(LOCAL_DIR)/../../prebuilts/linux/gcc/arm64/bin/aarch64-elf-)

# try to find the toolchain
include $(LOCAL_DIR)/toolchain.mk
TOOLCHAIN_PREFIX := $(ARCH_$(ARCH)_TOOLCHAIN_PREFIX)
$(info TOOLCHAIN_PREFIX = $(TOOLCHAIN_PREFIX))
ARCH_COMPILEFLAGS += $(ARCH_$(ARCH)_COMPILEFLAGS)

GLOBAL_LDFLAGS += -z max-page-size=4096

include $(MKROOT)/module.mk

MODULE_DEPS += arch/arm64/d9

