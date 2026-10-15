LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_POWER_GATE_EXAMPLE_E3), y)
MODULE_SRCS += $(LOCAL_DIR)/power_gate_e3.c

else ifeq ($(CONFIG_POWER_GATE_EXAMPLE_D3), y)
MODULE_SRCS += $(LOCAL_DIR)/power_gate_d3.c
endif

ifeq ($(CONFIG_CLOCK_GATE_EXAMPLE_E3), y)
MODULE_SRCS += $(LOCAL_DIR)/clk_gate_e3.c

else ifeq ($(CONFIG_CLOCK_GATE_EXAMPLE_D3), y)
MODULE_SRCS += $(LOCAL_DIR)/clk_gate_d3.c
endif

include $(MKROOT)/module.mk
