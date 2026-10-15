## MODIFIED Requirements

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
