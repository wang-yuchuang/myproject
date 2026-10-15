## MODIFIED Requirements

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
