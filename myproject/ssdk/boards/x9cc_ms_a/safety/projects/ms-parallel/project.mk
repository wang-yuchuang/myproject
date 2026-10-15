# gcc specialized scripts
ifeq ($(SSDK_TOOLCHAIN),gcc)
GLOBAL_LDFLAGS += \
    --defsym MEMBASE=$(CONFIG_MEM_BASE) \
    --defsym MEMSIZE=$(CONFIG_MEM_SIZE) \
    --defsym RESSIZE=$(CONFIG_RES_SIZE) \
    --defsym TCMA_BASE=$(CONFIG_ARMV7R_TCMA_BASE) \
    --defsym TCMB_BASE=$(CONFIG_ARMV7R_TCMB_BASE)
LINKER_SCRIPT += $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.ld

ifeq ($(CONFIG_MEM_BASE), 0x600000)
EXTRA_DDR_LINKERSCRIPT := $(PRJDIR)/extra_ddr.ld
EXTRA_DDR_LD := $(call TOBUILDDIR, $(EXTRA_DDR_LINKERSCRIPT))
EXTRA_LINKER_SCRIPTS += $(EXTRA_DDR_LD)
GENERATED += $(EXTRA_DDR_LD)

GLOBAL_COMPILEFLAGS += -DDDR_DATA=__attribute__\(\(section\(\".ddr_data\"\)\)\)

$(EXTRA_DDR_LD): $(EXTRA_DDR_LINKERSCRIPT) $(BUILDDIR)/configs/image_cfg.h
	@$(MKDIR)
	@echo generating $(EXTRA_DDR_LD)
	$(NOECHO)$(CC) -E -P -x assembler-with-cpp $(GLOBAL_INCLUDES) $< -o $@
	@sed -i "s/[uU];/;/g" $@
else
GLOBAL_COMPILEFLAGS += -DDDR_DATA=
endif
endif

# iar specialized scripts
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_LINKER_OPTIONS += \
    --config_def MEMBASE=$(CONFIG_MEM_BASE) \
    --config_def MEMSIZE=$(CONFIG_MEM_SIZE)
IAR_LINKCFG := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.icf
IAR_DEBUG_MACRO := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.mac
endif
