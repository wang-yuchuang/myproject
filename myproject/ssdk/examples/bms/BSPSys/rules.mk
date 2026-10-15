LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/Device \
	$(LOCAL_DIR)/Util \
	$(LOCAL_DIR)/BSPUser \
	$(LOCAL_DIR)/BSPCfg \
	$(SSDK_ROOT)/drivers/kunlun/watchdog \
	
MODULE_SRCS += \
	$(LOCAL_DIR)/Device/DEVGPIO.c \
	$(LOCAL_DIR)/Device/CD74HC595.c \
	$(LOCAL_DIR)/Device/SN74HC165.c \
	$(LOCAL_DIR)/Device/DEVADC.c  \
	$(LOCAL_DIR)/Device/GD25Q32.c  \
	$(LOCAL_DIR)/Device/M2514.c    \
	$(LOCAL_DIR)/Device/DEVCAN.c    \
	$(LOCAL_DIR)/Device/DEVUART.c    \
	$(LOCAL_DIR)/Device/DEVTIMER.c    \
	$(LOCAL_DIR)/Device/DEVWDT.c    \
	$(LOCAL_DIR)/BSPUser/BSPUserHook.c \
	$(LOCAL_DIR)/BSPUser/BSPUserAPI.c \
	$(LOCAL_DIR)/BSPCfg/BSPConfig.c \
	

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-pointer-cast -Wno-unused-but-set-variable

include $(MKROOT)/module.mk
