LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/inc \

MODULE_SRCS += \
	$(LOCAL_DIR)/src/sys_monitor.c \
	$(LOCAL_DIR)/src/sys_monitor_msg.c \
	$(LOCAL_DIR)/src/sys_monitor_channel.c \
	$(LOCAL_DIR)/src/sys_monitor_task.c

# sys_monitor_ota.c = XIP NOR GPT OTA 模块 (staging/STIG/切槽/mark_good) — NOR XIP 专属。
# emmc_only (ref): R 核固件由 FDA 载 OCRAM, 升级由 A 核 RAUC 直写 eMMC 分区,
# R 核无 NOR GPT/OTA 职责 — 不编译此模块 (消除 XIP 污染 + 减 ref 固件体积)。
ifeq ($(CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP),y)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/sys_monitor_ota.c
# C 层宏经 -D 传递 (config.h 为 gitignore 本地文件, 无法随代码提交; Kconfig 值经
# make 层可靠注入 sys_monitor.c/msg.c 的 #ifdef CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP)
MODULE_COMPILEFLAGS += -DCONFIG_SYSMON_BOOT_MEDIA_NOR_XIP
endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-unused-but-set-variable

include $(MKROOT)/module.mk
