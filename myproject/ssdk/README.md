# SSDK — SemiDrive Software Development Kit

SEC/SAF domain 实时固件开发套件，运行于 D9 系列 SoC 的 **Cortex-R5 实时核心**，提供驱动、中间件和板级支持代码。

## 目录结构

```
ssdk/
├── arch/                  # 架构相关代码（ARMv7-R 等）
├── boards/                # 板级配置
│   ├── bmu02_a00/        # BMU02 板卡
│   │   └── secure/       #   secure 核心代码
│   │       ├── board.c   #     板级初始化
│   │       ├── board.h
│   │       ├── configs/  #     外设配置（时钟、引脚、内存等）
│   │       └── projects/ #     工程配置
│   │           └── ref/  #       ref 工程模板（defconfig + main.c + variants/）
│   ├── bmu05_a00/        # BMU05 板卡（ref 模板 + variants/，xip/xip_b 为脚本层别名）
│   ├── bmu03_a00/
│   ├── emu01_a00/        # EMU01 板卡（已迁移归一化结构，同 bmu05）
│   ├── d9360_ref/        # SemiDrive 公板
│   └── ...                # 其他公板（d9_ref, e3_*, d3_*, x9* 等）
├── drivers/               # 硬件驱动
│   ├── can.c             #   CAN 总线
│   ├── dma.c             #   DMA 控制器
│   ├── i2c.c             #   I2C
│   ├── spi.c             #   SPI
│   ├── uart.c            #   串口
│   ├── gpio.c            #   GPIO
│   └── kunlun/           #   D9 平台专用驱动（DMA、加密、显示等）
├── middleware/            # 中间件
│   ├── update_monitor/   #   rpmsg OTA 固件更新
│   ├── iccmsg/           #   核间通信
│   └── ...
├── lib/                   # 通用库
├── include/               # 公共头文件
├── examples/              # 示例代码
├── rtos/                  # RTOS（FreeRTOS）
├── scripts/               # 构建脚本
│   └── ota-bundle.sh     #   RAUC OTA 升级包打包脚本
├── tools/                 # 开发工具
│   ├── avbtool           #   Android Verified Boot 工具
│   ├── rauc.sh           #   RAUC 1.15.2 包装脚本
│   ├── bin/rauc          #   RAUC 1.15.2 二进制
│   ├── lib/              #   RAUC 运行时库
│   ├── sign_tool/        #   签名辅助脚本
│   └── sdtools/          #   SemiDrive 烧录工具
├── prebuilts/             # 预编译工具链
├── doc/                   # 文档
│   └── design/           #   设计文档
├── build.sh               # 主构建脚本
├── makefile               # 构建系统入口（委派给 mk/engine.mk）
├── AGENTS.md              # AI 辅助开发文档
└── README.md              # 本文件
```

## 支持的板卡

### BMU 系列

| 板卡 | 核心 | 工程 | 构建命令 |
|------|------|------|----------|
| `bmu02_a00` | `secure` | `ref` | `./build.sh -b bmu02_a00 -p ref` |
| `bmu03_a00` | `secure` | `ref` | `./build.sh -b bmu03_a00 -p ref` |
| `bmu05_a00` | `secure` | `ref`（模板；`xip`/`xip_b` 为脚本层别名） | `./build.sh -b bmu05_a00 -m emmc` / `-m xip` |
| `emu01_a00` | `secure` | `ref`（模板；`xip`/`xip_b` 为脚本层别名，已迁移归一化） | `./build.sh -b emu01_a00 -m emmc` / `-m xip` |

### SemiDrive 公板

执行 `./build.sh` 查看完整列表，包括 e3_*、d3_*、d9360_ref、d9_ref、x9* 等系列。

## 构建方法

### 编译固件

```bash
# 查看所有支持的板卡和工程
./build.sh

# 编译 BMU05 ref 工程
./build.sh -b bmu05_a00 -p ref

# 编译并清除旧文件
./build.sh -b bmu05_a00 -p ref -clear

# 使用 IAR 工具链
./build.sh -b bmu05_a00 -p ref -iar

# 指定核心（BMU 默认 secure）
./build.sh -b bmu05_a00 -p ref -c secure

# 启动模式构建（-m 为启动模式语义轴，推荐入口）
./build.sh -b bmu05_a00 -m emmc          # OCRAM 单镜像 + legacy bundle（等价 -p ref）
./build.sh -b bmu05_a00 -m xip           # NOR A/B 双槽一次构建 + per-slot bundle
./build.sh -b bmu05_a00 --boot-mode=xip  # 等价长选项形式
```

### 构建输出

编译产物位于 `boards/<board>/build/<project>/gcc/<core>/`：

```
boards/bmu05_a00/build/ref/gcc/secure/
├── secure.bin          # 原始固件二进制（~440KB）
├── secure.elf          # ELF 调试镜像
├── secure.elf.map      # 内存映射表
├── secure.elf.lst      # 反汇编列表
└── secure.elf.size     # 内存占用统计
```

`-m xip` 产物分槽位于 `build/xip/` 与 `build/xip_b/`；启动模式变体 defconfig 生成到 `out/variant_defconfig/<board>/<slot>/`（MEM_BASE 由 `scripts/calc_xip_membase.sh` 从 ospi2.bpt 派生）。

### 构建配置

每个工程使用 Kconfig 管理构建选项，配置文件位于：

```
boards/<board>/<core>/projects/<project>/
├── defconfig     # 默认配置
├── config.h      # 生成的配置头文件
└── main.c        # 工程入口
```

修改配置：编辑 `defconfig` 文件后重新构建即可生效。

### RAUC OTA 升级包

编译固件后自动触发 OTA 升级包打包（auto-OTA）：

| 构建方式 | 产物目录 | bundle 形态 |
|----------|----------|-------------|
| `-m emmc` | `out/<board>/emmc/ota/` | legacy 单镜像 bundle |
| `-m xip` | `out/<board>/xip/ota/` | per-slot bundle（内含 `ssystem_a.img` + `ssystem_b.img` 双槽固件） |
| legacy `-p` | `$SSDK_OUTPUT_DIR/ota/`（superproject 在场时默认 `out/<MACHINE_DEPLOYDIR>/ota/`） | 单镜像 bundle |

```
out/<board>/<mode>/ota/
├── ota-ssystem-<version>.raucb       # RAUC 升级包
└── history/ssdk-<version>.json       # 打包记录（含 boot_mode 字段）
```

auto-OTA 语义：

- `-m` 独立构建始终打包，忽略外部 `SSDK_NO_AUTO_OTA=1`（环境作用域隔离）
- 顶层委托构建以 `SSDK_DELEGATED_BUILD=1` 抑制自动打包（顶层统一打一次）
- legacy `-p` 构建沿用旧行为：`SSDK_NO_AUTO_OTA=1` 可跳过
- `SSDK_SKIP_BUNDLE=1` 跳过 bundle 阶段（仅编译固件）
- xip 模式要求双槽固件具备 A/B 槽切换能力（`CONFIG_GPT_SLOT_SWITCH` 或 `CONFIG_SYSMON_BOOT_MEDIA_NOR_XIP`）

也可单独执行打包：

```bash
./scripts/ota-bundle.sh                            # 模式从 SSDK_PROJECT 自动推导
SSDK_BOOT_MODE=xip ./scripts/ota-bundle.sh        # 显式打 per-slot bundle
```

> OTA 打包需要连接远程签名服务器，默认使用 `http://192.168.34.207:8900`。

### 独立构建（自包含）

ssdk 可脱离 d9_sdk 主仓独立构建，不依赖主仓任何脚本实现：

- **vendored 工具**：`bpttool`、`avbtool`、`remote_sign_helper` 已内置于 `tools/`，与主仓副本的字节一致性由 `scripts/check_vendored_tools.sh` 断言
- **BPT 分区表**：硬件真相以主仓 `cfg_industry/<board>/pack_cfg/` 为权威，已 vendor 副本至 `boards/<board>/secure/pack_cfg/`（路径解析 env > cfg_industry > ssdk 副本；独立 clone 无 superproject 时用副本）
- **版本号**：主仓 `scripts/auto_version.sh` 不可达时回退 build.sh 内逐字等价的四态内联实现
- **远程签名服务器**：仍是外部服务（`REMOTE_SIGN_URL`，默认 `http://192.168.34.207:8900`）
- **容器**：可选增强。宿主机直接以 `-m` 调用会经主仓 `scripts/d9run.sh`（`--network host` + 属主对齐）自动重入构建容器，`SSDK_NO_DOCKER=1` 可跳过，`SSDK_D9RUN` 可指向外部副本

> **⚠️ 配置同步**：`boards/<board>/secure/pack_cfg/*.bpt` 和 `secure/configs/image_cfg.h` 是主库 cfg_industry 的副本。
> 独立构建前请确认副本为最新（主库修改分区/内存布局后须同步）。详见 AGENTS.md 跨仓库配置同步。

### 烧录包（PAC）

PAC 包生成仅适用于 **taishan 平台**（e3_*、d3_* 系列，需完整 SDK 环境）：

```bash
./build.sh -b e3_evb -p ref -pac
```

> 注意：本仓库仅包含 BMU 系列板卡，taishan 平台板卡代码需在完整 SDK 中使用。
BMU 系列不支持 `-pac`，需通过外部 SDK 中的 `meta-semidrive/scripts/emmc_pac_separate.sh` 生成。

BMU 系列不支持 `-pac`，需使用外部 SDK 中的 `meta-semidrive/scripts/emmc_pac_separate.sh` 生成。

## 新增板卡

以添加 `myboard_a00` 为例。

### 步骤

**1. 创建 board 目录**

```bash
mkdir boards/myboard_a00
```

**2. 创建 Kconfig**

`boards/myboard_a00/Kconfig`：

```
config BOARD_MYBOARD_A00
    bool
    select HAS_AP1
    depends on ARCH_CHIP_D9
    default y

rsource "$(CORE)/Kconfig"
```

**3. 创建核心目录和配置文件**

```bash
mkdir -p boards/myboard_a00/secure/configs
mkdir -p boards/myboard_a00/secure/projects/ref

```

**4. 添加芯片配置**

`boards/myboard_a00/secure/configs/` 下存放引脚复用、时钟、内存映射等配置头文件。可参考现有板卡（如 `bmu05_a00/secure/configs/`）复制修改。

**5. 添加工程配置**

`boards/myboard_a00/secure/projects/ref/defconfig` — 内核配置选项。

`boards/myboard_a00/secure/projects/ref/main.c` — 工程入口函数。

**6. 注册到 build.sh**

编辑 `build.sh`，在 `BOARDS` 数组中添加 `myboard_a00`，在 `PROJECTS` 中注册对应工程：

```bash
BOARDS=(\
    ...
    "myboard_a00" \
)

PROJECTS=(\
    ...
    "myboard_a00/ref"
    ...
)
```

**7. 构建验证**

```bash
./build.sh -b myboard_a00 -p ref
```

### 最小文件清单

```
boards/myboard_a00/
├── Kconfig
└── secure/
    ├── board.c
    ├── board.h
    ├── Kconfig
    ├── rules.mk
    ├── configs/
    │   ├── clock_cfg.c/h
    │   ├── uart_cfg.c/h
    │   └── memmap.h
    ├── scripts/
    │   └── gcc/
    │       └── ddr.ld
    └── projects/
        └── ref/
            ├── defconfig
            ├── main.c
            └── project.mk
```

## 开发注意事项

- 固件运行在 **SRAM（IRAM）**，地址 `0x140000~0x200000`，共 768KB，不可执行 DDR 代码
- 组件初始化通过 `register_component()` 机制
- 日志使用 `SSDK_PRINT` 宏
- 内存地址尽量通过 config 或 device tree 定义，避免硬编码
- 不要修改 `prebuilts/` 中的供应商预编译文件

---

## SEC 固件升级指南（应用层开发者）

### 构建固件

```bash
cd source/ssdk

# emmc 模式（OCRAM 单镜像）
./build.sh -b bmu05_a00 -m emmc -c secure

# XIP 模式（NOR A/B 双槽）
./build.sh -b bmu05_a00 -m xip -c secure
```

产物位于 `source/ssdk/out/<board>/<mode>/ota/ota-ssystem-*.raucb`。

### 安装升级

```bash
# 上传 bundle 到板子
scp source/ssdk/out/<board>/<mode>/ota/ota-ssystem-*.raucb root@<board_ip>:/tmp/

# SSH 到板子执行
date -s "YYYY-MM-DD HH:MM:SS"    # 设置时间（避免证书 not yet valid）
rauc install /tmp/ota-ssystem-*.raucb
reboot
```

### XIP vs emmc 模式区别

| | emmc | xip |
|-|------|-----|
| R 核执行位置 | OCRAM（FDA 从 eMMC 加载） | NOR Flash 原地执行 |
| 升级机制 | RAUC 直写 eMMC 分区 | RAUC → hook → ota-client → R 核写 NOR |
| bundle 形态 | 单镜像（ssystem.img + vbmeta） | 4 镜像 per-slot（ssystem_a/b + vbmeta_a/b） |
| 防回滚 | GPT 属性翻转 | NOR GPT + retry counter |

### 验证升级

```bash
# 检查当前活动槽
cat /data/d9-state    # SSDK_ACTIVE_SLOT=A 或 B

# 检查 R 核固件版本
cat /etc/os-release | grep SSDK_VERSION
```

R 核串口日志确认（正常启动应看到）：
```
[sys_monitor] MARK_GOOD: slot X retry reset (GPT)
```

### 常见问题

| 问题 | 原因 | 解决 |
|------|------|------|
| `certificate is not yet valid` | 板子时间不对 | `date -s "YYYY-MM-DD HH:MM:SS"` |
| `ota-client not found` | rootfs 缺 app_sysmgr | 重编 buildroot（`BR2_PACKAGE_APP_SYSMGR=y`）|
| OTA 后启动失败 | 新固件异常 | 防回滚自动切回旧槽，检查 R 核串口日志 |
| bundle 不兼容 | RAUC compatible 不匹配 | 确认板子与 bundle 的 compatible 一致 |
