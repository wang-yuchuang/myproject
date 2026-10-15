LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

SSDK_TOOLCHAIN_UPPER := $(shell echo ${SSDK_TOOLCHAIN} | tr a-z A-Z)

ifeq ($(CONFIG_ARCH_CORTEXR5), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_UPPER)/ARM_CR5/exceptions.S
endif

ifeq ($(CONFIG_ARCH_CORTEXA55), y)
MODULE_SRCS += \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_UPPER)/ARM_A55/exceptions.S
endif
MODULE_SRCS += \
    $(LOCAL_DIR)/irq_wrapper.c \
    $(LOCAL_DIR)/baremetal_cmsis.c

ifeq ($(CONFIG_STD_HEAP), y)
ifneq ($(SSDK_TOOLCHAIN), iar) # IAR don't need newlib heap
MODULE_SRCS += \
    $(LOCAL_DIR)/Heap/heap_newlib.c
endif
endif

include $(MKROOT)/module.mk
