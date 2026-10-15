## Why

`app_sysmgr`（由 `/etc/init.d/S91app_sysmgr` 拉起的 AP↔R 跨核系统管理守护）是一个**boot 关键、此刻正在运行**的 BSP 组件（bmu05 实测 PID 1003、4 线程、rpmsg fds），但在 `openspec/specs/` 下**零覆盖** —— grep `app_sysmgr` 的 19 处匹配**全部**落在 `openspec/changes/` 内。

这个空洞在 2026-09-16 一天之内**三次误导了判断**：

| # | 影响 |
|---|---|
| 1 | 它占用了 **S91** init 槽位，是原 `S91app_mount` 改名为 `S92app_mount` 的原因 —— 而改名残留导致 3 条 Requirement 长期写错脚本名（见已归档变更 `fix-app-lifecycle-script-refs`） |
| 2 | 它的 **v1 曾经负责启动 `/app/app_entry`**，v2 重写时移除了该能力 → 与 `S92app_mount.run_app()` 从未被调用叠加，造成「**冷重启后无人启动 app**」（见 `keep-app-alive-on-bmu05` D8） |
| 3 | 它有一套**独立于 syshand 的跨核心跳**（PING/PONG ↔ R 核），极易与 `system-hand` 的应用心跳（180s 无心跳）混淆 —— 两套心跳机制在 spec 里此前只有后者被记录 |

不补齐这一块，读者无法从 spec 回答三个基本问题：boot 顺序里 S91 是谁、谁负责启动应用、跨核心跳与应用心跳有何区别。

### 证据来源（2026-09-16 bmu05 真机取证）

| 类别 | 具体 |
|---|---|
| init 脚本 | `/etc/init.d/S91app_sysmgr`（1122 B）→ `"$APP" &`，`APP=/usr/sbin/app_sysmgr` |
| 二进制 | `/usr/sbin/app_sysmgr`（52624 B） |
| 进程 | PID 1003，4 线程；`fd 1,2 → /dev/console`；`fd 3-15` 全为 rpmsg（`/dev/rpmsg_ctrl0..3`、`/dev/rpmsg0..3`、sysfs `soc:ipcc@1.sysmgr.-1.500` / `virtio0.sysmgr.-1.500`） |
| 状态 | `S (sleeping)`，`wchan = hrtimer_nanosleep`（定时循环，非死锁） |
| 职责 | 全部摘自 `strings /usr/sbin/app_sysmgr` 的日志串（BOOT_READY×3 / SSDK 版本 / AP-R form / active slot / `MARK_GOOD` / heartbeat / `S00version-init --refresh`） |
| 能力移除 | v1 `/data/app_sysmgr`（69344 B）含 `/app/app_entry`、`Started app_entry (pid=%d)`；v2 `/usr/sbin/app_sysmgr` 对 `app_entry`/`nccu_app`/`syshand`/`run_app` 的匹配数均为 **0** |

## What Changes

- 在 `app-lifecycle` capability 下**新增 1 条 Requirement `R10`**「AP↔R 跨核系统管理守护（S91app_sysmgr / app_sysmgr，BSP 契约）」
- **纯新增**：不改动既有 R1-R9 的任何文字，不重编号
- **不改任何代码**：`app_sysmgr` 属 BSP 仓，本仓库只记录它对**应用启动链**的可观测影响（只读契约）
- 明确写入一项**否定性契约**：本守护 MUST NOT 承担启动 `/app/app_entry` 的职责（v2 起已无此能力），应用启动 owner 为 `S92app_mount`（boot）/ `app-post-install`（OTA）

## Capabilities

### New Capabilities

- (无)

### Modified Capabilities

- `app-lifecycle`：新增 `R10`，覆盖 `S91app_sysmgr` / `app_sysmgr` 的启动形态、四项职责（BOOT_READY 握手 / R 核信息查询 / RAUC `MARK_GOOD` / 跨核心跳）以及与 app 启动链的边界

## Impact

- **文档**：`openspec/specs/app-lifecycle/spec.md`（由 `openspec archive` 依据 delta 生成，将在 R9 之后追加 R10）
- **代码**：无变更
- **BSP 侧**：无变更（本变更不修改 BSP 行为，仅记录既有契约）
- **依赖**：无变更
- **关联**：`keep-app-alive-on-bmu05`（D8 记录「启动责任真空」的成因）、已归档 `fix-app-lifecycle-script-refs`（S91→S92 改名校正）、`openspec/.code-facts-snapshot.md`（`## app-sysmgr` 事实基线）
- **风险**：极低 —— 纯新增 Requirement，无既有内容改动
