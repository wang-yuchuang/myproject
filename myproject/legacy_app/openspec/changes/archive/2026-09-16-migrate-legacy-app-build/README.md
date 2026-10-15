### 开发任务说明

legacy_app 当前使用传统 ext4 镜像打包方式（`mkfs.ext4 -d out/`），缺乏 dm-verity 完整性校验和签名验签机制，无法与 D9 SDK rootfs 的 `S91app_mount` 验签挂载流程兼容。现 D9 SDK 已建立完整的在线签名基础设施（签名服务器 + trust anchor + D9APPMD1 metadata 协议），test_app 已具备成熟的构建流水线。

legacy_app 需改造构建方法以复用此基础设施，实现与 test_app 一致的构建输出格式和 OTA 升级能力，为后续正式升级迁移做准备。

相关参考案例：
- [OTA with dm-verity on Embedded Linux](https://www.rauc.io/) — RAUC 升级框架在嵌入式 Linux 中的 dm-verity 集成方案，本项目的 OTA bundle 基于 RAUC 实现
- [AVB (Android Verified Boot) dm-verity 实现](https://source.android.com/security/verifiedboot/dm-verity) — Android 的 dm-verity 方案与本项目使用的 `veritysetup` 命令行工具同源
- [dm-verity + signed metadata 安全启动链](https://www.kernel.org/doc/html/latest/admin-guide/device-mapper/verity.html) — Linux 内核 dm-verity 官方文档，本项目的 hashtree 格式和 hash 校验算法完全遵循此规范

### 变更信息

| 项 | 内容 |
|----|------|
| 涉及分支 | `dev` |
| 提交记录 | `da86c6e3` BMS配置参数等 — 构建迁移相关文件混入历史提交 |
| 变更文件 | `build.sh` (重构简化), `src/shells/run_app` (可写目录+日志路径), `src/shells/app_entry` (PID+SIGTERM), `src/shells/app_exit` (新), `src/shells/Makefile` (app_exit集成), `src/version_patch.h` (自动生成) |
| 删除文件 | `config/build.conf`, `src/shells/rsa_prublic_key`, `src/root_passwd` |
| 涉及子系统 | 构建系统, 应用启动流程 |
| 风险等级 | ⚡ 中 |
| 实施进度 | **41/41 项全部完成** ✅（含硬件验证） |
### 解决方案

**方案概述**：保留 legacy_app 现有的源码编译体系（`src/` 下 40+ 模块的 Makefile 编译链），仅替换打包阶段。创建顶层 `build.sh` 作为统一入口，内部双阶段执行：

1. **编译阶段**：调用现有 `source src/mkenv_arm_st && src/mk_define release mu3 && make install` 链，编译产物输出到 `out/`
2. **打包阶段**：以 `out/` 为输入，依次执行 `create_ext4 → generate_verity → sign_metadata → assemble_image` 流水线，最终输出 D9APPMD1 自包含镜像

**关键技术决策**：

| 决策 | 方案 | 原因 |
|------|------|------|
| 构建入口 | 顶层 `build.sh` 双阶段 | 不重写 40+ 子模块的 Makefile，降低风险 |
| 在线签名 | 复用 test_app 的 `sign_metadata.sh` | 已成熟使用，直接传入参数即可 |
| 分区大小 | 固定 256M（与硬件 app_a/app_b 分区一致，可从 `config/build.conf` 配置） | S91app_mount 从分区末尾读取 metadata，分区大小必须与硬件一致 |
| OTA bundle | 基于 test_app 的 `ota-bundle.sh` 适配 | 支持 RAUC 兼容的双槽位升级/回滚 |
| 签名URL | 三层链式配置：`--sign-url` > `$SIGN_URL` > `$REMOTE_SIGN_URL` > 默认 `http://192.168.34.207:8900/api/v1/sign` | 兼容本地开发和 CI/CD 流水线，无需硬编码 |
| 版本号 | `git describe` 自动生成（`dev-{sha}-{date}`），回退到当前日期 | 确保每次构建版本唯一可追溯 |
| 配置文件 | `config/build.conf` 管理签名URL、RAUC compatible 等默认值 | 避免重复输入常用参数，统一团队构建行为 |

**核心改造点**：
- 构建入口重构：`mkenv_arm_st → mk_define → make → do_pack()` → `build.sh` 统一入口
- 打包方式替换：纯 ext4 → D9APPMD1 自包含镜像（ext4 + dm-verity hashtree + 签名 metadata）
- 在线签名集成：废弃本地 RSA 密码加密，走远程签名服务器（`--sign-url`/`--sign-token`）
- 输出格式对齐：精确对齐分区大小，metadata 在镜像尾部 4096 字节位置
- OTA bundle 集成：基于 RAUC 格式的升级包生成

**脚本合并策略（Oracle 审核结论）**：

| 组件 | 操作 | 原因 |
|------|------|------|
| `mkenv_arm_st` | **100% 吸收到 build.sh** | 纯环境变量+别名，无需单独文件 |
| `mk_define` | **100% 吸收到 build.sh** | 参数解析+make 调用，直接内联 |
| `mk_define do_pack()` | **废弃，不移植** | 分区算法有 bug，被新流水线替代 |
| `Makefile` (40子目录) | **保留不改** | 递归 make 无法用 Shell 替代 |
| `include.mk` | **保留，`=`→`?=`** | 支持 Docker 注入工具链路径 |

**过渡策略**：`mkenv_arm_st` 和 `mk_define` 在第一个发布周期保留（移入 `legacy/`），供开发者回退。交互式别名（`mf`, `mkc3`, `mkr3`）在 README 中记录等效的新命令。

详细方案设计内容见 [design.md](design.md)。

### 开发任务设计

| 任务分组 | 说明 | 状态 | 验证方法 |
|---------|------|------|----------|
| 1. 构建脚本基础结构 (12项) | 创建 `scripts/`、`config/verity.conf`、`config/build.conf`、`build.sh`，吸收 `mkenv_arm_st`/`mk_define`，三层签名URL链式配置，git describe 自动版本号，清理 `openssl rsautl` | ✅ **12/12 完成** | `./build.sh --help` 输出完整参数说明；`mkenv_arm_st` 和 `mk_define` 不再作为独立入口 |
| 2. 编译阶段适配 (2项) | `build.sh` 编译阶段正确调用 `make install`，子 shell 隔离环境 | ✅ **2/2 完成** | 编译后 `out/` 包含 bin/config/lib/sampler/reporter 等子目录 |
| 3. 打包阶段适配 (8项) | 顺序执行 create_ext4 → generate_verity → sign_metadata → assemble_image，输出 `pkg/app.img` | ✅ **8/8 完成** | `pkg/app.img` 大小等于分区大小，metadata magic 为 `D9APPMD1` |
| 4. OTA Bundle 集成 (4项) | 创建 `scripts/ota-bundle.sh`，支持本地/远程签名，集成 `--ota`/`--legacy-rpkg` 参数 | ✅ **4/4 完成** | `build.sh --ota --version 1.0.0 --docker` 生成 `.raucb` bundle 文件 |
| 5. 验证 (4项) | 本机构建 + 产物检查 + RAUC 烧录设备验签 + 业务功能验证 | ✅ **4/4 完成** | `rauc install` 成功，`S91app_mount` 签名验证通过，nccu_app 启动 |
| 6. 构建精简与可写目录 (9项) | build.sh 参数简化(全部写死)、编译前自动生成 `version_patch.h` 注入版本号、`run_app` 创建 `/opt/appdata/{log,config,data}` 可写目录、`app_entry` PID+SIGTERM、`app_exit` 新脚本、冗余文件清理 | ✅ **9/9 完成** | `./build.sh` 零参数直接编译成功；`strings nccu_app | grep dev-` 确认版本号已嵌入二进制；`/opt/appdata/{log,config,data}` 目录存在于输出镜像中 |

更细粒度的任务分解和逐项勾选状态见 [tasks.md](tasks.md)。

### 测试结果

<!-- 硬件验证完成：2026-06-18 -->

| # | 测试项 | 结果 | 日志摘要 |
|---|--------|------|----------|
| 1 | 本机构建带签名镜像 `./build.sh --sign-url http://192.168.34.207:8900/api/v1/sign --version 1.0.0` | ✅ PASS | `metadata.sig (256 bytes)` 生成，app.img 大小 268435456 B（精确 256M），metadata magic `D9APPMD1` |
| 2 | 产物完整性检查 | ✅ PASS | ext4 block size 4096、hashtree 2076672 B、metadata 224 B、signature 256 B |
| 3 | RAUC bundle 安装 + S91app_mount 验签挂载 | ✅ PASS | `rauc install app-signed.raucb` → app_b；`Metadata v1: ver=1.0.0 key=ota_signing`，无签名 FAILED 警告，`/dev/mapper/app_b on /app type ext4 (ro,relatime)` |
| 4 | nccu_app 业务功能 | ✅ PASS | `/app/bin/nccu_app` ELF 64-bit ARM aarch64；app_entry 运行；“No upgrade package found”是无 SD 卡的正常业务状态 |

→ 详见 README.md §5（含完整原始操作日志）

### 经验教训

<!-- 硬件验证完成：2026-06-18 -->

| 类别 | 内容 |
|------|------|
| ⚠️ 发现的反模式 | **分区大小不能按数据量自动计算**。S91app_mount 从 `blockdev --getsize64` 读取物理分区大小（256M），从分区末尾 -4096 读取 metadata。如果 app.img 按数据量 3x 计算（如 87M），metadata 会在 87M 位置而非 256M 位置，导致验签失败。分区大小必须与 BPT 硬件分区表一致。 |
| ⚠️ 发现的反模式 | **RAUC 安装前必须即载 app 分区**。app_a 通过 dm-verity 挂载后，`rauc install` 写入会报 `Device or resource busy`。需先 `/etc/init.d/S91app_mount stop` 即载。 |
📝 AGENTS.md 更新 | 在 legacy_app README 中记录新构建命令：`./build.sh --sign-url <URL> --version <VER>` 替代旧的 `source mkenv_arm_st && mk_define release mu3` |
🔧 流程优化 | 镜像输出名带版本号 `app-dev-da86c6e3-20260618.img`，便于追溯；RAUC bundle 同名。 |
🔧 流程优化 | `config/build.conf` 已删除，构建参数全部写死在 `build.sh` 中 |


---

### ⚠️ 归档说明（2026-09-16，**有意未应用 spec delta**）

本变更于 2026-09-16 以以下命令归档 —— **canonical spec 未被修改**：

```bash
openspec archive migrate-legacy-app-build --skip-specs -y
```

**原因**：本变更声明的 3 个 capability 写于本仓完成 OpenSpec 化**之前**，其内容已由后续变更**以更高质量吸收**：

| 本变更的 delta capability | 已被 `app-lifecycle` 覆盖为 |
|---|---|
| `app-build-entry`（build.sh 入口 / Docker / 保留编译流 / 分区大小可配） | **R1 构建流程编排** |
| `app-image-builder`（ext4 4096 / dm-verity hashtree / D9APPMD1 / 远程签名 / 组装 app.img） | **R3 扩展镜像生成、R4 dm-verity、R5 元数据签名与镜像组装** |
| `app-ota-bundle`（RAUC bundle / 远程签名替换 / app-post-install 兼容） | **R6 RAUC OTA Bundle 生成** |

若照常归档，会产生**三份与 `app-lifecycle` 重复且日后必然互相漂移的规范**。故有意跳过 spec 更新，仅保留本提案作为**历史记录** —— 「当时为何废弃 `do_pack()`、为何改走在线签名、为何固定 256M」这些决策依据均在 `proposal.md` / `design.md` / 本 README。

> **规范的权威位置：`openspec/specs/app-lifecycle/spec.md` 的 R1 / R3 / R4 / R5 / R6。** 请勿从本提案（已归档）中寻找规范。

> **后续影响**：`#### 变更信息` 表与本 README 正文均写 `S91app_mount` —— 该文件已改名为 `S92app_mount`（见已归档变更 `fix-app-lifecycle-script-refs`）。本变更作历史记录保留原文，不做回改。