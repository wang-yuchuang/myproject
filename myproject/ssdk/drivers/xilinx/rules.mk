LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_XLI_TTC), y)
MODULES += $(LOCAL_DIR)/ttc
endif

ifeq ($(CONFIG_XLI_UART), y)
MODULES += $(LOCAL_DIR)/uart
endif

ifeq ($(CONFIG_ARM_GIC), y)
MODULES += $(LOCAL_DIR)/gic
endif