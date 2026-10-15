LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_INCLUDES += \
    $(LOCAL_DIR)

ifneq ($(wildcard $(COREDIR)/libFuncSafetyPkg.a),)
GLOBAL_LDFLAGS += \
	-L$(COREDIR) -lFuncSafetyPkg
endif

ifeq ($(SSDK_TOOLCHAIN), rapidstudio)
SSDK_TOOLCHAIN_LOWER := gcc
else
SSDK_TOOLCHAIN_LOWER := ${SSDK_TOOLCHAIN}
endif

MODULE_SRCS += \
    $(LOCAL_DIR)/arm_arch.c \
    $(LOCAL_DIR)/arm_irq.c \
    $(LOCAL_DIR)/arm_fpu.c \
    $(LOCAL_DIR)/arm_mpu.c \
    $(LOCAL_DIR)/arm_faults.c \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_LOWER)/arm_atomic.S \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_LOWER)/arm_start.S \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_LOWER)/arm_cache.S \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_LOWER)/arm_tcm.S \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_LOWER)/arm_exceptions.S \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_LOWER)/addr_access.S \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_LOWER)/arm_saveusercontext.S \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_LOWER)/arm_fullcontextrestore.S \
    $(LOCAL_DIR)/$(SSDK_TOOLCHAIN_LOWER)/arm_switchcontext.S

include $(MKROOT)/module.mk
