# gcc/rapidstudio specialized scripts
ifneq ($(SSDK_TOOLCHAIN),iar)
GLOBAL_LDFLAGS += \
    --defsym MEMBASE=$(CONFIG_MEM_BASE) \
    --defsym MEMSIZE=$(CONFIG_MEM_SIZE) \
    --defsym TCMA_BASE=$(CONFIG_ARMV7R_TCMA_BASE) \
    --defsym TCMB_BASE=$(CONFIG_ARMV7R_TCMB_BASE)
LINKER_SCRIPT += $(LOCAL_DIR)/scripts/gcc/ddr.ld
endif

# iar specialized scripts
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_LINKER_OPTIONS += \
    --config_def MEMBASE=$(CONFIG_MEM_BASE) \
    --config_def MEMSIZE=$(CONFIG_MEM_SIZE)
IAR_LINKCFG := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.icf
IAR_DEBUG_MACRO := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.mac
endif
