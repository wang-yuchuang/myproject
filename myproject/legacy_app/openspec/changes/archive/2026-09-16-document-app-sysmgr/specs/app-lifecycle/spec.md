## ADDED Requirements

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
