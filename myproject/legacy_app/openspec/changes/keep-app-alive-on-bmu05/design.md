## Context

`legacy_app` 在 bmu05 上的部署现状：OTA 安装能跑通（RAUC `app-post-install` → `start_app` → `app_entry` → `startrun` → `run_app` → `syshand` + `nccu_app`），但**boot 路径完全没有启动入口**，且应用会因 CCU 通信中断而主动重启整机。本轮目标是在**不做大改**的前提下让应用"装得上、起得来、跑得住"。

涉及的三个既有实现：

| 实现 | 位置 | 现状 |
|---|---|---|
| `app_entry`（app 仓库 shell） | `src/shells/app_entry` | 只做 PID 写入 + SIGTERM trap + 调 `startrun` |
| `S92app_mount:run_app()`（BSP shell 函数） | RO rootfs | 定义了但从不调用；内含残留清理 + 残留无效的目录/配置拷贝 |
| `mu3/run_app`（app 仓库 shell） | `src/shells/mu3/run_app` | 设置环境、起 syshand、起 nccu_app、存活检查 |

> **2026-09-16 真机补充调查**："boot 路径没有启动入口"的**成因**已查明 —— 该职责原由 BSP 的 `app_sysmgr` v1 承担，v2 重写时被移除，而 `S92app_mount.run_app()` 从未被接线。完整证据、`app_sysmgr` v2 的职责界定、以及对 D1 与任务 3.x 的影响，见 **D8**。

## Goals / Non-Goals

**Goals**
- boot 与 OTA 两条路径都通过**同一个** `app_entry` 启动应用
- 台架（无 CCU/无外围设备）上应用能持续运行，不再重启整机
- 保留生产环境的 CCU 失联重启自愈能力
- 改动可量化、可回滚、可在真机验证

**Non-Goals**
- 不引入进程监管 / 自动重启 / A/B 热切（留给 `app-supervision`）
- 不恢复硬件看门狗（D9360 无 `/dev/wd`）
- 不拆分 `stoprun` 语义、不改线程管理
- 不清理 `System.sql` 中"声明但未编译"的采样器行（已知隐患，另立变更）

## Decisions

### D1：`app_entry` 作为唯一入口，而非在 BSP 里各写一份

`app-post-install` 已经直接调用 `app_entry`（`"$APP_POINT/app_entry" "$APP_POINT" &`），而 boot 路径走的是 BSP 内部的 `run_app()`。两份逻辑对残留 PID 的处理不一致，是"有时起不来"的温床。

把逻辑收敛到 `app_entry` 的收益：
1. 两条路径行为一致，消除不一致缺陷
2. `app_entry` 位于 `/app`，可随 **app OTA** 演进，不需要为每次启动链调整做 rootfs OTA
3. BSP 侧净减代码（删掉冗余函数）

*替代方案*：在 `S92app_mount` 里补齐逻辑。否决 —— 会把启动链知识锁在 RO rootfs，且继续维持两条路径。

### D2：只吸收"残留清理"，明确丢弃残留无效逻辑

BSP `run_app()` 含四段逻辑，逐段判定：

| 逻辑 | 处置 | 依据 |
|---|---|---|
| `rm -f` 残留 PID/锁标记 | **吸收，但排除 `syshand.run`** | 唯一真正有价值的段；`mu3/run_app:136-139` 会因残留 `nccu.pid` 而 `exit 1`。**`syshand.run` MUST NOT 删除**：`mu3/run_app:87` 仅在"syshand 已在运行"分支才重新 `touch` 它，而 `app_entry` 先于 `run_app` 执行；若在 syshand 存活时删掉，`main.c:316` 的就绪等待会失败 |
| `mkdir -p /data/app/{config,log}` | **丢弃** | 实测 `/data/app` 不存在，且无任何代码读取 |
| `cp -rn /app/config/. /data/app/config/` | **丢弃** | 同上；与 `/app/run_app:32` 的 `cp -rn /app/config/* /opt/appdata/config/` 重复且目的地冲突 |
| 调用 `app_entry` + 3 s 健康检查 | **不搬** | 那是"调用者"的职责，保留在 BSP 侧一行 |

### D3：残留清理必须是 liveness-aware，而非盲删

boot 路径下进程确实已终止，盲删等价；OTA 路径同样如此 —— `app-post-install` 的 `stop_app()` 除终止 `/var/run/app.pid` 指向的进程外，还显式执行 `pkill -9 app_entry syshand nccu_app`（`:159-164`），并由 `close_verity_device()` 的 `fuser -k` 兜底。因此**两条路径到达 `app_entry` 时进程都已死亡**，PID 文件必然是陈旧死 PID。

因此：对 `nccu.pid` / `syshand.pid` **先 `kill -0` 并按 `/proc/<pid>/comm` 核对进程名，仅在确非目标进程时删除**（防止整机重启后 PID 回收复用造成误判；也为将来"应用仍在运行时调用本脚本"的场景保留防护）；`/var/volatile/*.pid` 为纯运行时产物仍可盲删。**`.syscmd_locker` 不删除** —— `flock` 随持有者退出由内核自动释放，残留文件不会阻塞后续加锁；而 unlink 会让新进程在新 inode 上加锁，与仍存活的旧持有者形成各自持锁，破坏跨进程互斥。

### D4：CCU 门控采用"只置位不复位"的闩锁，照抄同仓库既有模式

`kwhmeter_sampler` 已经用正确的方式解决了同一个问题：

```
kwhmeter.c:261                            bLast_AllMeterCommOK = FALSE;   ← 初始
rs485_whim_sampler_main.c:334/515/720     bLast_AllMeterCommOK = TRUE;    ← 通信 OK 后置位（不复位）
rs485_whim_sampler_main.c:1107            if (bLast_AllMeterCommOK == FALSE) return FALSE;  ← 门控
```

这也解释了为何 kWh 那条 `>=1200` 的自动重启在台架上未触发（实测 4/4 重启均来自 CCU 路径）。

对 CCU 施加同样模式：在 `ROUGH_CCU_DATA` 新增结构体字段 `BOOL bEverRecvHeart;`（与 kwhmeter 的 `bLast_AllMeterCommOK` 同构，见 `rs485_whim_sampler_main.h:436`），在**唯一真实接收点** `ccu_protocol_parsers.c:752` 置位，并在 `ess_sampler_main.c:1420` 的回触发条件前置该标记。

> **实现位置与两个必须避开的地雷**：经两轮独立审阅后选定**结构体字段**（而非文件静态），理由是遵循同仓库既有约定 —— kwhmeter 的 `bLast_AllMeterCommOK` 即为结构体字段（`rs485_whim_sampler_main.h:436`）加显式初始化（`kwhmeter.c:261`）。前提与约束：
>
> 1. **初始化位置**：结构体在 `Initialize()` 中已被整体清零（`ccu_sampler_main.c:429 ZERO_POBJS(pCCU, 1)`），字段天然为 FALSE，与语义一致。
> 2. **MUST NOT 在 `InitCCUStuffData` 中清零**：该函数是"局部状态复位"，且会在通信失败分支（`ess_sampler_main.c:1404`）与另一次调用（`:1543`）被重复执行。若把闩锁纳入其清零范围，"只置位不复位"会被静默破坏 —— 这正是必须写进 R15 的 MUST NOT。
> 3. **线程安全**：该字段与既有的 `strHEARTInfo.tmLastRecv_HEART` 是同一读写模式（同一条采样线程内写与读），无需额外同步。

> 注意**不能**复用 `tmLastRecv_HEART != 0` 之类的判据：`ccu_sampler_main.c:446` 在初始化时就把它预置为 `_NOW_`，无法表达"从未通信"。

*替代方案对比*：
- 抬高阈值（`>10` → 极大值）：1 行，但**生产保护一并失效**
- 直接注释 `SendMsg`：2 行，同样丢失生产保护
- 从 `System.sql` 删除 Sampler_ID=1 行：不改 C 代码，但会丢失全部 CCU/PCS/BMS 数据与相关设备映射，副作用过大
- **选用**：闩锁方案 —— 4 行，台架不重启、生产保护保留

### D5：存活判定改用 `pidof` 精确匹配

`ps | grep '[s]yshand'` 匹配的是**任意 argv 的子串**。bmu05 实测：syshand 确实未运行时该判定仍返回 MATCH（假阳性）—— 任何 argv 含该子串的进程都会命中，例如并发的 `rm -f .../syshand.pid`；插入 `tail -f .../syshand.pid` 诱饵后同样 MATCH。假阳性会跳过 syshand 启动，随后存活检查判定失败并以 `exit 0` 终止 → **应用永不启动**。

因此改为 **`pidof syshand`**（按 `/proc/<pid>/comm` 精确匹配进程名，无子串类误判，同时避免引入子串类的假阴性）。bmu05 实测：syshand 运行时 `pidof syshand` 正确返回其 PID（早期监控日志 `sysh=[1416]` / `[2692]`），未运行时返回空。同文件 `:117` 与非 mu3 `:168` 仍保留 `ps | grep bin/syshand`（带 `grep -v grep`，无自匹配问题，且已在生产运行），保持最小改动。

### D6：不恢复硬件看门狗（明确排除 8.7）

任务 `fix-legacy-app-runtime-issues` 的 8.7「恢复看门狗/心跳重启机制」在本变更中**不执行**：
- `/dev/wd`（major 241/236）是旧 IMAU/IMMU2 BSP 的字符设备，**D9360 上不存在**（实测只有 `/dev/watchdog1..4`，major 245，Semidrive *virtual* watchdog 且 state=inactive）
- 照抄恢复会 `open()` 失败 → `s_hWatchDog=-1` → 看门狗实际未启用，属"假绿"
- 方向与既定决策（应用故障不重启整机）冲突，应由后续 `app-supervision` 承接

## Risks / Trade-offs

| 风险 | 缓解 |
|---|---|
| 未建立 CCU 通信时的中断不再重启 → 掩盖"CCU 真故障" | B 的配套要求：该场景 MUST 产生告警（保留可观测性），仅取消重启动作 |
| 新增 boot 启动 owner，将来接入 guardian 时可能重复启动 | `app_entry` 顶部已加 `pidof nccu_app` 幂等守卫（覆盖 boot/OTA 全部调用方，且随 app OTA 一起更新，不必等 BSP）；guardian 变更中再统一 owner 语义 |
| `app_entry` 改动影响 OTA 路径（原本可正常工作） | 清理逻辑为幂等 `rm -f` / `kill -0` 判定，对不存在目标为 no-op；验证清单含 OTA 路径回归（4.1） |
| 残留清理误删存活进程的 PID 文件 | D3 的 liveness-aware 判定 |
| `handler.c:118-125`：syshand 在 180 秒无心跳后 `break` 退出（reboot 被注释） | **本变更未覆盖**（超出"最小改动"范围）。实测未触发：应用主循环每轮发送心跳，syshand 存活超过 337 秒。潜在级联：主循环真停滞 180 秒 → syshand 退出 → `_SYSTEM` 无应答 → `iBasic_Funs.c:361 raise(SIGUSR1)` → `sig_handler.c:165 Main_Exit` → 应用退出。建议在 `app-supervision` 中一并处理 |
| 审阅意见："`inpcs_sampler_main.c:116-125` 也有独立重启门（阈值 `>4`、无闩锁），故 B 打错了位置" | **已证伪，非风险**：`ccu_sampler/Makefile:20` 将该组 `.o` 整体注释（`# OBJS += inpcs_sampler_main.o inpcs_statemachine.o inpcs235kw_statemachine.o`）；部署镜像中 `grep -a -c "inpcs" /app/sampler/sampler_ccu.so` = **0**；且实测重启间隔 337 s 与 ESS 路径（~10 s + 11×30 s + 排空）吻合，与 InPCS 路径（阈值 `>4` → ≈160 s）不符。故此条不成立 |

## Migration Plan

1. 应用侧改动合并 → `./build.sh` → RAUC bundle
2. `rauc install` 到 bmu05 → 验证 OTA 路径回归 + 持续运行（`tasks.md` 4.1–4.3）
3. 提交 BSP 侧接线说明（`tasks.md` 3.x）→ rootfs OTA → 验证 boot 路径（4.4–4.5）
4. 全部通过后 `openspec archive keep-app-alive-on-bmu05`

## Open Questions

- BSP 是否接受删除 `S92app_mount` 的 `run_app()` 函数，还是先保留一个版本作为兼容包装？
- 未通信时的 CCU 中断告警应挂在哪个既有告警 ID 上（复用通信中断告警，还是新增）？
- `app_supervision` 的范围应如何界定？（D8）`app_sysmgr` **v1** 已有 `/var/run/sysmgr.sock` + 启动 `app_entry` 的实现，v2 把这两项删掉、只保留跨核功能。因此该任务不必从零设计 —— 是「在 v2 框架上补回监督能力」还是「另起独立 supervisor」，需要决策
- BSP 是否会（或已经计划）把启动 `app_entry` 的能力加回 `app_sysmgr`？（D8）若会，则本变更的 boot 接线与 `app_sysmgr` 将成为**两个启动者**，`app_entry` 的 `pidof nccu_app` 幂等守卫是唯一防线；需与 BSP 对齐 owner 语义

## D7：审核与排查中发现的附加缺陷修复（构建前）

三处与"应用能否持续运行"直接相关的缺陷，虽超出最初的最小集，但修复成本极低且风险可控，故在构建前一并处理：

### D7-1 `handler.c`：syshand 不再自杀

原实现：心跳丢失达 `18 × 10s` 时打印 "WOULD reboot but disabled" 后 **`break` 退出 `Main_Loop`**（`handler.c:125`）。syshand 一旦退出，其作为 OS HAL 的命令通道即失效 → 应用侧 `_SYSTEM` 无应答 → `iBasic_Funs.c:361 raise(SIGUSR1)` → `sig_handler.c:165 Main_Exit(SYS_EXIT_BY_FAULT)` → **应用退出**。这是一条"syshand 死 → 应用死"的级联，与"应用持续运行"目标直接冲突。

修复：去掉 `break`，保留日志（约每 180s 一条）、重置计数器、继续服务。与既定决策"syshand 保留为 OS HAL、移除心跳/reboot"方向一致。

### D7-2 `main.c`：传播退出码

原实现 `main()` 恒 `return 0`（`main.c:476`），`longjmp` 出来的 `nExitCode` 被丢弃 → 外部无法区分"正常停止"与"故障退出"（也堵死未来 supervisor 的判断依据）。

修复：`return s_pChargeThis->nExitCode;`。`run_app` 后台启动应用且不检查 `$?`，故对现有行为无影响。

### D7-3 `iBasic_Funs.c`：`trace_fatal` 的两处内存安全问题

`trace_fatal`（`iBasic_Funs.c:2634-2652`）存在两个真实缺陷，且被**活跃代码**调用（`ccu_com_rpmsg.c:64` 的 `TRACEF`）：

1. **`va_list` 复用（UB）**：先 `vprintf(format, args)`，随后用同一个 `args` 调 `vsnprintf`。`va_list` 已被消费；aarch64 上它是结构体，二次读取会越界。
2. **格式串漏洞**：`LogOut(..., szBuff)` 将消息缓冲区当格式串传入；消息中出现 `%s` / `%n` 会被二次解释（可致崩溃或任意内存写）。

修复：合并为单次 `vsnprintf` 解析，改 `LogOut(..., "%s", szBuff)`；可见输出（`trace_header` → 消息 → `trace_tail`）保持不变。对应新增 requirement `publib` R10。

### 记录但**不**修复

- `main.c:427-449`：事件繁忙时 `continue` 会跳过 `bResetCmdPending` 检查，可能**饿死** reset。仅"延迟"不"取消"，且修它会改变 R15 承诺"保持不变"的生产行为 → 不改，仅记录。
- `app_entry` 的 SIGTERM trap 仍不可达（`app_entry` 在 `startrun` 返回后即退出，而 `run_app` 后台化进程），且 `S92app_mount stop` 读到的 `/var/run/app.pid` 是 `app_entry` 的死 PID → **停止链路不可用**。真正解决需要前台 supervisor，留待 `app-supervision`。
- 其它 `EVENT_TYPE_SYSTEM_EXIT` 发起者（固件升级、控制类 reset/restore）为命令/外部驱动，台架不触发；且它们是**有意设计**的整机重启，不应门控。
- `main.c:316-326`：`syshand.run` 就绪等待已由 1.1s 延长到 10.1s（本次已修）—— `run_app` 的 `pidof syshand` 存活检查可能先于 syshand 写完就绪标记通过，原 1.1s 在高负载启动时会误判退出。
- `iBasic_Funs.c:361/492/627`：syshand 无应答时 `raise(SIGUSR1)` → `Main_Exit`（应用退出）。**不修**：若改为仅记日志，`_SYSTEM` 每次仍会阻塞至多 30s 等待应答，应用会从「死」变成「僵」，反而更差；正解是由 supervisor 重启 syshand。
- `iThread.c:787` → `main.c:57/77/459`：任一业务线程 3 次心跳丢失即整应用退出。**不修**：会掩盖真实线程卡死；属 supervision 范畴。

## D8：启动责任「真空」的成因（2026-09-16 真机补充调查）

本节补上 `Context` 缺失的一环：**为什么** boot 路径没有启动入口。它把 D1 与任务 3.x 的理由从"BSP 忘了调用 `run_app()`"升级为一个有实证的机制性结论。

### 三方比对

| 候选启动者 | 是否拉起 `app_entry` | 证据 |
|---|---|---|
| `S92app_mount.run_app()` | **代码在，调用点 0 处** | 定义于 `/etc/init.d/S92app_mount:436`；函数体 `:454-456` 即 `"$APP_POINT/$APP_INTERFACE" "$APP_POINT" &`；全文件无调用点 |
| `app_sysmgr` **v1**（`/data/app_sysmgr`，2025-07-29，69344 B） | **会** | `strings` 含 `/app/app_entry`、`Started app_entry (pid=%d)`；日志实证 `[2025-07-29 09:05:13] Started app_entry (pid=1526)` |
| `app_sysmgr` **v2**（`/usr/sbin/app_sysmgr`，Sep 11 06:37，52624 B） | **不会运行（但源码里有，只是未编译进去）** | `strings` 中 `app_entry` / `nccu_app` / `syshand` / `run_app` 匹配数均为 **0**；**然而 BSP 源码 `sysmgr_process.c:506 start_app()` / `:526 execl(SYSMGR_APP_ENTRY,…,"/app")` 确实存在该实现** —— BSP 自己的对账注写明「`sysmgr_process.c` 有实现但 **Makefile 未编译（未集成）**」 |
| RAUC `app-post-install` | 会（**仅 OTA 时**） | 这是"OTA 后能起"的唯一原因 |

**结论（2026-09-16 修订）**：这不是“设计缺陷”，而是 **BSP 一个在途变更的迁移空档** —— 表面上“无人启动 app”，实际是：**R1 已完成**（`S92app_mount` 的启动调用已按设计移除，**不得加回**）、**R2 未交付**（`app_sysmgr` 的门控启动源码已就绪但未编译/未集成）。

### ⚠️ 权威依据（2026-09-16 补）：BSP 自己定义了启动 owner

`d9_sdk/openspec/changes/add-system-mgmt-service/specs/app-boot/spec.md`（建立于 2026-07-15；**该变更仍在途**，未归档）：

> **Overview**：app 启动流程从 `S91app_mount` 直接启动改为由系统管理服务（`app_sysmgr`）门控启动。
> **变更原因**：app 启动需要等待 R 核 SSDK 就绪，而 S91app_mount 不具备 iccmsg 通讯能力。启动门控职责移交给系统管理服务。
>
> **R1 `S92app_mount` 仅挂载（不再启动 app）**：`S92app_mount` SHALL 仅执行 dm-verity 挂载；并将 `S91app_mount` 改编号为 `S92app_mount` 且**移除 `run_app` / `app_entry` 调用**。Scenario：脚本结束（**不再**调用 `app_entry`）。
>
> **R2 app 由系统管理服务门控启动**：系统管理服务 SHALL 在 **R 核就绪且 `/app` 已挂载后**，以 `fork` + `exec` 启动 app。Scenario：`fork` + `exec` 启动 `/app/app_entry /app` → app_entry → startrun → run_app → syshand + nccu_app（现有 shell 链不变）。

**代码侧佐证**：

- `meta-semidrive/recipes-bsp/app-sysmgr/app-sysmgr/src/sysmgr_process.c`：`:506 start_app()`、`:526 execl(SYSMGR_APP_ENTRY, SYSMGR_APP_ENTRY, "/app", NULL)`、`:530 log_message("Started app_entry (pid=%d)")`；`:385` 判定死子进程是否为 app → `:405 start_app()` 重新拉起（**退避重启**）；`:525` 注释 **"S2.8: … (matches S92 run_app)"** —— BSP 作者明确知道 S92 的 `run_app`，是**有意取代**它
- `inc/sysmgr_internal.h:30` → `#define SYSMGR_APP_ENTRY "/app/app_entry"`
- `app-sysmgr_1.0.bb:1` SUMMARY = **"v2 daemon: iccmsg dual-channel + process supervisor"**
- **未集成的证据**：`add-system-mgmt-service/tasks.md` 的 2.7 / 2.8 / 2.9（2026-09-11 对账）明写“未集成（app 仍由 S92app_mount 启动）”、“`sysmgr_process.c` 有实现但 **Makefile 未编译**”

**旁证（BSP 另两处文档）**：

- `flutter-hmi/design.md:60`：“运行时分层（终版）：BSP = 显示栈 + **app 保活机制（app_sysmgr/app_entry）**”
- `flutter-hmi/tasks.md:16`：“app_entry 停用（**系统保活会拉回**，验证后恢复）”
- `add-system-mgmt-service/app-integration-guide.md:22`：“A 核进程保活 | `app_sysmgr` | — | **app 崩溃自动退避重启**”

### ⚠️ 由此推翻的旧计划（2026-09-16）

原 `tasks.md` **3.1** 曾写“向 BSP 交付 `S92app_mount` 的 `start)` 分支改动说明：…执行 `/app/app_entry /app &`” —— **与 BSP 的 R1 完全相反，已作废**。板端准备的 `/data/S92app_mount.new` 补丁**同理作废，不得应用**。

### `app_sysmgr` v2 的职责界定（已确认它不是监督者）

- 4 线程（PID `1003 / 1010 / 1207 / 1208`），`fd 1,2 → /dev/console`（故其输出在串口、不落文件 —— 这解释了为何 `/data/app_sysmgr.log` 停在 2025-07-29）
- `fd 3-15` 全为 rpmsg：`/dev/rpmsg_ctrl0..3`、`/dev/rpmsg0..3`、sysfs `soc:ipcc@1.sysmgr.-1.500` / `virtio0.sysmgr.-1.500`
- 状态 `S`，`wchan = hrtimer_nanosleep` → **定时循环在跑，非死锁**
- 全部日志串揭示职责：Phase 1 `BOOT_READY` ×3 握手 → Phase 2 查询 R 核 sysmgr ept（SSDK 版本 / AP-R form / R 核 active slot）→ `MARK_GOOD ok (slot %c GPT retry reset)` → `heartbeat started (PING every %ds)` / `R-core heartbeat LOST (no PONG)`
- **v1 有、v2 删除的两项**：`/var/run/sysmgr.sock` socket 服务、启动 `app_entry`
- 由 `S91app_sysmgr`（1122 B，Sep 11 06:33）在 boot 早期拉起（`/proc/1003/exe` mtime = `Sep 16 03:32`，紧随 `03:31:51` 开机）

### 对 D1 / 任务 3.x 的影响

1. **方案不变**：D1 仍选"`app_entry` 作为唯一入口"。本条调查只是补强理由 —— 不是"BSP 漏调 `run_app()`"，而是"**v1 有能力 → v2 删除 → `S92app_mount` 的现成代码未接线**"。交付 BSP 团队时此理由更易被接受。
2. **`app_entry` 幂等守卫的价值被实证**：`Risks` 表中“将来接入 guardian 时可能重复启动”不再是假想 —— 守卫将保护 **RAUC `app-post-install`（安装时）与 `app_sysmgr`（boot 时）重叠调用的窗口**（R2 的 Scenario 与 OTA 路径共用同一入口 `/app/app_entry`）。⚠️ **不存在“两个启动者”** —— R1 已要求 `S92app_mount` 移除调用
3. **`app_supervision` 的范围需修正**：该任务不必从零设计 —— v1 已有 socket + 启动 `app_entry` 的实现，v2 是把这两项删掉后的跨核专用版本。已登记为 `Open Questions` 新增项。

### 证据获取方式与可信度

- **真机**（最高可信）：`ls /etc/init.d/`、`strings`、`/proc/<pid>/{fd,status,wchan,exe}`、`/data/AppLog.data`、`/data/app_sysmgr.log`
- **源码**（次之）：`d9_sdk/source/buildroot/def_buildroot_rootfs/etc/init.d/`
- ⚠️ **方法弯路（须记录）**：早期仅凭 BSP 源码目录判定"只有 `S92app_mount`"，但该目录是**部分 overlay**（缺 `S10udev` / `S25modules` / `S50sshd`，也无 `S91app_sysmgr`），**不足以单独定论**。最终结论依赖真机 `ls`。同样的教训适用于 `S91app_mount` 改名判定（见变更 `fix-app-lifecycle-script-refs`）
