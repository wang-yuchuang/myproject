LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(SSDK_TOOLCHAIN), rapidstudio)
SSDK_TOOLCHAIN_UPPER := GCC
else
SSDK_TOOLCHAIN_UPPER := $(shell echo ${SSDK_TOOLCHAIN} | tr a-z A-Z)
endif

MODULE_INCLUDES += \
    $(LOCAL_DIR)/Config \
    $(LOCAL_DIR)/Source/include

ifeq ($(CONFIG_ARCH_CORTEXR5),y)
MODULE_INCLUDES += \
    $(LOCAL_DIR)/Source/portable/$(SSDK_TOOLCHAIN_UPPER)/ARM_CR5
MODULE_SRCS += \
    $(LOCAL_DIR)/Source/portable/$(SSDK_TOOLCHAIN_UPPER)/ARM_CR5/portASM.S \
    $(LOCAL_DIR)/Source/portable/$(SSDK_TOOLCHAIN_UPPER)/ARM_CR5/port.c
endif

MODULE_SRCS += \
    $(LOCAL_DIR)/Source/portable/Common/irq_wrapper.c \
    $(LOCAL_DIR)/Source/portable/Common/timer_wrapper.c \
    $(LOCAL_DIR)/Source/portable/CMSIS/freertos_cmsis.c \
    $(LOCAL_DIR)/Source/croutine.c \
    $(LOCAL_DIR)/Source/event_groups.c \
    $(LOCAL_DIR)/Source/list.c \
    $(LOCAL_DIR)/Source/queue.c \
    $(LOCAL_DIR)/Source/stream_buffer.c \
    $(LOCAL_DIR)/Source/tasks.c \
    $(LOCAL_DIR)/Source/timers.c

ifeq ($(CONFIG_STD_HEAP), y)
ifeq ($(SSDK_TOOLCHAIN),iar)
MODULE_SRCS += \
    $(LOCAL_DIR)/Source/portable/MemMang/heap_iar.c
else
MODULE_SRCS += \
    $(LOCAL_DIR)/Source/portable/MemMang/heap_newlib.c
endif
else
MODULE_SRCS += \
    $(LOCAL_DIR)/Source/portable/MemMang/heap_4.c \
	$(LOCAL_DIR)/Source/portable/MemMang/heap_ext.c
endif

include $(MKROOT)/module.mk
