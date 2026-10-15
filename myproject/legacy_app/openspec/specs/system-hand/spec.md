# Spec: system-hand — 系统守护进程

**Subsystem**: system-hand
**Version**: 0.4
**Created**: 2026-07-01
**Last Updated**: 2026-07-14

---

## Purpose

系统守护进程（syshand，可执行文件 `bin/syshand`），与 nccu_app 主进程并行运行。syshand 是 OS 系统调用的抽象层（HAL），核心职责：

1. **System V IPC 通道初始化**：创建共享内存段和双信号量，发布就绪标记供 nccu_app 检测
2. **BSP 扩展安装**：启动时运行 `/app/extensions/install.sh`，按需触发 reboot
3. **心跳监控**：等待 nccu_app 通过 IPC 发来的心跳/命令，超时后恢复系统
4. **系统命令代理**：接收 nccu_app 的 shell 命令请求，代为执行（避免 nccu_app 直接 fork）

源文件位于 `src/utilities/system_hand/`。客户端 SDK 位于 `src/publib/iBasic_Funs.c`（详见 [publib spec](../publib/spec.md) R7），两端通过相同的 IPC 协议配对。

**启动方式**：由 `src/shells/run_app`（或 `src/shells/mu3/run_app`）以 `./bin/syshand &` 后台启动。run_app 在启动后检查进程存活和 PID 文件，若 syshand 未存活则退出（不启动 nccu_app）。

---
## Requirements
### Requirement: R1 主进程心跳监控

系统 MUST 持续监控 nccu_app 主进程的心跳；心跳丢失时**仅记录并继续服务**，MUST NOT 重启系统、MUST NOT 退出进程。

**依赖**：POSIX（`sem_timedwait`）、[publib](../publib/spec.md) R7（客户端 `Do_Proc_HeartBeat`）

**监控机制**（`handler.c:93-130`）：
- `Main_Loop` 以 `sem_timedwait(psemWaitCmd, {now+5s})` 循环等待命令
- **任何**收到的命令（含 `iCmdType=99` 心跳专用）均视为存活信号，更新 `tmLastHeartBeat` 并重置 `nNoHeartBeatCounter = 0`
- 若 `sem_timedwait` 超时（5s 内无信号）且 `ABS(tmNow - tmLastHeartBeat) >= 10`，则 `nNoHeartBeatCounter++`
- **阈值**：`nNoHeartBeatCounter >= 18`（即约 **180 秒**无心跳）触发**告警（仅记录）**

> **行为已确认（2026-09-16，非调试试态）**：心跳丢失 180s 后 syshand **仅记录日志并重置计数器，继续服务**，该分支**不存在** `system("reboot -f")` 调用（已移除，非注释）。
> **MUST NOT 以"生产前取消注释"为由恢复整机重启** —— 既定方向为「应用故障不重启整机」；重启应用归上层保活机制（BSP `app_sysmgr`，见 [app-lifecycle R10](../app-lifecycle/spec.md)）。残留 `//system("reboot")` 仅在 `handler.c:173` / `:215` 其它分支。
>
> **架构现状**：syshand 已保留为 **OS HAL 抽象层**，其**系统修复（reboot）职责已移除**，心跳监控仅保留为**可观测性**手段。
> 应用进程的启动与崩溃重启由 BSP 的 `app_sysmgr`（AP↔R 跨核管理守护的保活机制）承担，见 [app-lifecycle R10](../app-lifecycle/spec.md)。
> 本仓不实现进程监管；如需变更启动 owner，走 BSP 的 openspec 变更。

#### Scenario: 正常心跳
- **WHEN** nccu_app 定期发送心跳信号（`iCmdType=99` 或任意命令）
- **THEN** `tmLastHeartBeat` 更新为当前时间，`nNoHeartBeatCounter` 重置为 0
- **AND** syshand 不触发恢复动作

#### Scenario: 心跳间隔计数
- **WHEN** 5 秒内无命令到达（`sem_timedwait` 超时返回非 0）
- **THEN** 检查 `ABS(tmNow - tmLastHeartBeat) >= 10`
- **AND** 若不足 10 秒，不计数（`nNoHeartBeatCounter` 不变），`continue` 下一轮
- **AND** 若超过 10 秒，`nNoHeartBeatCounter++` 并更新 `tmLastHeartBeat = tmNow`

#### Scenario: 心跳丢失恢复
- **WHEN** 连续 180 秒（18 × 10s）无心跳，`nNoHeartBeatCounter >= 18`
- **THEN** syshand 记录 `LOG_TYPE_ERROR`：`"Main APP heartbeat lost; keep serving as OS HAL (no reboot, no exit)"`
- **AND** 同时向 stdout 打印 `"[syshand] heartbeat lost; staying alive (reboot/exit disabled)"`，并重置 `nNoHeartBeatCounter = 0`（约每 180s 一条）
- **AND** `continue` 回到 `Main_Loop` 的 `sem_timedwait` —— MUST NOT 重启系统、MUST NOT 退出（`handler.c:118-127`）

#### Scenario: 客户端心跳节流
- **WHEN** nccu_app 调用 `Do_Proc_HeartBeat()`（`iBasic_Funs.c:262`）
- **THEN** 若距上次心跳 `ABS(tmNow - s_tmLastSendHeartBeat) < 10` 秒，直接返回不发 IPC
- **AND** 否则设置 `pCmd->iCmdType = 99`，`sem_post(psemWaitCmd)`，等待应答（最多 3000 × 10ms = 30s）
- **AND** 若 syshand 无应答，客户端记录 `LOG_TYPE_ERROR` 并 `raise(SIGUSR1)` 通知主进程

### Requirement: R2 BSP 扩展安装

系统 MUST 在进入 `Main_Loop` 之前检测并运行 BSP 扩展安装脚本。

**依赖**：`popen` / `pclose`（`stdio.h`）、`getenv`（`stdlib.h`）

**机制**（`smain.c:20-63`）：
1. `fopen(_EXTENSION_INST_FILE, "r")` 检测 `/app/extensions/install.sh` 是否存在
2. 若存在，`popen(_EXTENSION_INST_FILE " -no_reboot", "r")` 执行（传 `-no_reboot` 禁止脚本内部自启 reboot）
3. 逐行 `fgets` 读取 stdout，检测是否含 `reboot=1` 标志

#### Scenario: 无扩展安装
- **WHEN** `/app/extensions/install.sh` 不存在（`fopen` 返回 `NULL`）
- **THEN** syshand 直接进入 `Main_Loop`
- **AND** 不创建 `EXTENS_MARK_FILE`

#### Scenario: 扩展安装成功无需重启
- **WHEN** `install.sh -no_reboot` 执行完成且输出不含 `reboot=1`
- **THEN** syshand 正常进入 `Main_Loop`
- **AND** `run_app` 脚本等待 `install.sh` 进程退出（`ps | grep install.sh`，最多 50 × 100ms = 5s）后继续

#### Scenario: 扩展安装要求重启
- **WHEN** `install.sh` 输出含 `reboot=1`
- **THEN** syshand 执行清理序列：
  1. `unlink(_SYSHAND_PID_FILE)` 删除 PID 文件
  2. 读取环境变量 `EXTENS_MARK_FILE`，若存在则 `touch $EXTENS_MARK_FILE` 创建安装完成标记
  3. `system("reboot")` 重启系统
- **AND** syshand 进程 `return 1` 退出，不进入 `Main_Loop`

> **`EXTENS_MARK_FILE`**：由 `run_app` 设为 `/var/.ext_mark` 并 export。startrun 检测此文件存在则跳过 USB/SD 升级检测逻辑（表示刚完成 BSP 升级，应直接启动）。

### Requirement: R3 系统命令代理

系统 MUST 提供命令接口，供 nccu_app 等模块通过 IPC 发送 shell 命令，由 syshand 代为执行（避免 nccu_app 进程 fork 导致资源竞争）。

**依赖**：`system()` / `popen()` / `pclose()`（`stdlib.h` / `stdio.h`）

**命令分发**（`handler.c:141-154` 中 `switch(pCmd->iCmdType)`）：

| iCmdType | 处理函数 | 执行方式 | 返回值语义 |
|:--------:|----------|----------|-----------|
| 0 / default | `HandleCmd` | `system(pCmd->szCmd)` | `_SYS_EXE_OK=0`（成功） / `_SYS_EXE_NO_SCRIPT=1`（system 返回 -1） / `_SYS_EXE_FAIL=2`（非零退出） |
| 1 | `Handle_sh` | `popen(pCmd->szCmd, "r")` | 读取 stdout 首字节转 int（脚本用 `echo 0/1` 表达成败）；`popen` 失败返回 -1 |
| 99 | （内联） | 不执行命令，仅重置心跳 | 固定返回 0 |

**应答机制**：syshand 将 `iCmdRet` 写入共享内存的 `pCmd->iCmdRet`，`shmdt` 分离后 `sem_post(psemAnswer)` 通知客户端。

> **注意**：命令应答超时的客户端行为（`_SYSTEM`/`_SYSTEM_EX` 的阻塞等待和超时逻辑）由 [publib](../publib/spec.md) R7 定义，本 spec 仅描述 syshand 服务端的行为。
#### Scenario: 普通命令执行（iCmdType=0）
- **WHEN** nccu_app 通过 `_SYSTEM("reboot")` 发送命令（`iBasic_Funs.c:371`）
- **THEN** syshand `HandleCmd` 调用 `system()` 执行
- **AND** 通过 `WIFEXITED` / `WEXITSTATUS` 解析退出码：0 → `_SYS_EXE_OK`，非零 → `_SYS_EXE_FAIL`
- **AND** `system()` 返回 -1 → `_SYS_EXE_NO_SCRIPT`
- **AND** 客户端阻塞等待 `sem_trywait(psemAnswer)`（超时 30000ms，见 publib R7）

#### Scenario: 脚本命令执行（iCmdType=1）
- **WHEN** nccu_app 通过 `_SYSTEM_EX("hostname | grep -c a02")` 发送命令（`iBasic_Funs.c:501`）
- **THEN** syshand `Handle_sh` 用 `popen("r")` 执行
- **AND** `fread` 读取 stdout 的**首字节**（1 char），转为 int 返回（`buf - '0'`）
- **AND** 客户端直接使用 `pCmd->iCmdRet` 作为返回值（不做 OK/FAIL 转换）

#### Scenario: 心跳上报（iCmdType=99）
- **WHEN** nccu_app 发送 `iCmdType=99`
- **THEN** syshand 不执行任何命令，仅更新 `tmLastHeartBeat`（R1 心跳监控）
- **AND** 返回 `iCmdRet = 0`

#### Scenario: 命令无应答超时
- **WHEN** syshand 进程崩溃或卡死，客户端 `sem_trywait(psemAnswer)` 在 3000 次重试（30s）后仍无应答
- **THEN** 客户端记录 `LOG_TYPE_ERROR`：`"CMD[%s] NO answer, need restart system."`
- **AND** `raise(SIGUSR1)` 向 nccu_app 主进程发送信号（由主进程信号处理器决定后续动作）

### Requirement: R4 System V IPC 通道初始化

系统 MUST 在启动时建立与 nccu_app 之间的双向 IPC 通道（共享内存 + 双命名信号量），并在初始化完成后发布就绪标记。

**依赖**：POSIX IPC（`sem_open` / `sem_post` / `sem_timedwait`）、System V IPC（`shmget` / `shmat` / `shmdt`）

**IPC 资源**（两端定义相同，见 `handler.c:14-16` 与 `iBasic_Funs.c:44-46`）：

| 资源 | 标识 | 创建方 |
|------|------|--------|
| 命令信号量 | `/_semSysHandWait`（`sem_open`, O_CREAT, 初始值 0） | syshand（`InitThis`） |
| 应答信号量 | `/_semSysHandAnsw`（`sem_open`, O_CREAT, 初始值 0） | syshand（`InitThis`） |
| 共享内存 | `shmget(ftok("/app", 15), 1024, 0666 \| IPC_CREAT)` | syshand（`InitThis`） |

**命令结构体**（`handler.h:25-31`）：

```c
#define MAX_CMD_CHARS   512
#define CMD_MEM_SIZE    1024
struct _sCmdItem {
    int  iCmdType;              // 命令类型：0=system, 1=popen, 99=heartbeat
    char szCmd[MAX_CMD_CHARS];  // 命令字符串
    int  iCmdRet;               // 返回值（syshand 填写，客户端读取）
};
typedef struct _sCmdItem CMD;
```

共享内存大小 `CMD_MEM_SIZE = 1024` 字节，大于 `sizeof(CMD)` 以留余量。

#### Scenario: IPC 资源创建
- **WHEN** syshand 启动进入 `InitThis()`（`handler.c:186`）
- **THEN** `sem_open(_SEM_CMDWAIT_PATH, O_CREAT, 0666, 0)` 创建命令信号量
- **AND** `sem_open(_SEM_ANSWER_PATH, O_CREAT, 0666, 0)` 创建应答信号量
- **AND** `shmget(_KEY_T_SYSHAND, CMD_MEM_SIZE, 0666 | IPC_CREAT)` 创建共享内存段
- **AND** 共享内存内容被 `memset(p, 0, CMD_MEM_SIZE)` 清零

#### Scenario: IPC 初始化失败
- **WHEN** `sem_open` 返回 `SEM_FAILED` 或 `shmget` 返回 `-1`
- **THEN** `InitThis()` 返回 `FALSE`，`Main_Loop` 的 `bIsRun = FALSE`
- **AND** while 循环不执行，syshand 进程退出
- **AND** `run_app` 检测到进程未存活，不启动 nccu_app

#### Scenario: 就绪标记发布
- **WHEN** `InitThis()` 成功完成所有 IPC 资源创建
- **THEN** syshand 执行 `system("touch /opt/appdata/data/syshand.run")`（`handler.c:220`）
- **AND** 记录 `LOG_TYPE_MILESTONE` 日志 `"****SysHand process is up now!*******"`
- **AND** nccu_app 启动时检查此标记文件存在后才继续（见 `src/main/main.c:316`：`access("/opt/appdata/data/syshand.run", R_OK)` 轮询，超 10 × 100ms = 1s 则退出）

#### Scenario: 客户端懒初始化配对
- **WHEN** nccu_app 首次调用 `_SYSTEM` / `_SYSTEM_EX` / `Do_Proc_HeartBeat` 时 IPC 句柄为空
- **THEN** `InitSysHandler()`（`iBasic_Funs.c:166`）以相同 key 打开已存在的 IPC 资源（`shmget` 不带 `IPC_CREAT`）
- **AND** 创建 `g_hMutexSystemHandler` 互斥锁保护命令通道
- **AND** 打开文件锁 `/opt/appdata/data/.syscmd_locker`（`flock` 互斥）
> **双锁层级**（`iBasic_Funs.c`）：客户端使用两层锁保护命令通道，层级互补：
> 1. **Mutex_Lock（`g_hMutexSystemHandler`）**：进程内互斥锁，序列化 nccu_app 单进程内多线程对共享内存命令通道的访问。不同调用方有不同超时：心跳（`Do_Proc_HeartBeat`）= 100ms（`_LOCK_HEARTBEAT_WAIT_TIMEOUT`），`_SYSTEM` / `_SYSTEM_EX` = 30000ms（`_SYSTEM_LOCK_WAIT_TIMEOUT`）。
> 2. **flock（`g_fd_lockerFile` → `/opt/appdata/data/.syscmd_locker`）**：跨进程文件锁，防止多个进程同时向 syshand 发送命令。使用 `LOCK_EX | LOCK_NB`（非阻塞）+ 重试循环（300 × 10ms ≈ 3s 有效超时），由 Mutex 持有期间在循环中尝试获取。
>
> 两者嵌套：先 Mutex_Lock（进进程）→ 再 flock（跨进程）→ 操作共享内存 → flock UN → Mutex_Unlock。

### Requirement: R5 PID 文件管理与进程生命周期

系统 MUST 通过 PID 文件防止重复启动，并在进程退出时清理。

**依赖**：POSIX（`fopen` / `unlink` / `chmod`）

**PID 文件路径**（`handler.h:17-20`）：
- 有 Flash 环境（`_HAS_FLASH_MEM`）：`/opt/appdata/data/syshand.pid`
- 无 Flash 环境（x86 测试）：`./syshand.pid`

#### Scenario: 防重复启动
- **WHEN** syshand 启动时以 `fopen(_SYSHAND_PID_FILE, "w+t")` 打开 PID 文件
- **THEN** 写入当前 PID（`fprintf(fp, "%d\n", getpid())`），`chmod 0777`；⚠️ `"w"` 语义是**截断或创建**，**文件已存在同样成功** —— 故 `smain.c:7` 的 `fp == NULL` 分支**不可能因"进程已在运行"而触发**（仅路径不可写时触发）。该文件是**供 app 端脚本判断的标记**，**不是自锁**
- **AND** 真正的防重复启动在 `run_app` 侧：启动前用 `pidof syshand`（按 `/proc/<pid>/comm` 精确匹配）检查，若已存在则不重复启动

#### Scenario: 正常退出清理
- **WHEN** syshand 从 `Main_Loop` 返回（**仅致命错误**如 IPC 初始化失败；心跳丢失路径已不退出）
- **THEN** `smain.c:71` 执行 `unlink(_SYSHAND_PID_FILE)` 删除 PID 文件
- **AND** 进程 `return 0` 退出

#### Scenario: run_app 存活检查
- **WHEN** `run_app` 启动 syshand 前用 `pidof syshand` 检测到进程已存在（`run_app:129` / `mu3/run_app:82`）
- **THEN** 不重复启动，确保 `syshand.run` 标记存在（`test -e || touch`）
- **WHEN** 启动后 `ps | grep bin/syshand` 检测到进程不存在
- **THEN** `run_app` 打印 `"syshand NOT alive, exiting!"` 并 `exit 0`，不启动 nccu_app
> **mu3 与非 mu3 变体差异**：`run_app`（非 mu3）在启动 syshand 后检查**双重**条件——进程存活（`ps | grep bin/syshand`）**和** PID 文件存在（`if ! [ -f $SYSHAND_PID_FILE ]`），任一失败则 `exit 0`（`run_app:174-179`）。`mu3/run_app` **仅检查进程存活**，PID 文件检查被注释禁用（`mu3/run_app:126-132`），并在日志中明确输出 `"syshand alive, skipping PID file check (process-based)"`（`mu3/run_app:122`）。

#### Scenario: install.sh 重启前 PID 清理
- **WHEN** BSP 扩展安装输出 `reboot=1`
- **THEN** `smain.c:50` 先 `unlink(_SYSHAND_PID_FILE)` 再 `system("reboot")`
- **AND** 确保 reboot 后系统不会因残留 PID 文件阻止 syshand 重新启动

## White-box Test Ready

❌ 否——依赖进程管理、System V IPC 和心跳检测时序，更适合集成测试或硬件验证

---

## Key Files

| 文件 | 说明 |
|------|------|
| `src/utilities/system_hand/smain.c` | **入口**：写 PID 文件、运行 install.sh、调用 `Main_Loop`、退出时清理 PID |
| `src/utilities/system_hand/handler.c` | **核心**：`Main_Loop` / `InitThis` / `HandleCmd` / `Handle_sh`，心跳监控与命令分发 |
| `src/utilities/system_hand/handler.h` | `CMD` 结构体 / `CMD_MEM_SIZE=1024` / `MAX_CMD_CHARS=512` / `_SYSHAND_PID_FILE` / `_EXTENSION_INST_FILE` / 错误码 `enum _eErrRet` |
| `src/utilities/system_hand/smain.h` | 仅 `#include <unistd.h>`（空头文件） |
| `src/shells/run_app`（及 `src/shells/mu3/run_app`） | 以 `./bin/syshand &` 启动守护进程，检查存活状态和 PID 文件，等待 install.sh 完成 |
| `src/publib/iBasic_Funs.c` | **客户端 SDK**（`_SYSTEM` / `_SYSTEM_EX` / `Do_Proc_HeartBeat` / `InitSysHandler`），详见 [publib spec](../publib/spec.md) R7 |
