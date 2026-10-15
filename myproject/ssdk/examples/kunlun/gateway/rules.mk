LOCAL_DIR := $(GET_LOCAL_DIR)

ifeq ($(CONFIG_SDPE_CTRL), y)
MODULES += $(LOCAL_DIR)/sdpe_ctrl
endif

ifeq ($(CONFIG_SDPE_MGR), y)
MODULES += $(LOCAL_DIR)/sdpe_mgr
endif

ifeq ($(CONFIG_SIG_ROUTE), y)
MODULES += $(LOCAL_DIR)/sig_route
endif

ifeq ($(CONFIG_LIN_SCHED), y)
MODULES += $(LOCAL_DIR)/lin_sched
endif
