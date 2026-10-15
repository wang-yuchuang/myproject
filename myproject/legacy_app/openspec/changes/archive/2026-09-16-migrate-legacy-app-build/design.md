## Context

legacy_app 是 Solaris Backup Management System 的 app 层业务程序，基于 SemiDrive D9 平台。当前构建流程：

```
mkenv_arm_st (环境变量) → mk_define (编译/安装/打包) → Makefile 编译 → make install → do_pack()
```

`do_pack()` 使用 `dd + mkfs.ext4 -d` 创建原始 ext4 镜像，无任何完整性校验或签名机制。

D9 SDK 的 test_app 已建立完整的 app 镜像构建流水线（`source/test_app/build.sh`），支持：
- dm-verity hash tree 完整性校验
- D9APPMD1 metadata 协议（含 root_hash、salt、版本、anti-rollback）
- 远程签名服务器在线签名
- RAUC OTA bundle 打包

rootfs 侧的 `S91app_mount` 脚本已支持 D9APPMD1 格式的验签和 dm-verity 挂载。

改造的目标是将 legacy_app 的打包阶段替换为与 test_app 兼容的流水线，同时保留现有的源码编译体系不变。

## Goals / Non-Goals

**Goals:**
- 复用 legacy_app 现有 `src/` 下全部 C/C++ 模块的编译逻辑（Makefile + include.mk + mkenv_arm_st 环境）
- 替换打包阶段：从纯 ext4 → D9APPMD1 自包含镜像（ext4 + dm-verity + 签名 metadata）
- 集成在线签名：通过 `--sign-url`/`--sign-token` 将 metadata 发送到远程签名服务器
- 输出产物与 `S91app_mount` 完全兼容，可直接用于 `/dev/by-name/app_a` / `app_b`
- 支持 RAUC OTA bundle 生成（app-post-install 钩子兼容）
- 构建脚本支持 Docker 内执行（`build.sh` + Docker 参数）

**Non-Goals:**
- 不改动 `src/` 下的业务模块代码
- 不改动 rootfs 侧的 `S91app_mount`、trust anchor、RAUC 配置
- 不改动分区布局（继续使用 `/dev/by-name/app_a` / `app_b`）
- 不涉及 DDR App / SEC 域代码

## Decisions

### Decision 1: 构建入口 — 单一 build.sh 整合分散脚本

**现状问题**：当前构建流程分散在 4 个脚本中（`mkenv_arm_st` → `mk_define` → `Makefile` → `do_pack()`），属于偶然性复杂而非本质性复杂。本质拆分只有两层：**Shell 编排** vs **Make 编译**。

**方案**：采用**混合整合**策略——创建 `build.sh` 作为单一入口编排器，吸收 `mkenv_arm_st` 的环境变量和 `mk_define` 的参数解析逻辑，但保留 `Makefile` 的 40+ 子目录递归编译不变。

```
build.sh <clean|build|install|pack|all> [mu_type] [lcd_type] [docker]
  ├── 第1部分: 环境变量 (原 mkenv_arm_st 内联)
  ├── 第2部分: 参数解析 (原 mk_define 内联)
  ├── 第3部分: 调用 Makefile 编译 (保留)
  └── 第4部分: 新打包流水线 (替换 do_pack)
```

**废弃**：`mkenv_arm_st` 和 `mk_define` 在验证期后被删除。
**保留**：`Makefile`（40 子目录递归 make）、`include.mk`（改为 `?=` 支持环境覆盖）。

完整分析见 Oracle 建议（附录 A）。

### Decision 2: 在线签名 — 链式环境变量 + 默认配置

**方案**：复用 `source/test_app/scripts/sign_metadata.sh`。签名 URL 采用三层链式配置（与 test_app 一致）：
```
SIGN_URL="${SIGN_URL:-${REMOTE_SIGN_URL:-http://192.168.34.207:8900}/api/v1/sign}"
```
优先级：`--sign-url` 参数 > `$SIGN_URL` 环境变量 > `$REMOTE_SIGN_URL` 环境变量 > 默认值

可通过 `config/build.conf` 文件统一管理默认配置，环境变量覆盖配置文件。
在 Docker/CI 中设置 `REMOTE_SIGN_URL` 环境变量即可覆盖。

`--skip-sign` 参数可强制跳过签名（开发调试模式）。
KEY_ID 缺省值：`ota_signing`

> **复用策略**：使用符号链接指向 d9_sdk 中的脚本，在 `scripts/UPSTREAM.md` 中追踪。

### Decision 3: 分区大小与 ext4 block size 约束

**分区大小必须与硬件分区一致**（默认 256M）：
- S91app_mount 从 `blockdev --getsize64` 读取物理分区大小，从分区末尾 -4096 读取 metadata
- 如果 app.img 按数据量自动计算（如 87M），metadata 会在 87M 位置而非 256M 末尾，导致验签失败
- 默认值 256M 定义在 `config/build.conf` 的 `PARTITION_SIZE`，可通过 `--partition-size` 覆盖

```
PART_BYTES = 268435456  # 256M (硬件 app_a/app_b 分区大小)
OVERHEAD = PART_BYTES * 2% + 4096
EXT4_BYTES = PART_BYTES - OVERHEAD
```

**ext4 block size 硬性约束**：dm-verity 要求 block size 为 4096。
mke2fs 对 <512MB 分区的默认 block size 是 1024，会导致不兼容。
`create_ext4.sh` 必须使用 `-b 4096`；build.sh 增加 `dumpe2fs` 检查。

### Decision 7: 版本号自动生成

**方案**：采用 `git describe` 自动生成版本号（与 d9_sdk 的 `ota_generate_version()` 一致）：
```
git describe --tags --always → dev-da86c6e3-20260618
无 git 仓库/无 tag → dev-20260618 (日期回退)
```
可通过 `--version` 参数或 `APP_VERSION` 环境变量显式覆盖。
版本号写入 D9APPMD1 metadata 的 `app_version(32)` 字段。
镜像输出名带版本号：`pkg/app-dev-da86c6e3-20260618.img`。

### Decision 4: OTA bundle — 独立 ota-bundle.sh + 过渡期兼容

**方案**：创建 `scripts/ota-bundle.sh`，基于 `source/test_app/scripts/ota-bundle.sh` 适配，将 `app.img` 打包为 RAUC bundle。

支持远程签名替换（`rauc replace-signature` 两阶段模式），或本地签名（`--rauc-cert`/`--rauc-key`）。

**过渡期兼容**：`build.sh --legacy-rpkg` 额外生成旧格式 `app_nccu.rpkg`，供未升级 rootfs 的设备使用。

### Decision 5: 目录结构

```
legacy_app/
├── build.sh               # [新] 单一构建入口
├── scripts/
│   ├── create_ext4.sh     # [符号链接→d9_sdk] 或复制+UPSTREAM.md
│   ├── generate_verity.sh # [同上]
│   ├── sign_metadata.sh   # [同上]
│   ├── assemble_image.sh  # [同上]
│   ├── ota-bundle.sh      # [新] OTA bundle
│   └── UPSTREAM.md        # [新] 脚本版本追踪
├── config/
│   └── verity.conf        # [新] dm-verity 参数
├── src/                   # [不变] 现有源码
├── out/                   # [不变] 编译产物中间目录
├── pkg/                   # [新] app.img 输出

### Decision 6: build 脚本合并策略（Oracle 审核结论）

**完整审计结论**（来源：Oracle 咨询）：

| 组件 | 操作 | 原因 |
|------|------|------|
| `mkenv_arm_st` | **100% 吸收到 build.sh** | 纯环境变量+别名，无需单独文件 |
| `mk_define` | **100% 吸收到 build.sh** | 参数解析+make 调用，直接内联 |
| `mk_define do_pack()` | **废弃，不移植** | 分区算法有 bug，被新流水线替代 |
| `Makefile` (40子目录) | **保留不改** | 递归 make 无法用 Shell 替代 |
| `include.mk` | **保留，`=`→`?=`** | 支持 Docker 注入工具链路径 |

**过渡策略**：
- `mkenv_arm_st` 和 `mk_define` 在第一个发布周期保留（注释或移入 `legacy/`），供开发者回退
- grep 所有调用点（CI、其他脚本），删除前更新调用者
- 交互式别名（`mf`, `mkc3`, `mkr3`）在 README 中记录等效的新命令

**不建议**全面整合（全部用 Shell 重写 Makefile）——边际收益几乎为零，成本巨大。

└── remote_pkg/            # [新] OTA bundle 输出
```

```

## Risks / Trade-offs

| 风险 | 缓解措施 |
|------|---------|
| **签名服务器不可用**：构建时签名服务器宕机导致打包失败 | 支持 `--skip-sign` 跳过签名；生产环境要求签名服务器 SLA |
| **ext4 block size 不兼容**：mke2fs 对 <512MB 分区默认 block size=1024，与 dm-verity 4096 不兼容 | `create_ext4.sh` 强制 `-b 4096`；build.sh 增加 `dumpe2fs` 检查 |
| **partition-size 估算不足**：数据量+hashtree+metadata 超出分区大小 | 构建时检查，超限时报错退出 |
| **test_app 脚本不同步**：后续 test_app 脚本优化未同步 | 使用符号链接指向 d9_sdk；或按 `UPSTREAM.md` 记录手动同步 |
| **openssl rsautl 残留**：Makefile 第186行 RSA 加密在改造后继续执行，产生无效 root_key 文件 | build.sh 中编译阶段后清理 root_key；或条件跳过该步骤 |
| **编译环境残留**：`mkenv_arm_st` 环境变量影响打包阶段 | 打包阶段在子 shell 中执行，隔离环境变量 |
| **Docker 内少交叉编译器**：`d9-sdk-build:2.11` 可能未预装 `aarch64-linux-gnu-gcc` | 构建前检查 `aarch64-linux-gnu-gcc --version`，缺时报错 |
| **旧 .rpkg 格式兼容**：现场设备可能仍依赖旧格式升级 | `--legacy-rpkg` 参数额外生成旧格式，过渡期后废弃 |

1. **创建 scripts/ 目录**，以符号链接或复制方式引入 test_app 的构建脚本，创建 `UPSTREAM.md`
2. **创建 config/verity.conf**，配置 dm-verity 参数
3. **创建 build.sh**，实现双阶段构建入口，处理 `--mu-type`、`--skip-sign`、`--legacy-rpkg` 参数
4. **修改 Makefile 或 build.sh**：处理 openssl rsautl 加密步骤，条件执行或移除
5. **创建 scripts/ota-bundle.sh**，适配 RAUC bundle 打包
6. **更新 .gitignore**：添加 `pkg/`、`output/`、`*.img`、`*.raucb`
7. **验证**：在开发环境用 Docker 执行完整构建，检查输出 `app.img` 格式
8. **硬件验证**：将 `app.img` 烧录到 D9360 开发板，确认 app 正常挂载和运行

- OTA bundle 中的 `--compatible` 字符串应与 d9_sdk RAUC 配置一致，需确认具体值
- RAUC cert/key 路径：是否使用 d9_sdk 中的正式证书，还是为 legacy_app 单独生成？
- Docker 镜像 `d9-sdk-build:2.11` 是否预装 `aarch64-linux-gnu-gcc` 交叉编译器？需验证
