LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS := $(LOCAL_DIR)/power_monitor.c

ifeq ($(BOARD), e3_evb)
MODULE_SRCS += $(LOCAL_DIR)/power_monitor_e3_evb.c
else ifeq ($(BOARD), e3_disp)
MODULE_SRCS += $(LOCAL_DIR)/power_monitor_e3_disp.c
else ifeq ($(BOARD), d3_disp)
MODULE_SRCS += $(LOCAL_DIR)/power_monitor_d3_disp.c
else ifeq ($(BOARD), e3_gateway)
MODULE_SRCS += $(LOCAL_DIR)/power_monitor_e3_gateway.c
else ifeq ($(BOARD), d3_gateway)
MODULE_SRCS += $(LOCAL_DIR)/power_monitor_d3_gateway.c
endif

include $(MKROOT)/module.mk
