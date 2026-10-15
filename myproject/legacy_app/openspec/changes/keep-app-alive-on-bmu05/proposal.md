## Why

在 bmu05 上，OTA 升级后应用**无法持续正常运行**，实测确认两个独立根因：

| # | 现象 | 根因（已实测/已定位） |
|---|---|---|
| 1 | **重启后应用不启动**（板子 up 12 小时、`pidof nccu_app` 为空） | **BSP 在途变更的迁移空档**：`add-system-mgmt-service` 的 **R1 已完成**（`S92app_mount` 的启动调用**按设计移除**，不得加回）、**R2 未交付**（`app_sysmgr` 门控启动的源码已就绪：`sysmgr_process.c:506 start_app()` / `:526 execl`，但 **Makefile 未编译**）。故当前仅 RAUC `app-post-install` 会在 OTA 时启动应用。⚠️ **不是“S92 忘了调用 `run_app()`”** —— 权威依据（BSP 的 `app-boot` spec R1/R2）见 `design.md` **D8** |
| 2 | **应用起来后每 ~5.5 分钟重启整机**（4/4 复现：12:11:10 / 12:36:16 / 12:50:57 / 13:05:2x） | CCU-RS232 采样器通信中断自愈：`ess_sampler_main.c:1388-1429` → `nCommFailTimes>=10` → 每 30 s 重开端口、计数 `>10` → `SendMsg(EVENT_TYPE_SYSTEM_EXIT)` → `event_manager.c:1663` 置 `bResetCmdPending` → `main.c:446` `_SYSTEM("reboot")`。台架无 CCU（`127.0.0.1:6690` 无监听）故必然触发 |

此外发现两个使问题更难排查的次生缺陷：

- **`ccu_sampler_main.c:446` 在初始化时把 `tmLastRecv_HEART` 预置为当前时间**，导致 `ess_sampler_main.c:1388` 的判定**无法区分"从未通信"与"通信中断"** —— 即"从未连过 CCU"也会被判定为"通信丢失"，这是根因 2 的直接机制。
- **`mu3/run_app:82` 的存活判定 `ps | grep -q '[s]yshand'` 不精确**：实测在 syshand 确实未运行时仍返回 MATCH（任何 argv 含 `syshand` 的进程都会命中，例如并发的 `rm -f .../syshand.pid`）。假阳性后 `:117` 判定 `bin/syshand` 不存在 → `:120 exit 0` → **应用永不启动**。

同时，BSP 侧已有一段**有价值但放错位置**的逻辑：`S92app_mount:run_app()` 会清理上次异常退出残留的 PID 文件。RAUC 的 `app-post-install` 路径虽然通过 `stop_app()` 的 `pkill -9 app_entry syshand nccu_app`（`:159-164`）与 `close_verity_device()` 的 `fuser -k` 把进程杀干净，却**不清理 PID 文件**；而 `mu3/run_app:136-139` 见到 `nccu.pid` 就 `exit 1` —— 于是每次 OTA/重启后都留下陈旧死 PID 文件，形成"装了起不来"的隐患。

## What Changes

- **A'｜单一入口收敛**：把 `app_entry` 做成**自包含的唯一入口**，吸收 BSP `run_app()` 中唯一真正有价值的"残留 PID/锁清理"，使 boot 与 OTA 两条路径行为一致
- **A''｜BSP 接线**（**BSP 侧依赖，不在本仓库实现**）：`S92app_mount` 的 `start)` 分支改为调用 `/app/app_entry /app`；其冗余的 `run_app()` 函数可随之删除
- **B｜CCU 重启门控**：为 CCU 通信中断自愈增加**只置位不复位**的"曾成功通信"标记，从未建立通信时仅告警不重启；参照同仓库 `kwhmeter_sampler` 已有的 `bLast_AllMeterCommOK` 模式
- **C｜存活判定精确化**：`mu3/run_app:82` 与 `run_app:129` 的 `ps | grep '[s]yshand'` 改为 **`pidof syshand`**（按进程名精确匹配，同时消除假阳性与子串类假阴性）；`app_entry` 顶部新增 `pidof nccu_app` 幂等守卫
- **D｜审核/排查发现的附加缺陷**（构建前一并修复）：① `handler.c:118-126` 心跳丢失不再让 syshand 自杀退出（避免级联到应用退出）；② `main.c:476` 传播 `nExitCode`（原恒返回 0，丢失故障原因）；③ `iBasic_Funs.c` `trace_fatal` 改为单次变参解析 + 格式串安全（原为 UB + 格式串漏洞）
- **明确不做**：恢复硬件看门狗（D9360 上 `/dev/wd` 不存在，且方向与"不重启整机"的既定决策冲突）、进程监管/自愈、`stoprun` 语义拆分、线程管理改造 —— 均留给后续 `app-supervision` 变更

## Capabilities

### New Capabilities

- (无)

### Modified Capabilities

- `app-lifecycle`: 修改 R7「运行时启动链」—— 收敛为单一入口 `app_entry`（BSP 与 RAUC 共同调用）、新增 liveness-aware 残留清理语义、存活判定要求进程名精确匹配（`pidof`）与 `app_entry` 幂等守卫
- `samplers`: 新增 R15「CCU 通信失败的系统重启门控」
- `publib`: 新增 R10「trace_* 追踪函数的格式化安全」
- `samplers`: 新增 R15「CCU 通信失败的系统重启门控」

## Impact

- **应用仓库代码**：`src/shells/app_entry`（单一入口 + `pidof nccu_app` 幂等守卫 + liveness-aware 残留清理）、`src/shells/mu3/run_app` 与 `src/shells/run_app`（各 1 行，改为 `pidof`）、`src/moduals/samplers/ccu_sampler/{ess_sampler_main.c, ccu_protocol_parsers.c, ccu_sampler_main.h}`（~4 行）、`src/utilities/system_hand/handler.c`（心跳丢失不再退出）、`src/main/main.c`（退出码传播）、`src/publib/iBasic_Funs.c`（`trace_fatal` 安全化）
- **BSP 侧（不在本仓库）**：`/etc/init.d/S92app_mount`（+3 行，可删约 25 行冗余函数）
- **运行时行为**：台架（无 CCU）不再重启整机；重启后应用能自启；OTA 与 boot 走同一入口
- **生产影响**：CCU 曾通信后中断的重启自愈保持不变，生产保护不削弱；未通信时的中断由"重启整机"降级为"告警"
- **依赖**：无新增外部依赖
- **验证**：需 bmu05 真机验证（见 `tasks.md` 第 4 组）
