# gcc specialized scripts
ifeq ($(SSDK_TOOLCHAIN),gcc)
GLOBAL_LDFLAGS += \
    --defsym MEMBASE=$(CONFIG_MEM_BASE) \
    --defsym MEMSIZE=$(CONFIG_MEM_SIZE) \
    --defsym TCMA_BASE=$(CONFIG_ARMV7R_TCMA_BASE) \
    --defsym TCMB_BASE=$(CONFIG_ARMV7R_TCMB_BASE) \
    --defsym RAM_BASE=$(CONFIG_RAM_BASE) \
    --defsym RAM_SIZE=$(CONFIG_RAM_SIZE)
LINKER_SCRIPT += $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/xip.ld
endif

# iar specialized scripts
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_LINKER_OPTIONS += \
    --config_def MEMBASE=$(CONFIG_MEM_BASE) \
    --config_def MEMSIZE=$(CONFIG_MEM_SIZE)
IAR_LINKCFG := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.icf
IAR_DEBUG_MACRO := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.mac
endif