LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_POWER_MONITOR), y)
MODULES += $(LOCAL_DIR)/power_monitor
endif

ifeq ($(CONFIG_POWER_CLOCK_GATE), y)
MODULES += $(LOCAL_DIR)/gating
endif

ifeq ($(CONFIG_MEM_SECTION_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/mem_section
endif

ifeq ($(CONFIG_PVT_EXAMPLE), y)
MODULES += $(LOCAL_DIR)/pvt
endif
