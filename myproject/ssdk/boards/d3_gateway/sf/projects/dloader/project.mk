# gcc/rapidstudio specialized scripts
ifneq ($(SSDK_TOOLCHAIN),iar)
GLOBAL_LDFLAGS += \
    --defsym MEMBASE=$(CONFIG_MEM_BASE) \
    --defsym MEMSIZE=$(CONFIG_MEM_SIZE) \
    --defsym TCMBASE=$(CONFIG_ARMV7R_TCMB_BASE) \
    --defsym TCMSIZE=0x20000 \
    --defsym NONEECCBASE=$(CONFIG_IRAM_NONE_ECC_BASE)
ifeq ($(CONFIG_IRAM_NONE_ECC),y)
GLOBAL_LDFLAGS += \
    --defsym NONEECCSIZE=$(CONFIG_IRAM_NONE_ECC_SIZE)
else
GLOBAL_LDFLAGS += \
    --defsym NONEECCSIZE=0
endif
LINKER_SCRIPT :=$(LOCAL_DIR)/scripts/gcc/ram.ld
endif

# iar specialized scripts
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_LINKER_OPTIONS += \
    --config_def MEMBASE=$(CONFIG_MEM_BASE) \
    --config_def MEMSIZE=$(CONFIG_MEM_SIZE) \
    --config_def TCMBASE=$(CONFIG_ARMV7R_TCMB_BASE) \
    --config_def TCMSIZE=0x20000 \
    --config_def NONEECCBASE=$(CONFIG_IRAM_NONE_ECC_BASE)
ifeq ($(CONFIG_IRAM_NONE_ECC),y)
IAR_LINKER_OPTIONS += \
    --config_def NONEECCSIZE=$(CONFIG_IRAM_NONE_ECC_SIZE)
else
IAR_LINKER_OPTIONS += \
    --config_def NONEECCSIZE=0
endif
IAR_LINKCFG := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ram.icf
IAR_DEBUG_MACRO := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ram.mac
endif
