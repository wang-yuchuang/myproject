## Why

`openspec/specs/system-hand/spec.md` 的两条 Requirement 与代码**相反**。其中 R1 会**诱导实施者做错事**。

### H1 — R1「主进程心跳监控」：把"仅记录并继续服务"描述成"调试态，生产前必须取消注释 reboot"

| 位置 | spec 现文 | 代码实测 |
|---|---|---|
| `:29` | 「心跳丢失时**恢复系统**」 | 仅记录，不恢复 |
| `:37` | 「阈值 … 触发**恢复**」 | 触发**告警（仅记录）** |
| `:39-40` | 「`handler.c:123` 的 `system("reboot -f")` 当前被注释禁用…**生产部署前必须取消注释**」 | `handler.c:123` 是 `nNoHeartBeatCounter = 0;`；该分支**不存在** reboot 调用（已**移除**，非注释）|
| `:42-44` | 「按「进程监管」方案，syshand … 将**移除**心跳和 reboot」 | 该演进**已完成**（reboot 职责已移除；启动/保活归 BSP `app_sysmgr`）|
| `:59` | 消息串 `"*****!!!Main APP is of NO heartbeat, WOULD reboot but disabled for debug!*****"` | `"Main APP heartbeat lost; keep serving as OS HAL (no reboot, no exit)"`（`handler.c:121`）|
| `:60` | 「生产态下执行 `system("reboot -f")`」 | 无此调用 |
| `:61` | 「… **`break` 退出 `Main_Loop`**」 | `handler.c:127` 是 **`continue`**，不退出 |

**危害**：任何按 spec 实施的人都会"生产前取消注释"，把整机重启加回来 —— 与既定方向「**应用故障不重启整机**」相反，并会推翻已上板验证的 4h25m 零重启结论。

### H2 — R5「PID 文件管理与进程生命周期」：把"标记文件"描述成"自锁"

`smain.c:6` 用 `fopen(_SYSHAND_PID_FILE, "w+t")` —— `"w"` 语义是**截断或创建**，**文件已存在同样成功**。故 `smain.c:7` 的 `fp == NULL` 分支**不可能因"进程已在运行"而触发**（只在路径不可写时触发）。`smain.c:5` 的原注释已写明该文件是**给 app 端脚本判断用的标记**。

| 位置 | spec 现文 | 代码实测 |
|---|---|---|
| `:210-212` | 「`fopen("w+t")` 成功 → 写 PID … **补充防护**：`run_app` 的 `ps \| grep -q '[s]yshand'`」 | PID 文件**不是自锁**；真正的防重复在 `run_app` 侧，且已改为 **`pidof syshand`**（`run_app:129` / `mu3/run_app:82`）|
| `:215` | 「从 `Main_Loop` 正常返回（**心跳超时 `break`** 或 IPC 初始化失败）」 | 心跳路径已不 `break`；仅致命 IPC 错误会退出 |

## What Changes

- `system-hand`：**MODIFIED R1 + R5**（共 **15 行**行内替换，其余逐字保留；块行数不变）
- **不改任何代码** —— 代码行为正确，错的是文档
- 不新增 Requirement；不重编号；不动 R2 / R3 / R4

## Capabilities

### New Capabilities

- (无)

### Modified Capabilities

- `system-hand`：R1「主进程心跳监控」改写为"仅记录并继续服务、不重启不退出"；R5「PID 文件管理与进程生命周期」改写为"PID 文件是标记而非自锁"

## Impact

- **文档**：`openspec/specs/system-hand/spec.md`（由 `openspec archive` 依据 delta 生成）
- **代码**：无变更
- **关联**：`keep-app-alive-on-bmu05` D7-1（移除 syshand 自退出）；`AGENTS.md` 与 `openspec/.code-facts-snapshot.md` 已于 2026-09-16 同步该方向，本变更是把 spec 也拉齐
- **风险**：低 —— 15 行文本，且块行数不变；以归档后 `git diff` **仅含这 15 处行内变更**作为验收判据（判据细节见已归档变更 `fix-app-lifecycle-script-refs` 的 tasks.md 3.7）
