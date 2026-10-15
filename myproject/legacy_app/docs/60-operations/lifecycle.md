# 应用启停（Start / Stop Runbook）

> **性质**：操作手册
> **需求权威源**：`openspec/specs/app-lifecycle/spec.md`（R7/R8/R9，另涉 R12/R13/R14）、`openspec/specs/system-hand/spec.md`（R1/R5）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`c48d724e`
> **状态**：🟠 部分

## 读前须知

本文写的是「今天实际能用的」启停做法。脚本注释里的设计意图与运行事实有出入，出入处用 ⚠️ 标出并给出代码位置。验收级断言不在此复制，需要时链到对应 spec 的 `R<n>`。

先记住两个事实：

1. **冷重启后没有任何进程会去启动应用**（boot 路径启动责任真空），详见[已知限制](#已知限制)。
2. **基于信号的停止链路不可达**：`/var/run/app.pid` 是死 PID（app-lifecycle R9），替代做法见[停止](#停止)。

## 前置条件

- 设备上有 root shell（串口或 SSH）。
- 确认平台变体：`cat /etc/hostname`。`MU_TYPE=mu3` 构建安装的是 `src/shells/mu3/*` 变体（app-lifecycle R12），bmu05 为 mu3；mu3 与非 mu3 的进程名差异见下文。
- 想停得干净，先拔掉 USB / SD 卡（原因见「启动」一节）。

## 启动

### 启动链全景

依据 app-lifecycle R7 与 `.code-facts-snapshot.md` 的 Startup chain（`:217-244`）：

```
S92app_mount（SDK 侧: 选槽 + 验 D9APPMD1 + dm-verity 挂载 /app 只读）
  └─ /app/app_entry /app &                  ← 唯一入口，boot 与 OTA 共用
       ├─ 幂等守卫: pidof nccu_app           (app_entry:21-24)
       ├─ 清理残留 PID: nccu.pid/syshand.pid (app_entry:50-51)
       ├─ 写 /var/run/app.pid                (app_entry:58)  ⚠️ 死 PID，见「停止」
       ├─ trap SIGTERM → stoprun             (app_entry:61)  ⚠️ 不可达，见「停止」
       └─ startrun
            ├─ USB/SD 升级介质检测            (startrun:52-53 / mu3/startrun:61-62)
            ├─ 检测到介质 → app_update        (startrun:99,105 / mu3/startrun:88,94)
            └─ /app/run_app                   (startrun:119 / mu3/startrun:104)
                 ├─ 环境变量 NCCU_RUN_PATH=/app、NCCU_DATA_PATH=/data (run_app:4-14)
                 ├─ mkdir /opt/appdata/{log,config,data} + 幂等复制默认配置 (run_app:50-54)
                 ├─ ./bin/syshand &           (run_app:136，PID 写入 run_app:141)
                 ├─ 等 /app/extensions/install.sh 完成 (run_app:148-165)
                 ├─ syshand 存活检查，失败则 exit 0 (run_app:168-179)
                 └─ ./bin/nccu_app -pid=$VAR_DIR/nccu.pid & (run_app:191)
                      └─ 等待 /opt/appdata/data/syshand.run，约 10.1s 超时 (main.c:314-326)
                      └─ Main_LoadModuals 6 步加载 (main.c:147-202，app-lifecycle R8)
```

nccu_app 以生产模式直接启动，无 strace 包装（app-lifecycle R13）。

### 谁在今天真的会启动它

| 路径 | 现状 | 依据 |
|---|---|---|
| OTA 安装 | **可用**。RAUC 安装脚本执行 `"$APP_POINT/app_entry" "$APP_POINT" &` | `app_entry:6-8`、`openspec/changes/keep-app-alive-on-bmu05/design.md:33` |
| 冷重启（boot） | **无人启动**。`S92app_mount` 只挂载不启动，其内部 `run_app()`（含启动代码 `:454-456`）定义后从未被调用；`S91app_sysmgr` v2 已移除启动 app_entry 的能力 | snapshot `:226-229`、`:220-224` |

冷重启后需要人工拉起：

```sh
/app/app_entry /app &
```

与 OTA 路径同一入口。app_entry 自带幂等守卫，重复执行安全（`app_entry:21-24`）。

### 升级介质检测的副作用

startrun 用 `/dev/sd*` 与 `/dev/mmcblk[1-9]*` 判断外置介质（`startrun:52-53`、`mu3/startrun:61-62`）。模式已排除内置 eMMC `/dev/mmcblk0`，这是 2026-07 的修正，此前 `ls /dev/mmc*` 会把 eMMC 误判成 SD 卡导致卡死在 app_update（app-lifecycle R7 注记，`spec.md:256`）。

插着 U 盘或 SD 卡上电会进入 `app_update`，不会起业务。要正常运行，上电前拔掉可移动介质。

### mu3 与非 mu3 变体

`MU_TYPE=mu3` 构建实际安装 `src/shells/mu3/*`。对任一变体的修复必须同步到另一个（app-lifecycle R12），本文在差异处分别标注行号。

## 停止

### ⚠️ 信号停止当前不可达（app-lifecycle R9）

`app_entry:61` 注册的 SIGTERM trap 设计上会调用 stoprun、删 PID 文件、退出。但 `app_entry:64` 调用 startrun 后脚本随即结束，syshand 与 nccu_app 都已被后台化，trap 随父 shell 一起消失。后果：

- `/var/run/app.pid` 里的 PID 是**死 PID**（`app_entry:58` 写入的进程已退出）；
- `S92app_mount stop`（读该 PID 发 SIGTERM）**停不掉任何业务进程**；
- `app_entry:14` 的注释「4. 注册 SIGTERM 信号处理器，优雅退出」与代码事实不符，spec R9 已记录此偏差（`spec.md:366-368`）。

**不要**依赖 `kill "$(cat /var/run/app.pid)"` 来停应用。

### 实际可行的手动停止

首选直接执行 /app 里的 stoprun。它是独立脚本，不依赖那个不可达的 trap：

```sh
/app/stoprun
```

它按顺序做四件事：

| 步骤 | 动作 | 非 mu3 证据 | mu3 证据 |
|---|---|---|---|
| 1 | `killall -9 app_update` | `stoprun:12` | `mu3/stoprun:6` |
| 2 | 杀 UI 进程：非 mu3 为 `nccu_ui_mm`，mu3 为 `nccu_ui` | `stoprun:13` | `mu3/stoprun:7` |
| 3 | `kill $(cat /opt/appdata/data/syshand.pid)` + 删 PID 文件 | `stoprun:20-25` | `mu3/stoprun:14-19` |
| 4 | `kill $(cat /opt/appdata/data/nccu.pid)` + 删 PID 文件 | `stoprun:27-32` | `mu3/stoprun:21-26` |

stoprun 本身不可执行时的等价手工序列：

```sh
killall -9 app_update
killall -9 nccu_ui_mm        # mu3 设备（如 bmu05）改用: killall -9 nccu_ui
kill "$(cat /opt/appdata/data/syshand.pid)" 2>/dev/null
rm -f /opt/appdata/data/syshand.pid
kill "$(cat /opt/appdata/data/nccu.pid)" 2>/dev/null
rm -f /opt/appdata/data/nccu.pid
```

排查「停不干净」时可参考 OTA 安装器的更强组合：`stop_app()` 杀掉 `/var/run/app.pid` 目标后执行 `pkill -9 app_entry syshand nccu_app`（app-post-install `:159-164`），再由 `close_verity_device()` 的 `fuser -k` 兜底释放设备（`app_entry:30` 注释、`openspec/changes/keep-app-alive-on-bmu05/design.md:55`）。

### 停止后验证与重启

```sh
pidof nccu_app; pidof syshand     # 两者均应无输出
ls /opt/appdata/data/*.pid        # 应无 syshand.pid / nccu.pid
```

重新启动就是再执行一次 `/app/app_entry /app &`。停止后 `/opt/appdata/data/syshand.run` 可能残留，无需处理：syshand 每次启动都会重建它（`handler.c:218`），app_entry 启动时也刻意不删它以免竞争（`app_entry:35-36`）。

## syshand 心跳丢失时的行为

监控机制：`Main_Loop` 以 5s 超时的 `sem_timedwait` 等命令，任何到达的命令（含 `iCmdType=99` 心跳专用包）都算存活信号；以 10s 为窗口计数，累计 18 次（约 180 秒）触发告警（`handler.c:114-125`，system-hand R1）。

告警动作只有一条：**写一条错误日志，然后继续服务**。

```
Main APP heartbeat lost; keep serving as OS HAL (no reboot, no exit)
```

- **不重启系统**：该分支没有 `system("reboot -f")`，调用已被移除（不是注释掉），见 `keep-app-alive-on-bmu05` D7-1（2026-09-16）；
- **不退出进程**：计数器归零后回到 `sem_timedwait` 循环，之后约每 180s 重复一条日志。

⚠️ 旧版根 README（以及 2026-07-02 版快照）曾表述为「心跳丢失则重启系统（reboot 被注释，生产前取消注释）」。**该说法已失效，且方向相反**：既定决策是「应用故障不重启整机」，把 reboot 恢复回来属错误操作。证据与演变见 `.code-facts-snapshot.md:179-187` 与 system-hand R1 注记。

## PID 文件、锁文件与 DB 文件

| 路径 | 内容 | 备注 |
|---|---|---|
| `/var/run/app.pid` | app_entry 的 PID | ⚠️ **死 PID**，app_entry 写完即退出（R9） |
| `/opt/appdata/data/syshand.pid` | syshand PID | stoprun 与 run_app 使用（`run_app:126,141`）；是**标记不是锁**：`fopen "w+t"` 为截断语义，文件已存在同样成功（system-hand R5，`spec.md:208-209`） |
| `/opt/appdata/data/nccu.pid` | nccu_app PID | 由 `-pid=` 参数写入（`run_app:125,191`） |
| `/opt/appdata/data/syshand.run` | syshand 就绪标记 | nccu_app 启动时轮询它（`main.c:314-326`） |
| `/opt/appdata/data/.syscmd_locker` | 系统命令 flock 目标 | 残留无害，勿删：unlink 会破坏跨进程互斥（`app_entry:52-54`） |
| `/opt/appdata/data/.{applog,bmsrundata,clouddata}.lock` | 三个 DB 的锁文件 | `iDataBase.c:34/39/44` |
| `/data/nccu.data` 等 | SQLite 数据库本体 | 见下方 ⚠️ |

⚠️ **数据库文件与锁文件不在同一目录**。`/opt/appdata` 是 bind mount，源为 `/data/rootfs/opt/appdata`，与 `/data` 顶层同分区但不同目录。四个 DB（`nccu.data`、`AppLog.data`、`BmsRunData.data`、`CloudData.data`）与 `loginfo.N` 都在 `/data/` 顶层，`/opt/appdata/data/nccu.data` 不存在。完整拓扑见 `.code-facts-snapshot.md` 的「运行时可写路径与 DB/锁文件」（`:100-114`）。

两个易踩的坑：

- `AppLog.data` 是 SQLite 库文件，不是文本日志，对它 `grep` 出的匹配数没有意义（snapshot `:114`）；
- 应用运行日志在 `/opt/appdata/log/trace.log`（snapshot `:108`，app-lifecycle R13）。

## OTA 安装路径上的启停

RAUC 安装 app bundle 时，`/usr/lib/rauc/app-post-install` 依次：

1. **停旧实例**：`stop_app()` 杀 `/var/run/app.pid` 指向的进程，执行 `pkill -9 app_entry syshand nccu_app`（`:159-164`），`close_verity_device()` 用 `fuser -k` 兜底；
2. **写新分区**（RAUC slot 安装本身）；
3. **拉起新实例**：`start_app()` 执行 `"$APP_POINT/app_entry" "$APP_POINT" &`，与 boot 路径共用同一入口（`app_entry:6-8`）。

这是当前唯一会在生产路径上「停止后又启动」应用的流程。反过来说：**除了 OTA，没有任何自动机制会（重新）启动应用**。

## 已知限制

- **boot 启动责任真空**：冷重启后 `S92app_mount` 只挂载不启动（其 `run_app()` 是死代码），app_sysmgr v2 也不再启动应用，唯一启动者是 OTA 安装。方案讨论见 `openspec/changes/keep-app-alive-on-bmu05/design.md`（D6/D7）。
- **停止链不可达**：SIGTERM trap 失效、`/var/run/app.pid` 为死 PID，现场只能用 stoprun 或手工 kill（上文「停止」）。修复方向（前台 supervisor 持有子进程并转发信号）同上链接。
- **无崩溃自愈**：nccu_app 崩溃后无人拉起；syshand 按设计不重启、不退出（system-hand R1）。
- **变体差异**：app-lifecycle R12 要求 mu3 与非 mu3 启停脚本行为一致；stoprun 的 UI 进程名差异（`nccu_ui_mm` vs `nccu_ui`）是 R9 Scenario 记录在案的已知差异，排查前先确认设备变体。
- 启动脚本若需改动，注意 R14：部署 `libnopoll.so` 时必须同时有 `libnopoll.so.0` 链接，否则 comm_websocket_ex 的 dlopen 会失败。

## 相关文档

- 需求：`openspec/specs/app-lifecycle/spec.md`（R7 启动链 / R8 初始化 / R9 停止 / R12 mu3 一致性 / R13 生产模式 / R14 libnopoll）、`openspec/specs/system-hand/spec.md`（R1 心跳 / R5 PID 文件）
- 代码事实基线：`openspec/.code-facts-snapshot.md`
- 变更设计：`openspec/changes/keep-app-alive-on-bmu05/design.md`
- 目录索引：[`docs/60-operations/README.md`](README.md)
