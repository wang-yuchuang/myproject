# Spec: app-lifecycle — 应用生命周期

**Subsystem**: app-lifecycle
**Version**: 0.3
**Created**: 2026-07-01
**Last Updated**: 2026-07-14

---

## Purpose

legacy_app 的生命周期管理，涵盖构建、部署和运行时三个阶段：

**阶段一：构建与打包**（R1-R2）——`build.sh` 编排三阶段流水线（编译 → 镜像 → RAUC bundle），版本号从 `git describe` 自动生成并注入到编译产物。

**阶段二：镜像安全**（R3-R6）——ext4 镜像创建（4096 block）、dm-verity hashtree 生成（纯 Python）、D9APPMD1 元数据远程签名、RAUC OTA bundle 组装。

**阶段三：运行时**（R7-R9）——设备上电后 S91app_mount 挂载分区、app_entry 注册信号、startrun/run_app 启动 syshand 守护进程与 nccu_app 主进程、main.c 按 6 步加载子系统、stoprun 优雅停止。

> **范围说明**：dm-verity 的**运行时挂载**（`veritysetup open` / `dmsetup`）由 SDK 侧的 `S91app_mount` 脚本负责，不在本仓库。本 spec R4 仅描述构建期的 hashtree 生成。
>
> **已知技术债**：当前无进程监管（app_entry 是 shell 脚本，非 fork+waitpid 守护），崩溃后不自愈。`handler.c:123` 的 `system("reboot -f")` 被注释禁用。详见 AGENTS.md 已知技术债。

---
## Requirements
### Requirement: R1 构建流程编排

系统 MUST 提供一键式构建脚本 `build.sh`，支持三个 action（`all` / `build` / `clean`），编排编译、镜像生成和 OTA 打包的完整流水线。

**依赖**：`aarch64-linux-gnu-gcc`（交叉编译器）、`make`、`python3`、`curl`、`openssl`、`git`、`dumpe2fs`

#### Scenario: 完整构建（all）
- **WHEN** 执行 `./build.sh`（默认 action=all）
- **THEN** 依次执行三个阶段：`do_compile` → `do_package` → `do_rauc_bundle`
- **AND** 编译阶段注入版本号到 `src/version_patch.h`，执行 `bash mk_define release mu3` + `MU_TYPE=mu3 make install`，输出 ELF 到 `out/`
- **AND** 打包阶段依次调用 `create_ext4.sh` → `generate_verity.sh` → 校验 block size → `sign_metadata.sh` → `assemble_image.sh` → 校验 magic
- **AND** 最终产物为 `remote_pkg/app-<version>.raucb`
- **AND** 中间产物（`app.ext4`、`app_hashtree.bin`、`metadata.bin` 等）在 RAUC bundle 生成后自动清理（清理发生在 `do_rauc_bundle` 阶段，非 `do_package`）

#### Scenario: 仅编译（build）
- **WHEN** 执行 `./build.sh build`
- **THEN** 仅调用 `do_compile`，执行源码编译
- **AND** 不生成 ext4 镜像、不签名、不打包 RAUC bundle
- **AND** 输出 ELF 二进制和库到 `out/` 目录

#### Scenario: 清理（clean）
- **WHEN** 执行 `./build.sh clean`
- **THEN** 执行 `make clean`（忽略错误）
- **AND** 清空 `out/`、`pkg/`、`remote_pkg/` 目录

#### Scenario: 前置环境检查
- **WHEN** 构建前检查交叉编译器
- **THEN** `command -v aarch64-linux-gnu-gcc` 必须成功，否则报错退出
- **AND** 设置环境变量 `NCCU_CROSS_PLATFORM=arm`、`NCCU_SKIP_ENCRYPT=1`、`NCCU_MAKE_MANIF=ST`

> **写死的生产配置**（build.sh L22-29）：
> - `SIGN_URL="http://192.168.34.207:8900"` — 远程签名服务器
> - `SIGN_TOKEN="token-d9-sdk-001"` — 远程签名令牌
> - `KEY_ID="ota_signing"` — 签名密钥标识
> - `METADATA_VERSION="1"` — D9APPMD1 元数据版本号
> - `PARTITION_SIZE="256M"` — 固定分区大小
> - `RAUC_COMPATIBLE="bmu02_a00"` — RAUC 兼容标识
> - `MU_TYPE="mu3"` — 平台变体

### Requirement: R2 版本号自动管理

系统 MUST 从 `git describe` 自动生成构建版本号，并注入到编译产物和 OTA bundle 的所有元数据中。

#### Scenario: 版本号生成规则
- **WHEN** 构建执行 `auto_version()` 函数
- **THEN** 执行 `git describe --tags --always` 获取版本描述
- **AND** 若返回纯 hex（无 tag）：格式为 `dev-<8位hex>-<YYYYMMDD>`
- **AND** 若返回 tag 描述：格式为 `<git-describe>-<YYYYMMDD>`
- **AND** 若无 git 仓库或失败：回退为 `dev-<YYYYMMDD>`

#### Scenario: 版本号注入源码
- **WHEN** `do_compile` 阶段开始
- **THEN** 生成 `src/version_patch.h`，内容为 `#define SWVERSION_PATCH_NUMBER "<version>"`
- **AND** 此宏被 `config_mangmt.c` 直接引用（`config_mangmt.c:5` `#include "../../../version_patch.h"`；`app_incs.h:41` 的同名 include 已被注释禁用），编译时注入到二进制中
- **AND** 该文件为自动生成，不应手动编辑

#### Scenario: 版本号注入元数据
- **WHEN** `sign_metadata.sh` 构建元数据二进制（`metadata.bin`）
- **THEN** 版本号写入 `app_version` 字段（32 字节，`\0` 填充）
- **AND** `ota-bundle.sh` 将版本号写入 RAUC manifest 的 `version=` 字段
- **AND** 产物文件名包含版本号：`app-<version>.img`、`app-<version>.raucb`

##### Test: 设备端版本查询
- **Type**: functional
- **Method**: 在设备上执行 `cat /var/lib/rauc/slot-status | grep bundle.version`
- **Expected**: 返回值与构建时 `APP_VERSION` 一致

### Requirement: R3 ext4 镜像生成

系统 MUST 将编译产物（`out/` 目录）打包为固定大小的 ext4 镜像，block size 必须为 4096 以兼容 dm-verity。

**实现**：`scripts/create_ext4.sh`

#### Scenario: ext4 创建
- **WHEN** `do_package` 调用 `create_ext4.sh --input-dir <out> --output pkg/app.ext4 --size <bytes>`
- **THEN** 镜像大小 = `PARTITION_SIZE - 2% overhead - 4096`（预留给 metadata 和签名）
- **AND** 使用 `mke2fs -t ext4 -b 4096 -d <input_dir> -L "APP_DATA"` 创建
- **AND** 文件预截断到 block 对齐大小（`truncate -s`）

#### Scenario: block size 校验
- **WHEN** ext4 镜像创建后
- **THEN** `build.sh` 执行 `dumpe2fs -h pkg/app.ext4 | grep 'Block size'` 校验
- **AND** block size MUST 为 4096，否则报错退出（dm-verity 兼容性要求）

### Requirement: R4 dm-verity 完整性校验

系统 MUST 在构建期生成 dm-verity hashtree（Merkle tree），用于运行时挂载时验证 app 分区完整性。

**实现**：`scripts/generate_verity.sh` 使用**纯 Python 实现**（不依赖 `veritysetup` CLI），参数由 `config/verity.conf` 提供。

**参数**（`config/verity.conf`）：

| 参数 | 值 |
|------|-----|
| `HASH_ALGORITHM` | sha256 |
| `DATA_BLOCK_SIZE` | 4096 |
| `HASH_BLOCK_SIZE` | 4096 |
| `SALT_SIZE` | 32 |

#### Scenario: hashtree 生成（构建期）
- **WHEN** `generate_verity.sh --image app.ext4 --output-dir pkg/ --block-size 4096`
- **THEN** Python 脚本逐 block 读取 ext4 镜像，计算 `sha256(salt + block_data)` 叶子节点哈希
- **AND** 每个数据 block 产生 32 字节哈希，128 个哈希打包为一个 hash block（4096 / 32 = 128）
- **AND** 自底向上构建多级 Merkle tree，直到只剩一个根哈希
- **AND** Salt 为 32 字节随机数（`os.urandom`），每次构建重新生成
- **AND** 输出 `pkg/app_hashtree.bin`（树数据，不含 root hash）和 `pkg/verity_info.txt`（含 Root hash / Salt / Hash blocks 数量）

#### Scenario: 运行时 verity 挂载（SDK 侧，非本仓库）
- **WHEN** 设备启动时 SDK 的 `S92app_mount` 挂载 app 分区
- **THEN** 使用元数据中的 Root hash 建立 dm-verity 设备
- **AND** 只读挂载（`-o ro`）防止 superblock 被修改
- **AND** 任何 block 被篡改时 dm-verity 校验失败，拒绝挂载

> **注意**：原 spec 描述的 "使用 `veritysetup open` 建立 dm-verity 设备" 是**设备端运行时行为**，由 SDK 侧 S92app_mount 实现，不在本仓库代码中。本仓库仅负责构建期生成 hashtree 和元数据。

### Requirement: R5 元数据签名与镜像组装

系统 MUST 通过远程签名服务器对 D9APPMD1 元数据进行签名，并将 ext4 + hashtree + 签名 + 元数据组装为固定 256MB 的 app.img。

**实现**：`scripts/sign_metadata.sh`（元数据生成 + 远程签名）+ `scripts/assemble_image.sh`（镜像组装）

#### Scenario: D9APPMD1 元数据二进制格式
- **WHEN** `sign_metadata.sh` 构建元数据（`metadata.bin`）
- **THEN** 二进制格式为：`magic(8)` + `version(4)` + `total_len(4)` + `header_len(4)` + `image_size(8)` + `hash_tree_offset(8)` + `hash_tree_size(8)` + `data_block_size(4)` + `hash_block_size(4)` + `data_blocks(8)` + `root_hash_len(4)` + `salt_len(4)` + `root_hash(32)` + `salt(32)` + `app_version(32)` + `rollback_index(8)` + `key_id(32)` + `signature_alg(16)` + `signature_len(4)`
- **AND** magic 为 ASCII `D9APPMD1`
- **AND** signature_alg 为 `RSA-SHA256`，signature_len 为 256（RSA-2048）
- **AND** key_id 为 `ota_signing`

#### Scenario: 远程签名
- **WHEN** `sign_metadata.sh` 调用远程签名服务
- **THEN** 将 `metadata.bin` base64 编码后 POST 到 `${SIGN_URL}/api/v1/sign`
- **AND** 请求头包含 `Authorization: Bearer ${SIGN_TOKEN}`
- **AND** 请求体为 JSON：`{"project":"d9_sdk","version":"<ver>","key_id":"<id>","metadata":"<base64>"}`
- **AND** 返回的 base64 签名解码后写入 `metadata.sig`（256 字节）
- **AND** 若未提供 `--sign-url` → 输出 WARNING，`metadata.sig` 创建为空文件（无签名构建）
- **AND** 若提供 `--sign-url`（在线签名服务）但服务返回空签名 → 输出 ERROR + `exit 1`（硬失败，构建中止）
- **AND** 若提供 `--sign-url`（本地密钥下载）但密钥未在归档中找到 → 输出 ERROR + `exit 1`（硬失败）

#### Scenario: 镜像组装（assemble_image.sh）
- **WHEN** `assemble_image.sh` 组装最终 app.img
- **THEN** 布局为：`[ext4 数据] [hashtree] [zero padding] [signature] [metadata]`
- **AND** metadata 固定位于 partition_end - 4096（最后 4KB）
- **AND** signature 紧邻 metadata 之前（partition_end - 4096 - sig_size）
- **AND** 总大小严格等于 PARTITION_SIZE（256MB）

#### Scenario: 镜像 magic 校验
- **WHEN** app.img 组装完成后
- **THEN** `build.sh` 执行 `dd if=app.img bs=1 skip=$((256*1024*1024 - 4096)) count=8` 读取最后 4KB 起始 magic
- **AND** magic MUST 为 `D9APPMD1`，否则报错退出

##### Test: 签名完整性验证
- **Type**: functional
- **Method**: 提取 metadata.bin 和 metadata.sig，用对应公钥验证 RSA-SHA256 签名
- **Expected**: 签名验证通过

### Requirement: R6 RAUC OTA Bundle 生成

系统 MUST 将签名后的 app.img 封装为 RAUC verity bundle，支持通过 RAUC 框架进行 A/B 槽位 OTA 升级。

**实现**：`scripts/ota-bundle.sh`，优先使用 `tools/bin/rauc`（自包含），回退到 Buildroot host-rauc，最后系统 rauc。

#### Scenario: Bundle manifest 生成
- **WHEN** `ota-bundle.sh` 构建 bundle
- **THEN** 生成 `manifest.raucm`：
  ```
  [update]
  compatible=bmu02_a00
  version=<APP_VERSION>
  description=App partition update for bmu02_a00

  [bundle]
  format=verity

  [image.app]
  filename=app.img
  ```
- **AND** bundle format 为 `verity`（RAUC 1.15+ 支持的增量格式）

#### Scenario: 远程 CMS 签名
- **WHEN** 使用远程 CMS 签名（`--sign-url` 提供）
- **THEN** 流程为：生成临时 RSA-2048 密钥 → 用临时密钥创建 verity bundle → 提取 CMS 签名 → 剥离临时证书 → 远程 CMS 重签（3 次重试）→ 替换签名
- **AND** 远程签名 URL：`${SIGN_URL}/api/v1/sign/rauc/cms?project=d9_sdk&key_id=ota_signing`
- **AND** 临时密钥用完后 `shred -u` 安全删除

#### Scenario: 本地签名回退
- **WHEN** 提供 `--rauc-cert` 和 `--rauc-key` 而非远程签名
- **THEN** 使用 `rauc bundle --cert=<cert> --key=<key>` 直接签名
- **AND** 证书和密钥文件必须存在

#### Scenario: RAUC 安装（设备端）
- **WHEN** 在设备端执行 `rauc install remote_pkg/app-<version>.raucb`
- **THEN** RAUC 将 bundle 解包到备用槽位（app.0 或 app.1）
- **AND** 下次重启后从新槽位启动
- **AND** 安装前 MUST 先执行 `S92app_mount stop`（卸载当前 /app）

### Requirement: R7 运行时启动链

系统 MUST 按固定顺序启动，从 SDK 分区挂载到 syshand 守护进程和 nccu_app 主进程的就绪。

**启动链**：
```
S91app_mount start  (SDK 侧: dm-verity 挂载 /app 只读)
  └─ /app/app_entry /app &           ← PID 写入 /var/run/app.pid
       └─ trap SIGTERM → stoprun
       └─ startrun
            ├─ 检测 USB/SD 升级介质
            ├─ 若有升级 → app_update -fwupdt
            └─ 若无升级 → /app/run_app
                 ├─ 设置环境变量 (NCCU_RUN_PATH / LD_LIBRARY_PATH)
                 ├─ 创建 /opt/appdata/{log,config,data}
                 ├─ 首次复制 /app/config/ → /opt/appdata/config/ (幂等 cp -rn)
                 ├─ 复制 log4crc 到 /opt/appdata/config/
                 ├─ ./bin/syshand &                    ← 守护进程
                 ├─ 等待 syshand 存活检查
                 └─ ./bin/nccu_app -pid=$APP_PID_FILE  ← 主进程
```

#### Scenario: app_entry 信号处理
- **WHEN** `app_entry` 被 S91app_mount 调用（`/app/app_entry /app &`）
- **THEN** 写入当前 PID 到 `/var/run/app.pid`
- **AND** 注册 SIGTERM trap：收到信号时调用 `stoprun`、删除 PID 文件、退出
- **AND** 调用 `startrun` 启动业务

#### Scenario: 升级介质检测（startrun）
- **WHEN** startrun 检测启动路径
- **THEN** 检查 `/var/.ext_mark` 文件存在性：存在则跳过升级检测（升级后首次启动）
- **AND** 否则检测 USB 设备（`/dev/sd*`）和 SD 卡设备（`/dev/mmcblk[1-9]*`）
- **AND** 若发现可移动设备 → 执行 `app_update -qws -nokeyboard -fwupdt`
- **AND** 若无可移动设备 → 调用 `wait_appupdt_exe_done`（等待 firmwareupdt.flag 消失）后调用 `/app/run_app`

> **SD 卡检测修正**（2026-07）：原代码使用 `ls /dev/mmc*` 会误检测内部 eMMC（`/dev/mmcblk0`），已修正为 `ls /dev/mmcblk[1-9]*` 以排除 mmcblk0。

#### Scenario: run_app 环境准备
- **WHEN** `run_app` 执行
- **THEN** 设置 `NCCU_RUN_PATH=/app`、`NCCU_DATA_PATH=/data`、`LD_LIBRARY_PATH` 包含 `/app/lib:/app/comm_port:/app/extensions`
- **AND** 通过 hostname 判断运行平台（`bmu*` → `VAR_DIR=/opt/appdata/data`）
- **AND** 创建运行时可写目录：`/opt/appdata/{log,config,data}`
- **AND** 首次启动幂等复制配置：`cp -rn /app/config/* /opt/appdata/config/`
- **AND** 复制 log4crc：`/app/extensions/log4crc → /opt/appdata/config/log4crc`

#### Scenario: syshand 守护进程启动
- **WHEN** `run_app` 启动 syshand
- **THEN** 检查 syshand 是否已在运行（`ps | grep [s]yshand`）
- **AND** 若已在运行 → 确保 `/opt/appdata/data/syshand.run` 标记文件存在
- **AND** 若未运行 → `./bin/syshand &` 启动，写入 PID 到 `$SYSHAND_PID_FILE`
- **AND** 等待 BSP 扩展安装完成（`/app/extensions/install.sh` 进程退出）
- **AND** 存活检查失败（syshand 未运行或 PID 文件缺失）→ `exit 0` 终止启动

#### Scenario: nccu_app 启动
- **WHEN** syshand 存活检查通过
- **THEN** 检查 nccu_app 是否已在运行（避免重复启动）
- **AND** 执行 `sync && echo 3 > /proc/sys/vm/drop_caches` 清理缓存（**仅 mu3 变体**，`src/shells/mu3/run_app:143-144`；非 mu3 版 `run_app` 无此操作）
- **AND** 启动 `./bin/nccu_app -pid=$APP_PID_FILE`，stdout/stderr 重定向到 `/opt/appdata/log/trace.log`

### Requirement: R8 nccu_app 初始化序列

nccu_app 主进程 MUST 按严格顺序完成初始化，任何步骤失败都将导致进程退出。

**实现**：`src/main/main.c`

**预初始化**（步骤 1-16，在子系统加载之前）：

| 步骤 | 函数 | 失败行为 |
|------|------|---------|
| 1 | `Main_ProcessCmdLine` | 解析命令行参数 |
| 2 | `MemoryPool_Init` | 初始化内存池 |
| 3 | `Lock_SqlDB_FileLocker` | 等待其他进程释放 DB 锁（最多 30 次重试，每次 1s）|
| 4 | `sqlite3_config(SERIALIZED)` | SQLite 多线程安全模式 |
| 5 | `sqlite3_open_v2` | 打开/创建 `$NCCU_DATA_PATH/nccu.data`（运行时 `/data/nccu.data`；`run_app` 设 `NCCU_DATA_PATH=/data`）|
| 6 | `InitCreatDataBase` | 数据库表结构初始化 → 失败 `return -1` |
| 7 | `Init_Database` | 日志表创建 → 失败 `return -1` |
| 8 | `Database_Switch_RunMode` | 切换到运行时模式 |
| 9 | `Init_CreadDataDB` | 数据 DB 初始化 → 失败 `return -1` |
| 10 | `test_log_init` | log4c 初始化 → 失败 `return -1` |
| 11 | `Create_Charger` | 创建 CHARGER 上下文 → 失败 `return -1` |
| 12 | 等待 `syshand.run` | 轮询 `/opt/appdata/data/syshand.run`（`nMaxWait` 从 0 起，每次 `Sleep_Ex(100)` 后自增，当 `nMaxWait > 100` 时退出，共 101 × 100ms = ~10.1s 超时；main.c:314-326）|
| 13 | `WatchDog_Open` | 硬件看门狗（当前禁用）|
| 14 | `Install_SigHandlers` | 注册信号处理器 |
| 15 | `setjmp` | 设置故障跳转点 |
| 16 | `RunThread_ManagerInit` | 线程管理器初始化 → 失败 `return -1` |

#### Scenario: 6 步子系统加载（Main_LoadModuals）
- **WHEN** 预初始化完成后调用 `Main_LoadModuals`
- **THEN** 按以下顺序加载：
  1. `Init_CreateDataProcessor` — DPR 数据中心初始化
  2. `LoadConfigAndData` — 从 SQLite `System.cfg` 加载配置和运行时数据
  3. `Init_LoadDAI` — 数据访问接口加载
  4. `Init_Load_AllCommPorts` — 9 个通信模块动态加载（`dlsym`）
- **AND** 在步骤 4 和 5 之间调用 `Set_SqliteDB_2RunTime_Mode` 切换 SQLite 到运行时模式
  5. `Init_Load_AllSamplers` — 采样器模块加载
  6. `Init_Load_AllReporters` — 上报器模块加载
- **AND** 任何步骤失败 → 记录错误日志、`return FALSE` → main 返回 -1

#### Scenario: 事件主循环
- **WHEN** `Main_LoadModuals` 成功后进入主循环（`while(!s_pChargeThis->bExit)`）
- **THEN** 每次循环调用 `Do_Proc_HeartBeat()` 向 syshand 发送心跳
- **AND** 从事件队列（`hqEvent`）取出事件，100ms 非阻塞超时
- **AND** 有事件时调用 `dataprocessor.DoEvent()` 处理
- **AND** 处理 `bResetCmdPending` 系统重置请求（最多等待 10s 后 `_SYSTEM("reboot")`）
- **AND** 线程管理器心跳计数器递减，归零时触发 PANIC 退出

#### Scenario: 故障退出（longjmp）
- **WHEN** 信号处理器触发 `longjmp(s_JumpBufFaultError, nExitCode)`
- **THEN** 跳回 main 中 `setjmp` 返回点
- **AND** 调用 `RunThread_ManagerExit(10000, TRUE)` 等待线程退出（10s 超时，强制 cancel）
- **AND** 调用 `_exit(nExitCode)`（不调用 `exit`，避免 atexit 钩子）

#### Scenario: 优雅退出与资源释放
- **WHEN** 主循环正常退出（bExit=TRUE）
- **THEN** 调用 `RunThread_ManagerExit(10000, TRUE)` 停止所有线程
- **AND** 调用 `Main_Exit` 清理资源
- **AND** 调用 `MemoryPool_Exit` 释放内存池
- **AND** 调用 `Exit_LogRepo` 关闭日志
- **AND** `sqlite3_close(db)` + `ExitDatabase()` + `sqlite3_shutdown()`

### Requirement: R9 优雅停止与信号处理

系统 MUST 在收到停止信号时优雅停止 syshand 和 nccu_app 进程，并清理 PID 文件。⚠️ **该能力当前不可达**（见下方 Scenario 与"已知缺陷"）

**实现**：`src/shells/app_entry`（SIGTERM trap）+ `src/shells/stoprun`（停止脚本）

#### Scenario: SIGTERM 触发停止（⚠️ 当前不可达）

- **WHEN** `S92app_mount stop` 读取 `/var/run/app.pid` 并向该 PID 发送 SIGTERM
- **THEN** ⚠️ **trap 不会触发**：`app_entry` 在 `:64` 调用 `startrun` 后**立即结束**（`startrun` 将 syshand 与 nccu_app 后台化），shell 退出则 trap 随之消失，`/var/run/app.pid` 成为**死 PID**
- **AND** 目标行为（设计意图）：trap 依次执行 ① 打印停止日志 ② 调用 `${APP_SHELL_PATH}/stoprun` ③ 删除 `/var/run/app.pid` ④ `exit 0`

#### Scenario: stoprun 停止顺序
- **WHEN** `stoprun` 被调用
- **THEN** 先 `killall -9 app_update` 停止升级进程；再按**变体**停止 UI 进程：**非 mu3** 为 `killall -9 nccu_ui_mm`（`stoprun:13`）／**mu3（bmu05 实际生效）** 为 `killall -9 nccu_ui`（`mu3/stoprun:7`）
- **AND** 然后通过 PID 文件（`/opt/appdata/data/syshand.pid`）向 syshand 发送 SIGTERM
- **AND** 最后通过 PID 文件（`/opt/appdata/data/nccu.pid`）向 nccu_app 发送 SIGTERM
- **AND** 每个进程停止后删除对应的 PID 文件

#### Scenario: PID 文件管理
- **WHEN** 进程启动和停止时
- **THEN** app_entry PID → `/var/run/app.pid`（供 S92app_mount stop 使用）⚠️ **但 app_entry 随即退出，该 PID 实际为死 PID**
- **AND** syshand PID → `/opt/appdata/data/syshand.pid`（供 stoprun 使用）
- **AND** nccu_app PID → `/opt/appdata/data/nccu.pid`（由 `nccu_app -pid=` 参数写入，供 stoprun 使用）
- **AND** 进程退出时 MUST 删除对应 PID 文件，避免僵尸 PID
> **已知缺陷（2026-09-17 核实）**：`app_entry` 是**非驻留**脚本（调用 `startrun` 后即退出），因此 SIGTERM trap **不可达**、`/var/run/app.pid` 为死 PID ⇒ **当前停止链路不可用**。
> 正解需**前台 supervisor**（持有子进程并转发信号），属 `app-supervision` 范畴；已记录于 `keep-app-alive-on-bmu05` 的 `design.md`。
> ⚠️ `src/shells/app_entry:14` 的注释「4. 注册 SIGTERM 信号处理器，优雅退出」同样与此事实不符（本变更不改代码，仅记录）。

### Requirement: R10 AP↔R 跨核系统管理守护（S91app_sysmgr / app_sysmgr，BSP 契约）

系统 MUST 在 boot 早期由 BSP 侧 `/etc/init.d/S91app_sysmgr` 拉起 `/usr/sbin/app_sysmgr`，作为 AP 核与 R 核之间的**系统级**管理守护：BOOT_READY 握手、SSDK 版本与 form 一致性校验、R 核 active slot 查询、RAUC 槽位标记与跨核心跳。

本 Requirement 是**只读契约** —— 实现归 BSP 仓，本仓库仅记录其可观测行为及其对**应用启动链**的边界。本守护 **MUST NOT** 承担启动 `/app/app_entry` 的职责。

**位置与形态**（2026-09-16 bmu05 真机取证）：
- init 脚本 `/etc/init.d/S91app_sysmgr`（1122 B）执行 `"$APP" &`，`APP=/usr/sbin/app_sysmgr`，PID 文件 `/var/run/app_sysmgr.pid`
- 二进制 `/usr/sbin/app_sysmgr`；进程 **4 线程**；`fd 1,2 → /dev/console`（输出走串口、**不落文件**）
- `fd 3-15` 全为 rpmsg：`/dev/rpmsg_ctrl0..3`、`/dev/rpmsg0..3`、sysfs `soc:ipcc@1.sysmgr.-1.500` 与 `virtio0.sysmgr.-1.500`
- 常态 `S (sleeping)` / `wchan = hrtimer_nanosleep` → 定时循环，非死锁

**与 app 启动链的边界**：
- 本守护占用 **S91** init 槽位，是原 `S91app_mount` 改名为 `S92app_mount` 的原因（S92 跑在其后）
- 应用启动 owner **不是**本守护：boot 路径为 `S92app_mount`，OTA 路径为 `/usr/lib/rauc/app-post-install`
- 本守护的**跨核心跳**（AP↔R，PING/PONG）与 `system-hand` 的**应用心跳**（180 s 无心跳）是**两套独立机制**，MUST NOT 混用

**v1 → v2 能力移除（历史事实，供追溯）**：v1（`/data/app_sysmgr`，2025-07-29）含 `/app/app_entry` 与 `Started app_entry (pid=%d)`，并曾提供 `/var/run/sysmgr.sock`；v2（`/usr/sbin/app_sysmgr`，Sep 11）对 `app_entry` / `nccu_app` / `syshand` / `run_app` 的 `strings` 匹配数均为 **0**。

#### Scenario: boot 早期拉起
- **WHEN** 系统 boot 进入 S91 阶段
- **THEN** `/usr/sbin/app_sysmgr` 被后台拉起
- **AND** 其标准输出与错误写入 `/dev/console`，不产生应用侧日志文件
- **AND** PID 记录于 `/var/run/app_sysmgr.pid`

#### Scenario: Phase 1 BOOT_READY 握手
- **WHEN** `app_sysmgr` 启动
- **THEN** 向 boot_ready sysfs 连续写入 `BOOT_READY` 三次，逐次记录 `BOOT_READY #%d sent`，完成后记录 `BOOT_READY x3 handshake COMPLETE`
- **AND** 随后记录 `R-core g_ap_ready = true, iccmsg gate released`，释放跨核消息闸门
- **AND** 若 boot_ready sysfs 不存在，记录 `ERROR: boot_ready sysfs not found` 而不中断进程

#### Scenario: Phase 2 R 核信息查询
- **WHEN** Phase 1 完成
- **THEN** 查询 R 核 sysmgr 端点，取得 SSDK 版本、AP/R form 一致性（`AP/R form OK: %s`，不一致则 `WARN: AP/R form mismatch: ap=%s r=%s`）与 R 核 active slot（`R-core active slot: %s (media: %s)`）
- **AND** 查询失败时记录 `iccmsg_hal_init failed (sysmgr ept not ready?)`，并按 `sysmgr query retry #%d (ver_ok=%d slot_ok=%d, R-core not ready?)` 重试
- **AND** 重试耗尽后记录 `WARN: slot query failed after retries; cache may be stale`（非致命）
- **AND** 版本信息变化时执行 `S00version-init --refresh`

#### Scenario: RAUC 槽位标记
- **WHEN** R 核信息查询成功
- **THEN** 记录 `MARK_GOOD ok (slot %c GPT retry reset)`，完成 RAUC 槽位的 good 标记并重置 GPT 重试计数
- **AND** 失败时记录 `WARN: MARK_GOOD failed (non-fatal, boot retry will keep counting)`，不影响进程继续运行

#### Scenario: 跨核心跳（AP↔R，独立于应用心跳）
- **WHEN** 守护进入稳定运行
- **THEN** 记录 `heartbeat started (PING every %ds)`，周期性向 R 核发送 PING
- **AND** 心跳恢复时记录 `R-core heartbeat recovered (uptime=%ums)`
- **AND** 丢失时记录 `WARN: R-core heartbeat LOST (no PONG)`，连续丢失记录 `heartbeat dead %d`
- **AND** 跨核通道初始化失败时记录 `heartbeat: iccmsg_hal_init failed (retry in 5s, #%d)` 并每 5 秒重试

#### Scenario: 不承担应用启动职责
- **WHEN** 应用未在运行（例如冷重启之后）
- **THEN** 本守护 MUST NOT 启动 `/app/app_entry`
- **AND** 应用启动 MUST 由 `S92app_mount` 的 boot 路径或 RAUC `app-post-install` 的 OTA 路径负责

### Requirement: R11 构建脚本自包含

本仓库的构建流水线 MUST NOT 依赖指向外部仓库的符号链接。`scripts/` 下的构建脚本 MUST 是本仓库内的**实体文件**，使不具备 `d9_sdk` 访问权限的开发人员能够独立完成构建。

**背景**（历史事实，供追溯）：改造前 `scripts/` 下 5 个构建脚本均为指向 `d9_sdk/source/test_app/scripts/` 的**符号链接**，离开 d9_sdk 树即无法构建。本变更将其替换为实体文件副本，并新增 `scripts/UPSTREAM.md` 记录上游来源与同步策略。

**同步策略**：`scripts/UPSTREAM.md` MUST 记录每个脚本的上游仓库路径与同步方式。上游变更时由人工同步，MUST NOT 通过符号链接实时跟随。

#### Scenario: 无 d9_sdk 权限时独立构建

- **WHEN** 不具备 `d9_sdk` 仓库访问权限的开发人员执行 `./build.sh all`
- **THEN** 构建成功完成，产出 `app.img` 与 RAUC bundle
- **AND** 构建全程 MUST NOT 访问本仓库之外的路径

#### Scenario: 脚本来源可追溯

- **WHEN** 需要核对 `scripts/` 下某个脚本与其上游的差异
- **THEN** `scripts/UPSTREAM.md` 提供该脚本的上游仓库路径与同步记录
- **AND** `scripts/` 目录下 MUST NOT 存在任何符号链接

### Requirement: R12 mu3 平台启动脚本与主脚本功能一致

`MU_TYPE=mu3` 时构建实际加载的是 `src/shells/mu3/run_app`（**不是** `src/shells/run_app`）。因此对任一 `run_app` 变体的修复 MUST 同步到另一个变体，二者 MUST 在以下方面行为一致：

- `LD_LIBRARY_PATH` MUST 包含 `/app/extensions`
- MUST NOT 存在任何向 `/usr/lib/` 拷贝 `.so` 的命令
- syshand 存活判定 MUST 使用**进程检查**（`pidof syshand` 或 `ps | grep '[s]yshand'`），MUST NOT 仅依赖 PID 文件存在性
- MUST 定义 `TRACE_LOG` 并创建 `/opt/appdata` 运行目录
- log4crc 拷贝 MUST 带容错（`2>/dev/null || true`）

**背景**（历史事实，供追溯）：`MU_TYPE=mu3` 的构建实际使用 `mu3/run_app`，但早期修复只做在主 `run_app` 上，导致 mu3 版缺 `LD_LIBRARY_PATH`、仍向 `/usr/lib` 拷贝、且用 PID 文件判定 syshand —— 应用启动后立即退出，核心业务无法运行。

#### Scenario: 修改任一变体需同步

- **WHEN** 修改 `src/shells/run_app` 或 `src/shells/mu3/run_app` 中的任一文件
- **THEN** 另一个变体同步修改，二者在上述各方面行为一致

#### Scenario: mu3 变体的 syshand 存活判定

- **WHEN** `mu3/run_app` 判定 syshand 是否已运行
- **THEN** 使用进程检查而非 PID 文件存在性
- **AND** 判定为已运行时 MUST 确保 `/opt/appdata/data/syshand.run` 标记文件存在（供 `nccu_app` 就绪检查使用）

### Requirement: R13 nccu_app 以生产模式直接启动

`run_app` MUST 直接启动 `nccu_app`，MUST NOT 使用 `strace` 等调试包装器。stderr MUST 重定向到追踪日志，以保留错误可见性。

**背景**（历史事实，供追溯）：`run_app` 曾以 `strace -o $TRACE_LOG ... ./bin/nccu_app` 的形式启动（调试残留），不应出现在生产启动链中。

#### Scenario: 生产启动

- **WHEN** `run_app` 启动 nccu_app
- **THEN** 命令形式为 `./bin/nccu_app -pid=$APP_PID_FILE 2>>$TRACE_LOG &`
- **AND** MUST NOT 出现 `strace` 包装

### Requirement: R14 libnopoll SONAME 兼容（libnopoll.so.0）

构建产物 MUST 在部署 `libnopoll.so` 的同时创建指向它的 **`libnopoll.so.0`** 链接，以满足动态链接器对 SONAME 的解析需求。

**机制**：`libnopoll.so` 的 SONAME 是 `libnopoll.so.0`，而 `libcomm_ws_ex.so`（comm_websocket_ex 模块）的 `NEEDED` 条目同样是 `libnopoll.so.0`。动态链接器按 **SONAME** 查找而非文件名，因此部署中仅有 `libnopoll.so` 而无 `.so.0` 链接时，`dlopen` 会失败。

**实现**：`src/extensions/Makefile` 安装目标中创建链接 —— `-ln -sf libnopoll.so $(NCCU_TARGET_DIR)/extensions/libnopoll.so.0`。

#### Scenario: dlopen libcomm_ws_ex.so

- **WHEN** 通信层通过 `dlopen` 加载 `libcomm_ws_ex.so`
- **THEN** 其依赖 `libnopoll.so.0` MUST 可在库搜索路径中解析
- **AND** MUST NOT 出现 `cannot open shared object file` 错误

