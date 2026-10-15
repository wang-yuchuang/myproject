## 1. 应用侧启动链（A' + C）

- [x] 1.1 `src/shells/app_entry` 增加 liveness-aware 残留清理：对 `/opt/appdata/data/nccu.pid` 与 `syshand.pid`，先 `kill -0` **并按 `/proc/<pid>/comm` 核对进程名**（防止整机重启后 PID 回收复用造成误判），仅在确非目标进程时删除；`/var/volatile/{syshand,nccu}.pid` 直接删除；**`.syscmd_locker` 改为不删除**（`flock` 随持有者退出由内核自动释放，残留文件不阻塞加锁；unlink 反而会使新进程在新 inode 上加锁而破坏跨进程互斥）
- [x] 1.2 更新 `src/shells/app_entry` 头部注释：调用方为 BSP `S92app_mount`（boot）与 `/usr/lib/rauc/app-post-install`（OTA），不再是 S91app_mount
- [x] 1.3 `src/shells/mu3/run_app:82` 存活判定由 `'[s]yshand'` 改为 **`pidof syshand`**（依据审核建议：`pidof` 按进程名精确匹配，同时消除假阳性与子串类假阴性；bmu05 实测有效）
- [x] 1.4 修复非 mu3 变体的同类缺陷：`src/shells/run_app:129` 同样使用 `'[s]yshand'`，已同步改为 `pidof syshand`。（注：同文件 `:117` 与非 mu3 `:168` 的 `ps | grep bin/syshand` 未改动 —— 它带 `grep -v grep` 无自匹配问题且已在生产运行，保持最小改动）
- [x] 1.5 同步 `openspec/specs/app-lifecycle/spec.md` 中 R7 之外的 `S91app_mount` 引用（R6 `:221`、R8/R9 `:350`、`:366`；另有 Purpose 段 `:18,20` 与 `:136,141`）：**已记录**。delta 机制只能替换 Requirement 块，Purpose 段需在下次 canonical 整体重写时处理

## 2. CCU 看门狗门控（B）

- [x] 2.1 在 CCU 结构体 `ROUGH_CCU_DATA`（`ccu_sampler_main.h`，紧邻 `bIsReceivedCFGG_FromLCCU`）新增 `BOOL bEverRecvHeart;`，与 kwhmeter 的 `bLast_AllMeterCommOK` 同构。初始化为 FALSE —— `Initialize()` 的 `ZERO_POBJS(pCCU, 1)`（`ccu_sampler_main.c:429`）已覆盖；**未**加入 `InitCCUStuffData`（`ccu_sampler_main.c:257-268`）
- [x] 2.2 `ccu_protocol_parsers.c:752`（唯一真实心跳帧接收点）置该标记为 TRUE
- [x] 2.3 `ess_sampler_main.c:1420` 的回触发条件前置该标记：`if(!pCCU->bEverRecvHeart) { …告警… } else if(s_nCommFaileResetCounter > 10) { …重启… }`
- [x] 2.4 未建立通信时的通信中断保留可观测性：既有的 `"CCU communication FAILED, please check comm-port cable!!!"`（`ess_sampler_main.c:1403`）继续输出，并新增一次性 `"CCU-RS232 never established communication; skip system reset (alarm only)"`（计数器==11 时只打一次）；MUST NOT 触发重启。**注**：真实 DPR 告警 ID 仍为 Open Question，待定后再挂
- [x] 2.5 已记录 `handler.c:118-125`（180 秒无心跳 → syshand `break` 退出）为本变更**未覆盖**的级联风险（`_SYSTEM` 失效 → `raise(SIGUSR1)` → 应用退出）；实测未触发，交 `app-supervision` 处理（已写入 `design.md` 风险表）

## 3. BSP 侧依赖交付（A''，本仓库不实现）

- [ ] 3.1 **【2026-09-16 方向更正】** 向 BSP **确认/催办 `add-system-mgmt-service`（R2：app 由系统管理服务门控启动）**，而**不是**修改 `S92app_mount`。
  - **权威依据**：`d9_sdk/openspec/changes/add-system-mgmt-service/specs/app-boot/spec.md`
    - **R1「`S92app_mount` 仅挂载（不再启动 app）」**：明确要求“移除 `run_app` / `app_entry` 调用”
    - **R2「app 由系统管理服务门控启动」**：“系统管理服务 SHALL 在 R 核就绪且 `/app` 已挂载后，以 `fork` + `exec` 启动 app”；其 Scenario 即为 `/app/app_entry /app`
    - Overview 写明改名原因：“app 启动需要等待 R 核 SSDK 就绪，而 S91app_mount 不具备 iccmsg 通讯能力”
  - ⚠️ **曾错误地计划“在 `S92` 的 `start)` 加一行 `/app/app_entry /app &`” —— 该做法直接违反 BSP 的 R1，已作废**
- [ ] 3.2 向 BSP 说明其 `run_app()` 已冗余可删（**已有权威依据**：`app-boot` R1 明确要求“移除 `run_app` / `app_entry` 调用”），并指出 `mkdir /data/app/*` 与 `cp /app/config → /data/app/config` 为残留无效逻辑（实测 `/data/app` 不存在且无读取方）

## 4. 验证（bmu05 真机）

- [x] 4.1 ✅ **已完成**（构建由上题所述的子代理执行，产物已审核）：`remote_pkg/app-dev-9b3a00b3-20260916.raucb`（15022545 B, `Compatible: bmu05_a00`, `Root hash d2c25c93…`, 构建日志 `error:` 计数=0）→ `rauc install` 签名验证通过、目标槽 **app.1 (app_b)** → post-install 后 `/app` 挂载于 `/dev/mapper/app_b`，**`nccu_app`=2030 / `syshand`=2017 已启动**。部署镜像内新代码已核实：`/app/app_entry:21` 含 `pidof nccu_app`、`cleanup_stale_pid`×3、`/app/run_app:82` 含 `pidof syshand`、`sampler_ccu.so` 含闩锁新字符串、`syshand` 含新心跳消息；`/app/app_entry` 大小 3457 B 与 `out/app_entry` 完全一致。另已完成**幂等守卫**验证：运行中再调 `app_entry` → 打印 `nccu_app already running, skip duplicate start`、`exit=0`、前后 PID 完全相同
- [x] 4.2 ✅ 已完成（远超 ≥10 分钟要求）：(a) OTA 启动实例：03:22:28 至 03:31:21 逐 5s 采样，PID 恒定 2030/2017、loop 173 增至 249、rcK 0；(b) 手动启动实例（4.7 那次，PID 2884/2871）：连续运行 7341 秒 = 2h02m21s（由 /proc/2884/stat starttime 精确换算），monitor4 采集 240 条样本（03:35:00 至 03:55:07，PID 恒定、loop 258 增至 415、rcK 0），trace 主循环 count=76600 持续递增，全程 rcK=0。对比：旧代码每约 337s 重启一次，2 小时应约 21 次重启，实测 0 次
- [x] 4.3 ✅ **已完成（B 生效，决定性证据）**：应用运行 ~400s（> 旧代码必重启点 337s）时，AppLog 新增行 `CCU232Samp [CCU_Tcp_232] CCU-RS232 never established communication; skip system reset (alarm only)`（正是新增分支的消息），且 `grep -c rcK /var/log/messages` **= 0**、`nccu_app`/`syshand` 存活；旧代码应打印的 `!!!Now we will RESET the system because of NO response from CCU-RS232!!!` **未出现**
- [ ] 4.4 ⛔ **根因已改写（2026-09-16）**：该验证阻塞于 **BSP 完成 `add-system-mgmt-service` 的 R2**（app 由 `app_sysmgr` 门控启动），**不是**“S92 需要启动改动”。BSP 自己的对账注（`add-system-mgmt-service/tasks.md` 的 2.7 / 2.8 / 2.9，2026-09-11）写明：**“未集成（app 仍由 S92app_mount 启动）”**、**“`sysmgr_process.c` 有实现但 Makefile 未编译（未集成）”** —— 即 **R1（移除 S92 的启动调用）已完成、R2（app_sysmgr 接管启动）未交付**，属**迁移空档**。⚠️ **板端 `/data/S92app_mount.new` 补丁为红鲱鱼，应删除、不得应用**（它与 R1 完全相反）。已完成部分：① 负向对照 —— 重启后 `/app` 已挂载但两进程为空，完整复现线上症状；② app 侧那一半已用手动 `/app/app_entry /app` 验证通过（见 4.1 / 4.7）
- [ ] 4.5 ⛔ 依赖 4.4，**需 BSP 交付 R2（app_sysmgr 门控启动）后重跑**（非“rootfs 改 S92”）。注：本轮已用手动启动路径取得两次独立观察窗口的零重启证据（4.2 / 4.3），说明 A+B 的应用侧行为正确
- [x] 4.6 ✅ N/A，无需反证：两个观察窗口内 rcK 均为 0，AppLog 中已无 RESET the system 字样，取而代之的是新增的 skip system reset (alarm only)
- [x] 4.7 ✅ 已完成（含 PID 回收复用保护）：人为遗留 nccu.pid=999999（不存在）与 syshand.pid=1（存在但 comm 为 init）后调用 /app/app_entry /app，输出 removing stale /opt/appdata/data/nccu.pid (pid=999999) 与 removing stale /opt/appdata/data/syshand.pid (pid=1)，随后 nccu_app launched PID=2884、syshand=2871 启动成功，两个 pid 文件被进程自身重写。同时构成反向验证：若残留 nccu.pid 未被清除，run_app:136-139 会 exit 1 拒绝启动
- [ ] 4.8 ⛔ 台架不可验证：需真实 CCU 先成功通信一次（置位闩锁）再断链。台架无 CCU，且伪造合法 HEART 帧需实现 CCU 的 NL 组帧与 CRC 协议，不具性价比。替代依据：① 代码级已确认闩锁置位后 else if 分支与原逻辑逐字节等价（双审核加本机复核）；② 同一模式已在 kwhmeter_sampler 生产运行（bLast_AllMeterCommOK）；③ 建议在接真实 CCU 的现场或带仿真器环境补验
- [x] 4.9 ✅ 崩溃后手动重启（补充验证，覆盖 4.7 未涉及的保留分支）：kill -9 nccu_app（PID 2884）后 syshand 仍存活、nccu.pid 残留；调用 /app/app_entry /app 输出 removing stale .../nccu.pid (pid=2884) 与 .../syshand.pid - pid 2871 (syshand) alive, keeping it（保留分支生效），随后 run_app 打印 The sys_hand is ALREADY running（pidof syshand 的 MATCH 路径真实生效）并启动 nccu_app PID=31146，syshand PID 2871 未变（正确复用、未被杀，无 System V shm/sem 遗留）

## 5. 审核与排查中发现的附加缺陷（构建前修复）

- [x] 5.1 `src/utilities/system_hand/handler.c:118-126`：心跳丢失达 18×10s 时不再 `break` 退出（原实现使 syshand 自杀 → `_SYSTEM` 无应答 → `iBasic_Funs.c:361 raise(SIGUSR1)` → 应用退出）；现保留日志、重置计数器、继续服务
- [x] 5.2 `src/main/main.c:476`：`return 0` → `return s_pChargeThis->nExitCode`，传播退出原因（0=正常停止，非 0=故障），供诊断与未来 supervisor 使用
- [x] 5.3 `src/publib/iBasic_Funs.c:2634-2652` `trace_fatal`：① `va_list` 改为只解析一次（原实现 `vprintf` 后复用同一 `va_list` 调 `vsnprintf`，属 UB）；② `LogOut(..., "%s", szBuff)`（原实现把消息当格式串，格式串漏洞）。对应新增 requirement `publib` R10
- [x] 5.4 **已知未修复项（已记录于 `design.md` D7）**：`main.c:427-449` reset 在事件繁忙时可能被 `continue` 饿死（只延迟不取消，且改它会违反 R15 的「生产行为保持不变」，故不改）；`app_entry` 的 SIGTERM trap 仍不可达 + `S92app_mount stop` 读到的是 app_entry 死 PID → **停止链路不可用**（需前台 supervisor，留待 `app-supervision`）
- [x] 5.5 `src/main/main.c:316-326`：`syshand.run` 就绪等待由 1.1s（`nMaxWait > 10`）延长到 10.1s（`> 100`）。原窗口在启动负载高时可能因 `run_app` 的 `pidof` 存活检查先于 syshand 写完就绪标记通过而误判退出（直接关系「能否启动」）
- [x] 5.6 **记录不修（已写入 design.md D7）**：`iBasic_Funs.c:361/492/627` syshand 无应答 → `raise(SIGUSR1)` → 应用退出（改为仅记日志后 `_SYSTEM` 会阻塞 30s，应用由「死」变「僵」；正解是 supervisor 重启 syshand）；`iThread.c:787` 线程 3 次心跳丢失即整应用退出（属 supervision 范畴）
- [x] 5.7 **构建前预检**（本地沙箱不允许 `sh -n`，故上传到 bmu05 用**目标 shell**（BusyBox ash）校验）：`app_entry`(64 行/LF)、`mu3/run_app`(154 行/**CRLF**)、`run_app`(200 行/LF) 经 `tr -d '\r'` 归一化后 `sh -n` **全部 SYNTAX OK**；且 `mu3/run_app` 相对 HEAD 归一化后**仅 1 行差异**（`[s]yshand` → `pidof syshand`）。CRLF 为**既有属性**（HEAD 同为 154/154 CRLF，非本次引入），由 `src/shells/Makefile` 的 `dos2unix` 在安装时归一化。
- [x] 5.8 **首次构建失败的修复记录**：首次构建报 `main.c:477:1: error: expected declaration or statement at end of input` —— 原因是 5.5 的编辑中，替换块末行 `}` 与紧随的既有 `}` 相同，被编辑工具的 boundary-echo 自动剔除，导致 `if` 未闭合。已补回并标注为 `} /* end if(nMaxWait > 100) */`；预检确认 `nMaxWait > 100` 与 `end if(nMaxWait` 各命中 1 次。
