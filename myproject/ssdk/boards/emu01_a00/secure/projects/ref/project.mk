# emu01_a00 secure 模板 project.mk (ssdk-standalone-boot-mode Stage-3)
#
# 链接脚本按变体选择 (由变体 defconfig 的 CONFIG_XIP_MODE 驱动):
#   CONFIG_XIP_MODE=y  -> xip.ld   (NOR XIP: + TCM/RAM/NONEECC defsym)
#   CONFIG_XIP_MODE 未定义 -> ddr.ld (OCRAM/emmc_only: 仅 MEMBASE/MEMSIZE)
#
# 注: CONFIG_RAM_BASE/RAM_SIZE/IRAM_NONE_ECC_BASE/SIZE 已在
#     boards/emu01_a00/secure/Kconfig 定义 (由 variants/xip.conf 显式消歧为 board 值)

# === 变体分支: XIP (xip.ld) ===
ifeq ($(CONFIG_XIP_MODE),y)
ifneq ($(SSDK_TOOLCHAIN),iar)
GLOBAL_LDFLAGS += \
    --defsym MEMBASE=$(CONFIG_MEM_BASE) \
    --defsym MEMSIZE=$(CONFIG_MEM_SIZE) \
    --defsym TCMBASE=$(CONFIG_ARMV7R_TCMB_BASE) \
    --defsym TCMSIZE=0x20000 \
    --defsym RAM_BASE=$(CONFIG_RAM_BASE) \
    --defsym RAM_SIZE=$(CONFIG_RAM_SIZE) \
    --defsym NONEECCBASE=$(CONFIG_IRAM_NONE_ECC_BASE)
ifeq ($(CONFIG_IRAM_NONE_ECC),y)
GLOBAL_LDFLAGS += \
    --defsym NONEECCSIZE=$(CONFIG_IRAM_NONE_ECC_SIZE)
else
GLOBAL_LDFLAGS += \
    --defsym NONEECCSIZE=0
endif

LINKER_SCRIPT :=$(LOCAL_DIR)/scripts/gcc/xip.ld
endif

# iar (XIP)
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_LINKER_OPTIONS += \
    --config_def MEMBASE=$(CONFIG_MEM_BASE) \
    --config_def MEMSIZE=$(CONFIG_MEM_SIZE) \
    --config_def TCMBASE=$(CONFIG_ARMV7R_TCMB_BASE) \
    --config_def TCMSIZE=0x20000 \
    --config_def RAM_BASE=$(CONFIG_RAM_BASE) \
    --config_def RAM_SIZE=$(CONFIG_RAM_SIZE) \
    --config_def NONEECCBASE=$(CONFIG_IRAM_NONE_ECC_BASE)
ifeq ($(CONFIG_IRAM_NONE_ECC),y)
IAR_LINKER_OPTIONS += \
    --config_def NONEECCSIZE=$(CONFIG_IRAM_NONE_ECC_SIZE)
else
IAR_LINKER_OPTIONS += \
    --config_def NONEECCSIZE=0
endif
IAR_LINKCFG := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/xip.icf
IAR_DEBUG_MACRO := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/xip.mac
endif

# === 变体分支: emmc_only (ddr.ld) ===
else
ifneq ($(SSDK_TOOLCHAIN),iar)
GLOBAL_LDFLAGS += \
    --defsym MEMBASE=$(CONFIG_MEM_BASE) \
    --defsym MEMSIZE=$(CONFIG_MEM_SIZE)
LINKER_SCRIPT += $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.ld
endif

# iar (emmc_only)
ifeq ($(SSDK_TOOLCHAIN),iar)
IAR_LINKER_OPTIONS += \
    --config_def MEMBASE=$(CONFIG_MEM_BASE) \
    --config_def MEMSIZE=$(CONFIG_MEM_SIZE)
IAR_LINKCFG := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.icf
IAR_DEBUG_MACRO := $(LOCAL_DIR)/scripts/$(SSDK_TOOLCHAIN)/ddr.mac
endif
endif
