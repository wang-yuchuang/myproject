LOCAL_DIR := $(GET_LOCAL_DIR)


GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include \
	$(LOCAL_DIR)/adapter

ifneq ($(wildcard $(LOCAL_DIR)/adapter/Mcu_Soc.c),)

ifeq ($(wildcard $(COREDIR)/libFuncSafetyPkg.a),)
MODULE_STATIC_LIB := true
endif

MODULE := $(LOCAL_DIR)
MODULE_SRCS += \
	$(LOCAL_DIR)/adapter/Mcu_Arch.S \
	$(LOCAL_DIR)/adapter/Mcu_Cfg.c \
	$(LOCAL_DIR)/adapter/Mcu_Ckgen.c \
	$(LOCAL_DIR)/adapter/Mcu_Delay.c \
	$(LOCAL_DIR)/adapter/Mcu_Delay_asm.S \
	$(LOCAL_DIR)/adapter/Mcu_Modules.c \
	$(LOCAL_DIR)/adapter/Mcu_Rstgen.c \
	$(LOCAL_DIR)/adapter/Mcu_Scr.c \
	$(LOCAL_DIR)/adapter/Mcu_Soc.c \
	$(LOCAL_DIR)/adapter/Mcu_Pll.c \
	$(LOCAL_DIR)/adapter/Mcu_Xtal.c \
	$(LOCAL_DIR)/src/Eic_Ip.c \
	$(LOCAL_DIR)/src/FuncSafetyPkg_Asm.S \
	$(LOCAL_DIR)/src/FuncSafetyPkg_Irq.c \
	$(LOCAL_DIR)/src/Fuse_Ip.c \
	$(LOCAL_DIR)/src/Iram_Ip.c \
	$(LOCAL_DIR)/src/Sem_Ip.c \
	$(LOCAL_DIR)/src/DDR_ecc.c

ifeq ($(CONFIG_FUSALIB_DDR), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/fusalib_ddr/fusalib_ddr.c \
	$(LOCAL_DIR)/fusalib_ddr/fusalib_ddr_local_test.c
endif
else
GLOBAL_COMPILEFLAGS += -DLIBFuncSafetyPkg_VERSION
endif

MODULE := $(LOCAL_DIR)
MODULE_SRCS += \
	$(LOCAL_DIR)/src/FuncSafetyPkg.c \
	$(LOCAL_DIR)/adapter/FuncSafetyPkg_Cfg.c

ifeq ($(CONFIG_FUSALIB_DDR), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/fusalib_ddr/fusalib_ddr_local_test.c
endif

include $(MKROOT)/module.mk
