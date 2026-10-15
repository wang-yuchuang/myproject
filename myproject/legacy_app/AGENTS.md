# legacy_app — Solaris Backup Management System

**基于 SemiDrive D9360 平台的储能后备管理系统 App 层业务程序**

**Commit**: dev 分支 | **Specs**: `openspec/specs/`（7 个子系统，版本见各 spec 头部）| **Snapshot**: `openspec/.code-facts-snapshot.md`

---
## 构建

```bash
./build.sh              # 一键构建（编译 + 在线签名 + RAUC bundle）
./build.sh build        # 仅编译
./build.sh clean        # 清理
```

| 输出文件 | 路径 |
|---------|------|
| RAUC OTA bundle | `remote_pkg/app-<version>.raucb` |
| app 镜像（工厂烧录） | `pkg/app-<version>.img` |

## 目录结构

```
legacy_app/
├── build.sh                # 构建入口（所有参数写死）
├── scripts/                # 构建流水线（ext4 / verity / sign / assemble / ota）
├── config/verity.conf      # dm-verity 参数（sha256 / 4096 block）
├── tools/                  # RAUC 工具链（bin/rauc + 依赖库）
├── src/                    # 源码（C，~40 子模块）
│   ├── main/               # nccu_app 主进程入口
│   ├── publib/             # 公共基础库（线程/锁/队列/内存池/日志/DB）
│   ├── comm_ports/         # 通信抽象层（9 个 HAL 模块）
│   ├── utilities/system_hand/  # syshand 守护进程
│   ├── moduals/data_processor/ # DPR 数据中心（配置/事件/告警/表达式）
│   ├── moduals/samplers/   # 数据采集（**10 个编译** / 7 个未编译）
│   ├── moduals/reporters/  # 数据上报（8 个编译）
│   └── shells/             # 启动脚本（app_entry/startrun/stoprun）
├── openspec/               # 架构 specs + 变更提案 + 代码事实快照
├── out/                    # 编译产物
├── pkg/                    # app.img 中间产物
└── remote_pkg/             # 最终 .raucb 输出
```

## 架构全景

### 双进程模型

```
设备上电
  ├─ S91app_sysmgr → /usr/sbin/app_sysmgr      ← **AP↔R 跨核管理守护 + app 保活**
  │     职责: BOOT_READY×3 握手 / SSDK 版本与 AP-R form 校验 / R 核 active slot
  │           / RAUC `MARK_GOOD` / 跨核 heartbeat PING+LOST
  │     ✅ **BSP 设计上由它门控启动 app**（`fork` + `exec` `/app/app_entry /app`，崩溃后退避重启）
  │        依据: d9_sdk `openspec/changes/add-system-mgmt-service/specs/app-boot/spec.md` 的 **R2**（该变更仍在途）
  │     ⚠️ 但**当前 rootfs 的二进制未含该能力**（`sysmgr_process.c` 有实现、**Makefile 未编译**）
  │        → 这正是「冷重启后无人启动应用」的成因（**迁移空档，非设计缺陷**）
  │
  └─ S92app_mount (SDK 侧: 选槽 + 验 D9APPMD1 + dm-verity 挂载 /app 只读)
       ⚠️ **按设计仅挂载、不启动 app** —— 同一 spec 的 **R1** 明确要求“移除 `run_app` / `app_entry` 调用”
       ⚠️ 其 `run_app()` 内的启动代码是**遗留死代码**（定义后从未被调用），**不得加回**
       ⚠️ 当前唯一能启动应用的是 OTA 路径：/usr/lib/rauc/app-post-install（安装时）

应用启动链（boot 与 OTA 两条路径共用同一入口）
  /app/app_entry /app &
       ├─ PID → /var/run/app.pid（⚠️ 脚本随即退出，实为死 PID）
       ├─ trap SIGTERM → stoprun（⚠️ 当前不可达）
       └─ startrun
            └─ /app/run_app              ← 运行时准备（环境变量 / 目录 / syshand）
                 ├─ ./bin/syshand &      ← 守护进程
                 └─ ./bin/nccu_app       ← 主进程（内部等待 syshand.run 就绪，main.c:316）

syshand (src/utilities/system_hand/)
  职责: BSP 扩展安装 / 心跳监控 / 系统命令代理
  IPC: System V 共享内存 + 命名信号量
  心跳阈值: 180s (18 × 10s 窗口)

nccu_app (src/main/main.c)
  6 步加载:
    1/6 Init_CreateDataProcessor   (DPR 核心)
    2/6 LoadConfigAndData          (SQLite System.cfg)
    3/6 Init_LoadDAI
    4/6 Init_Load_AllCommPorts     (9 个通信模块)
    5/6 Init_Load_AllSamplers      (7 个采样器)
    6/6 Init_Load_AllReporters     (8 个上报器)
```

### 数据流

```
设备 ─→ samplers ──→ DPR(配置/事件/告警/表达式) ──→ reporters ──→ 云平台/本地
  ↑                                              ↑
  └── comm_ports (9 模块, 统一 HAL 接口) ────────┘
```

详细架构见 `openspec/specs/` 各子模块 spec。

## 运行时可写目录

| 目录 | 用途 |
|------|------|
| `/opt/appdata/log` | 业务日志（TRACE_LOG）—— 应用 stdout/stderr 重定向于此 |
| `/opt/appdata/config` | 配置覆写（首次复制 `/app/config/`）、`log4crc` |
| `/opt/appdata/data` | 运行时数据：PID（`syshand.pid` / `nccu.pid`）、锁（`.{applog,bmsrundata,clouddata}.lock`）、`.logIdx`、`syshand.run`、升级临时件 |
| **`/data`** | **SQLite 数据库**：`nccu.data`、`AppLog.data`、`BmsRunData.data`、`CloudData.data`（由 `NCCU_DATA_PATH=/data` 决定）+ `loginfo.N` |
| `/var/volatile/` | 运行时临时文件（tmpfs）；应用侧现已基本改用 `/opt/appdata/data` |

> ⚠️ **`/opt/appdata` 是 bind mount**，源为 `/data/rootfs/opt/appdata`（`S00data_mount`）—— 它与 `/data` **同分区但不同目录**，因此 `/opt/appdata/data/nccu.data` **不存在**，DB 在 `/data/` 顶层。详见 `.code-facts-snapshot.md` 的 `### 运行时可写路径与 DB/锁文件`

## 版本号

自动从 `git describe` 生成，注入：`version_patch.h`、D9APPMD1 metadata、RAUC bundle、文件名。
设备端查询：`cat /var/lib/rauc/slot-status | grep bundle.version`

## 重要规则

- **编译依赖**: `aarch64-linux-gnu-gcc` 交叉编译器、`make`、`mkfs.ext4`、`python3`、`curl`、`openssl`、`git`
- **签名服务器**: 写死在 `build.sh` 中（`http://192.168.34.207:8900`），生产环境需确认可用
- **分区大小**: 固定 256MB（与硬件 app_a/app_b 分区一致）
- **不要用 `dd` 写入已挂载的 app 分区**，应使用 RAUC 安装
- **RAUC 安装前需先** `S92app_mount stop`

---

## 子系统关键事实（spec 全量审核结果）

| 子系统 | spec | 核心机制 | 关键陷阱 |
|--------|------|---------|---------|
| comm-ports | R10 | 9 模块 .so + dlsym 动态加载，6 个 HAL 函数 | `comm_websocket_ex` 用 **noPoll**（非 libwebsockets） |
| publib | R7 | 线程/锁/队列/内存池/日志/DB + syshand 客户端 SDK | `iCfg_Parser.c` 是**死代码**（不编译、无引用） |
| system-hand | R3 | syshand 守护进程，System V IPC + 心跳 | 心跳丢失 180s 后**仅记录并继续服务**（`no reboot, no exit`）—— **按设计，非调试试态**；重启应用归未来 `app-supervision` |
| app-lifecycle | R6 | 构建→部署→运行时启动全流程 | `generate_verity.sh` 用**纯 Python**（非 veritysetup） |
| data-processor | R5 | DPR 数据中心，SQLite 配置 + 事件 + 告警 + 表达式 | 配置用 **SQLite System.cfg**（非 INI 文件） |
| samplers | R8 | 7 个采样器，4 个 DLLExport (Init/Sample/Control/JigTest) | 条件加载：IMMU3/SEMS/AEMS 跳过特定 ID |
| reporters | R8 | 8 个上报器，1 个 DLLExport (Initialize) | `nPlatformType` 决定互斥跳过 6/7/8/9 |

## HAL 通信接口（6 函数）

原型在 `src/include/comm_ports/comm_port.h:284-291`，9 个模块各通过 `#define Xxx_Comm* HAL_Comm*` 别名复用：

```c
HANDLE HAL_CommOpen(char*, char*, DWORD, int, int*);   // 打开
HANDLE HAL_CommAccept(HANDLE);                         // TCP/WS Server 接受连接
int    HAL_CommRead(HANDLE, char*, int);                // 读
int    HAL_CommWrite(HANDLE, char*, int);               // 写
int    HAL_CommControl(HANDLE, int, void*, int);        // 控制
int    HAL_CommClose(HANDLE);                           // 关闭
```

## 配置加载机制

**非 INI 文件**。通过 SQLite 数据库加载：
- 配置 DB: `/app/config/System.cfg`
- 实现: `src/moduals/data_processor/dpr/config_mangmt.c`
- 解析器: `Cfg_Parse_Sampler` / `Cfg_Parse_Reporter` / `Cfg_Parse_CommPort` / `Cfg_Parse_Device` 等
- 数据 DB: `$NCCU_DATA_PATH/nccu.data`（运行时 `/data/nccu.data`）

## syshand IPC 协议（客户端 SDK 在 publib）

两端通过相同的 IPC 配对：
```c
共享内存 key: _KEY_T_SYSHAND = ftok("/app", 15)
命令信号量:   /_semSysHandWait
应答信号量:   /_semSysHandAnsw
命令结构体:   CMD { iCmdType, szCmd[512], iCmdRet }
命令超时:     30000 ms (_SYSTEM_LOCK_WAIT_TIMEOUT)
```
iCmdType 分流：0=system() / 1=popen() / 99=心跳

## 已知技术债

1. **【已结案 2026-09-16，非技术债】** 原记载「`handler.c:123` `system("reboot -f")` 被注释禁用 — 调试试态，生产前必须取消注释」**已失效**：syshand 心跳丢失 180s 后**仅记录并继续服务**（`handler.c:118-124`，消息含 `no reboot, no exit`），该 reboot 调用在本分支**已被移除**（不是注释），仅 `:173` / `:215` 的其它分支留有注释形式。**既定方向：应用故障不重启整机**；重启应用的职责归未来 `app-supervision`（见 `keep-app-alive-on-bmu05` D6/D7-1）
2. **`iBasic_Funs.c` 混合性质**（2738 行）— 含 system-hand 客户端 SDK + OS 工具 + 业务常量，未拆分
3. **零测试代码** — 所有 Makefile `TEST_EXEC =`（空），全项目无单元测试
4. **死代码未清理**: `iCfg_Parser.c`（1012 行）、`modbustcp2_reporter/`
5. **签名服务器写死开发 IP** `192.168.34.207:8900`，生产环境需确认

## 构建产物验证

```bash
# 检查 magic
magic=$(dd if=pkg/app-*.img bs=1 skip=$((256*1024*1024 - 4096)) count=8)
echo $magic  # 应输出 D9APPMD1

# 检查 root hash
grep '^Root hash:' pkg/verity_info.txt

# 检查 ext4 block size
dumpe2fs -h pkg/app.ext4 | grep 'Block size'  # 必须为 4096
```

## 代码探索指南

```
找配置加载逻辑     → data_processor/dpr/config_mangmt.c (非 publib)
找通信模块加载     → comm_ports/comm_loader.c
找采样器加载       → moduals/samplers/sampler_loader.c
找上报器加载       → moduals/reporters/reporters_loader.c
找 syshand 客户端  → publib/iBasic_Funs.c (_SYSTEM / Do_Proc_HeartBeat)
找系统命令服务端   → utilities/system_hand/handler.c
找启动流程         → shells/app_entry → startrun → main/main.c
找 dm-verity 生成  → scripts/generate_verity.sh (纯 Python)
找签名逻辑         → scripts/sign_metadata.sh
找 RAUC 打包       → scripts/ota-bundle.sh
```
