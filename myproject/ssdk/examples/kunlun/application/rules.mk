LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/utils \
	$(LOCAL_DIR)/arbitrator/inc

ifeq ($(CONFIG_APP_LAUNCHER), y)
MODULE_DEPS += $(LOCAL_DIR)/arbitrator
MODULE_DEPS += $(LOCAL_DIR)/launcher
endif

ifeq ($(CONFIG_APP_AUDIO), y)
MODULE_DEPS += $(LOCAL_DIR)/audio
endif

ifeq ($(CONFIG_AUDIO_HUB_SVC), y)
MODULE_DEPS += $(LOCAL_DIR)/ahub
endif

ifeq ($(CONFIG_APP_DEMO1), y)
MODULE_DEPS += $(LOCAL_DIR)/app_demo1
endif

ifeq ($(CONFIG_BOOT_ANIMATION), y)
MODULE_DEPS += $(LOCAL_DIR)/bootanimation
endif

ifeq ($(CONFIG_FASTAVM), y)
MODULE_DEPS += $(LOCAL_DIR)/fastavm
endif

ifeq ($(CONFIG_CLUSTER), y)
MODULE_DEPS += $(LOCAL_DIR)/cluster
endif

ifeq ($(CONFIG_CONTROL_PANEL), y)
MODULE_DEPS += $(LOCAL_DIR)/controlpanel
endif

ifeq ($(CONFIG_EMIRROR), y)
MODULE_DEPS += $(LOCAL_DIR)/emirror
endif

ifeq ($(CONFIG_PVT_APPLICATION), y)
MODULES += $(LOCAL_DIR)/pvt
endif

ifeq ($(CONFIG_CAM_PRE), y)
MODULES += $(LOCAL_DIR)/cam_pre
endif

ifeq ($(CONFIG_CAM_DEBUG), y)
MODULES += $(LOCAL_DIR)/cam_debug
endif

ifeq ($(CONFIG_PFM_ON), y)
MODULES += $(LOCAL_DIR)/pfmon
endif

ifeq ($(CONFIG_POWER_OFF), y)
MODULES += $(LOCAL_DIR)/poweroff
endif

ifeq ($(CONFIG_KERNEL_MONITOR), y)
MODULES += $(LOCAL_DIR)/kernel_monitor
endif

ifeq ($(CONFIG_SEM_MONITOR), y)
MODULES += $(LOCAL_DIR)/sem_monitor
endif

ifeq ($(CONFIG_SDLOG_COMMAND), y)
MODULES += $(LOCAL_DIR)/sdlog-tools
endif

ifeq ($(CONFIG_EXT_RST_REQ), y)
MODULES += $(LOCAL_DIR)/ext_rst_req
endif

ifeq ($(CONFIG_FuncSafetyPkg_TEST), y)
MODULES += $(LOCAL_DIR)/FuncSafetyPkg_test
endif

ifeq ($(CONFIG_ICCMSG_DEMO), y)
MODULES += $(LOCAL_DIR)/iccmsg_demo
endif

ifeq ($(CONFIG_SYS_MON), y)
MODULE_DEPS += $(LOCAL_DIR)/sys_mon
endif

ifeq ($(CONFIG_APPLICATION_SYS_MONITOR), y)
MODULES += $(LOCAL_DIR)/sys_monitor
endif

ifeq ($(CONFIG_GPT_SLOT_SWITCH), y)
MODULES += $(LOCAL_DIR)/gpt_slot_switch
endif

ifeq ($(CONFIG_FLASH_TEST), y)
MODULES += $(LOCAL_DIR)/flash_test
endif

ifeq ($(CONFIG_APP_COREMARK), y)
MODULES += $(LOCAL_DIR)/coremark
endif
