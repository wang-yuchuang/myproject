# iar specialized scripts
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_LINKER_OPTIONS += \
    --config_def MEMBASE=$(CONFIG_MEM_BASE) \
    --config_def MEMSIZE=$(CONFIG_MEM_SIZE) \
    --config_def TCMBASE=$(CONFIG_ARMV7R_TCMB_BASE) \
    --config_def TCMSIZE=0x20000 \
    --redirect osDelay=mdelay

ifeq ($(CONFIG_READ_BACK_BUFFER),y)
IAR_LINKER_OPTIONS += --config_def READ_BACK_BUFFER_SIZE=$(CONFIG_READ_BACK_BUFFER_SIZE)
else
IAR_LINKER_OPTIONS += --config_def READ_BACK_BUFFER_SIZE=0
endif

IAR_LINKCFG := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/flashloader.icf
IAR_DEBUG_MACRO := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/flashloader.mac
endif