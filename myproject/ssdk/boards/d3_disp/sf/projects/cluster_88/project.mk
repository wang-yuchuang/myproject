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
ifeq ($(CONFIG_HEAP_ON_SDRAM),y)
GLOBAL_LDFLAGS += \
    --defsym SDRAMBASE=$(CONFIG_SDRAM_BASE) \
    --defsym SDRAMSIZE=$(CONFIG_SDRAM_SIZE)
LINKER_SCRIPT :=$(LOCAL_DIR)/scripts/gcc/sdram.ld
else
LINKER_SCRIPT :=$(LOCAL_DIR)/scripts/gcc/ram.ld
endif
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
ifeq ($(CONFIG_HEAP_ON_SDRAM),y)
IAR_LINKER_OPTIONS += \
    --config_def SDRAMBASE=$(CONFIG_SDRAM_BASE) \
    --config_def SDRAMSIZE=$(CONFIG_SDRAM_SIZE)
IAR_LINKCFG := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/sdram.icf
else
IAR_LINKCFG := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ram.icf
endif
IAR_DEBUG_MACRO := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ram.mac
endif
