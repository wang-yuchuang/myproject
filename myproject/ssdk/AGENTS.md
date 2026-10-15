# source/ssdk/

SemiDrive Software Development Kit - SEC/SAF domain drivers, middleware, and board support code.

This SDK runs on **Cortex-R5 real-time cores** (SEC/SAF domains), separate from the Linux-running Cortex-A55 cores.


## D9 MULTI-DOMAIN ARCHITECTURE

| 域 | 核心 | OS | 源码 |
|----|------|----|----|
| AP | Cortex-A55 ×6 | Linux 5.10 | linux5.10/, u-boot/ |
| SEC/SAF | Cortex-R5 ×2 | RTOS | ssdk/, BareMetal_Suite/ |

## SUPPORTED BOARDS

本项目支持的 BMU 系列板卡：

| Board | Core | Project | Build Command |
|-------|------|---------|---------------|
| `bmu02_a00` | `secure` | `ref` | `-b bmu02_a00 -p ref` |
| `bmu03_a00` | `secure` | `ref` | `-b bmu03_a00 -p ref` |
| `bmu05_a00` | `secure` | `ref`（模板；`xip`/`xip_b` 为脚本层别名） | `-b bmu05_a00 -m emmc` / `-m xip` |
| `emu01_a00` | `secure` | `ref`（模板；`xip`/`xip_b` 为脚本层别名，已迁移归一化） | `-b emu01_a00 -m emmc` / `-m xip` |

> **bmu05 工程归一化（ssdk-project-mode-unification）**：`boards/bmu05_a00/secure/projects/` 仅保留 `ref/` 一份实体模板
> （defconfig 公共集 + variants/{emmc_only,xip}.conf + 归一 main.c/project.mk）。`-p xip`（XIP slot A）、
> `-p xip_b`（slot B）为构建脚本层别名——无物理目录；project_build 检测 `ref/variants/` 后以
> `PROJECT_SRC_DIR` 覆盖 make PRJDIR=ref 模板，变体 defconfig 生成到 `out/variant_defconfig/`
> （MEM_BASE 由 scripts/calc_xip_membase.sh --emit 从 ospi2.bpt 派生，改 NOR 分区自动适配）。
> 新增板参考：复制 bmu05 `secure/` 目录 → 按板裁剪 defconfig 公共集 + 配置 variants 值即可（另开变更）。

> 除 BMU 外还支持大量 SemiDrive 公板（e3_*, d3_*, d9360_ref, d9_ref, x9* 等），详见 `build.sh` 帮助。

## STRUCTURE

```
source/ssdk/
├── AGENTS.md              # This file
├── arch/                  # Architecture-specific code
├── drivers/               # Hardware drivers
├── middleware/           # Middleware services
├── boards/               # Board-specific code
│   ├── bmu05_a00/       # BMU05 board configs
│   ├── bmu02_a00/       # BMU02 board configs
│   └── ...
├── scripts/              # Build scripts
│   └── ota-bundle.sh    # RAUC OTA bundle builder
├── tools/                # Development tools
│   ├── sdtools/         # SemiDrive flashing tools
│   ├── avbtool          # Android Verified Boot tool (Python)
│   ├── rauc.sh          # RAUC binary wrapper
│   ├── install-rauc.sh  # RAUC build & install script
│   ├── bin/rauc         # RAUC 1.15.2 binary
│   ├── lib/             # RAUC shared libraries
│   └── genpac.py        # PAC package generator (taishan only)
├── prebuilts/            # Prebuilt binaries
├── include/              # Public headers
├── lib/                  # Libraries
├── examples/             # Example code
├── rtos/                 # RTOS components (if applicable)
├── doc/                  # Documentation
│   └── design/           # Design documents
└── makefile             # Build system veneer (delegates to mk/engine.mk)
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| Chip-agnostic drivers | `drivers/*.c` | CAN, DMA, I2C, SPI, UART, GPIO, etc. |
| D9 platform drivers | `drivers/kunlun/` | Platform-specific (DMA, crypto, display, etc.) |
| Board configs | `boards/<board>/` | Pinmux, clocks, memory map, etc. |
| Update monitor | `middleware/update_monitor/` | R5 OTA firmware update via rpmsg |
| RAUC bundle | `scripts/ota-bundle.sh` | OTA bundle builder |
| RAUC toolchain | `tools/{bin,lib}/rauc` | Bundled RAUC 1.15.2 |
| Flash tools | `tools/sdtools/` | SDTool / PAC generation |

## DEVELOPMENT WORKFLOW

### Building SEC Firmware

```bash
# Build BMU board firmware (e.g. bmu05_a00)
./build.sh -b bmu05_a00 -p ref

# Build for a specific core (secure is default for BMU)
./build.sh -b bmu05_a00 -p ref -c secure

# Build with IAR toolchain
./build.sh -b bmu05_a00 -p ref -iar

# Boot-mode build (-m is the boot-mode axis; recommended entry)
./build.sh -b bmu05_a00 -m emmc            # OCRAM single image + legacy bundle (≈ -p ref)
./build.sh -b bmu05_a00 -m xip             # NOR A/B dual-slot; both slots in one invocation + per-slot bundle
./build.sh -b emu01_a00 --boot-mode=xip    # equals-form; emu01_a00 is variant-migrated

# -m rules: conflicts with an inconsistent -p; only valid with -c secure;
# requires the board to have boards/<board>/secure/projects/ref/variants/

# Clean build
./build.sh -b bmu05_a00 -p ref -clear

# See all available board/project combinations
./build.sh

# Build output
#   boards/<board>/build/<project>/gcc/<core>/
#   ├── secure.bin          # Raw firmware binary
#   ├── secure.elf          # ELF debug image
#   └── secure.elf.map      # Memory map
```

> bmu05 归一化后 `-p xip`/`-p xip_b`/`-p ref` 均编译 `ref` 模板的不同变体（见上表注）；
> 产物目录仍按别名 `build/xip/`、`build/xip_b/`、`build/ref/` 分开。`-p xip_b` 的 MEM_BASE
> 由 calc 从 ospi2.bpt 派生 slot B 地址（0x082db000）。

### Building BareMetal Test Suite

```bash
./build/bm/build.sh
```

### PAC Flash Package (原烧录包)

PAC 包生成仅在 **taishan 平台**（e3_*, d3_* 系列）上受支持：

```bash
# Build + generate PAC package (taishan only)
./build.sh -b e3_evb -p ref -pac
```

**BMU 系列不支持 `-pac`** — 因 BMU 使用 `secure` core，不满足 build.sh 中的 taishan 平台判定。
PAC 烧录包需使用外部 SDK 项目中 `meta-semidrive/scripts/emmc_pac_separate.sh` 完成。

### RAUC OTA Bundle

```bash
# 前提：先编译固件
./build.sh -b bmu05_a00 -p ref

# 打包 RAUC bundle（需配置远程签名服务器）
export REMOTE_SIGN_TOKEN="your_token"
export SSDK_OUTPUT_DIR="./out"
./scripts/ota-bundle.sh

# 产物
#   <SSDK_OUTPUT_DIR>/ota/ota-ssystem-<version>.raucb
#   <SSDK_OUTPUT_DIR>/ota/history/ssdk-<version>.json
#
# -m xip 构建自动产出 per-slot bundle（写 <ssdk>/out/<board>/xip/ota/）：
#   bundle 内含 ssystem_a.img + ssystem_b.img 双槽固件
#   独立 -m 构建忽略 SSDK_NO_AUTO_OTA；顶层委托（SSDK_DELEGATED_BUILD=1）不自动打包
#   SSDK_SKIP_BUNDLE=1 仅编译不打包
```

**环境需求：**

| 依赖 | 说明 |
|------|------|
| `tools/{bin,lib}/rauc` | ✅ 已安装 1.15.2（系统 `rauc` 1.6 也可用） |
| 远程签名服务器 | ❌ 需要 `REMOTE_SIGN_URL` + `REMOTE_SIGN_TOKEN`（默认 `http://192.168.34.207:8900`） |
| `meta-semidrive` | ⚠️ 可选：superproject 在场时优先其 `.project.conf`/cfg_industry；standalone 经 env 代偿（见下方独立运行限制） |

**独立运行限制：** 脚本/工具实现零外部依赖：`bpttool`/`avbtool`/`remote_sign_helper_with_files` 已 vendor 至 `tools/`（byte-identity 由 `check_vendored_tools.sh` 断言），`calc_xip_membase.sh` 位于 `scripts/`，`auto_version.sh` 不可达时回退逐字等价内联逻辑。剩余外部前置：① 板级分区表（已 vendor 副本至 `boards/<board>/secure/pack_cfg/`；路径解析 env > cfg_industry > ssdk 副本，见下方跨仓库配置同步）② 可选板级配置（`RAUC_COMPATIBLE`/`SSDK_OUTPUT_DIR` env 可代）③ 远程签名服务器（仅 bundle 阶段，不可本地化）④ 可选容器运行时（`d9run.sh` 或自备 Ubuntu 18.04 容器，`SSDK_NO_DOCKER=1` 宿主直跑）。

### Firmware OTA Update

固件 OTA 由 AP (Linux) 侧通过 rpmsg 触发 R5 核心升级。Kconfig 配置：`UPDATE_FLASH` (OSPI NOR)、`UPDATE_EMMC`、`UPDATE_MONITOR_NO_UPDATE` (仅 WDT)。

### Memory Analysis

**SEC Domain Memory Architecture**: Boot ROM → Load from eMMC → SRAM → Execute (DDR not initialized)

| Region | Address | Size | Purpose |
|--------|---------|------|---------|
| SRAM (Code) | 0x140000~0x200000 | 768KB (FIXED) | .text, .data, .bss, Heap, Stack |
| DDR SEC | 0x30000000~0x30600000 | 6MB | rpmsg buffers, shared memory (NOT for code) |
**Key Facts**:
- SEC code runs in **SRAM (IRAM)**, not DDR
- IRAM = Internal SRAM (on-chip memory)
- CONFIG_MEM_SIZE is fixed (768KB, hardware limitation)
- DDR region cannot execute code (MPU XN flag)

**Memory Layout** (SRAM 768KB): .text (133KB) + .rodata (26KB) + .data (58KB) + .bss (51KB) + Heap (525KB auto) = 767KB ✅

**⭐ Heap Dynamic Calculation**: `Heap = _heap_end - _heap_start = SRAM_END - Static_Memory_End`. Auto-calculated by linker (no manual config). Reducing static memory → Heap increases automatically.

**⭐ Design Constraints**: Hardware: SRAM 768KB fixed (0x140000~0x200000), DDR MPU XN (no execution). Memory: Static fixed at compile, Heap = SRAM - static. Targets: SRAM usage <80%, Heap <70%.

**⭐ Quick Optimization (Save 144KB)**: Disable STR if not needed. Industrial control ✅, Battery ❌. Impact: only STR suspend lost.

**FreeRTOS Memory**: Tasks, queues, semaphores from Heap (~31KB typical). Available: 525KB Heap, budget: ~494KB for extensions.

**Design Guidelines**: Core security only (minimize), business logic → AP domain, memory <80%, heap ~27KB typical.
**Documentation**: [SEC Memory Design Guide](doc/design/SEC_MEMORY_DESIGN_GUIDE.md), [Quick Reference](doc/design/MEMORY_QUICK_REFERENCE.md)

### Key Differences from AP Development

| Aspect | AP Domain (Linux) | SEC/SAF Domain |
|--------|-------------------|-----------------|
| Core | Cortex-A55 | Cortex-R5 |
| OS | Linux | Baremetal/RTOS |
| Source | `linux5.10/`, `u-boot/` | `ssdk/`, `BareMetal_Suite/` |
| Build | Yocto / Buildroot | `./build.sh -b <board> -p <project>` |
| Flash | eMMC/SD | Separate partition |

## 跨仓库配置同步（必须遵守）

以下配置文件在 ssdk 和主库（cfg_industry）各存一份，**修改任何一份 MUST 同步另一份**：

| 配置 | 主库位置（真相源） | ssdk 副本 | 同步方式 |
|------|-------------------|----------|---------|
| BPT 分区表 | `cfg_industry/.../pack_cfg/{ospi2,global_emmc_only}.bpt` | `boards/<board>/secure/pack_cfg/` | 手动复制，两边同时提交 |
| image_cfg.h | `cfg_industry/.../image_cfg.h` | `boards/<board>/secure/configs/` | 顶层构建自动复制；独立构建时用手动同步 |
| project.conf | `cfg_industry/.../project.conf` | 不复制（构建时读取） | 自动 |

**⚠️ 违规后果**：
- BPT 不同步 → MEM_BASE 按陈旧偏移计算 → **砖板**
- image_cfg.h 不同步 → SAF_MEMBASE/SIZE 过期 → 内存越界
- `check_vendored_tools.sh` 会在主库构建时检测漂移并 WARN

**操作规程**：
1. 修改分区 → 先改主库 BPT → 立即复制到 ssdk pack_cfg/ → 两边同时提交
2. 修改内存布局 → 先改主库 image_cfg.h → 执行一次顶层 `./build.sh ssdk` 刷新 ssdk 副本 → 两边同时提交
3. 独立 clone 构建前 → 确认 ssdk 副本为主库最新版本

## CONVENTIONS

- **C-based component architecture**
- **Component initialization** via `register_component()`
- **Logging** via `SSDK_PRINT` macros
- **Board-specific** `#ifdef` guards
- **Memory addresses** from device tree or config

## COMMANDS

```bash
# 构建 SEC 固件 (BMU05)
./build.sh -b bmu05_a00 -p ref

# 裸机测试套件
./build/bm/build.sh

# 查看所有支持的 board/project
./build.sh

# RAUC OTA bundle
export SSDK_OUTPUT_DIR=./out
export REMOTE_SIGN_TOKEN="token"
./scripts/ota-bundle.sh
```

## ANTI-PATTERNS

- DO NOT modify hardware registers without datasheet
- DO NOT skip error checking in driver code
- DO NOT hardcode memory addresses - use device tree or config
- DO NOT mix AP and SEC/SAF domain code

## NOTES

- Contains many FIXME/TODO markers (work in progress)
- Vendor code in `prebuilts/` (do not modify)
- Some components use littlevgl for display GUI
- lwIP stack for networking
- Secure boot requires signed binaries
