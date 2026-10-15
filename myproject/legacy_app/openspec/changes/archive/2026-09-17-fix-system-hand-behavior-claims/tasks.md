## 1. Delta 编写（**机械生成**，禁止人工重抄）

> delta 文件：`specs/system-hand/spec.md` ← `## MODIFIED Requirements` + **R1** + **R5**
> 生成方式：从 canonical **逐行提取** → 做下列 **8 处替换** → `diff` 自检；**替换前后行数必须相等**。

- [ ] 1.1 提取 `openspec/specs/system-hand/spec.md` 的 **第 27–67 行**（R1，41 行）与 **第 199–229 行**（R5，31 行）
  - 校验：R1 首行 = `### Requirement: R1 主进程心跳监控`；R5 首行 = `### Requirement: R5 PID 文件管理与进程生命周期`
- [ ] 1.2 R1 的 **5 处**替换（共 10 行）

| # | canonical 行 | 原（行首片段） | 改为 |
|---|---|---|---|
| S1a | `:29` | `系统 MUST 持续监控 nccu_app 主进程的心跳，心跳丢失时恢复系统。` | `系统 MUST 持续监控 nccu_app 主进程的心跳；心跳丢失时**仅记录并继续服务**，MUST NOT 重启系统、MUST NOT 退出进程。` |
| S1b | `:37` | `- **阈值**：\`nNoHeartBeatCounter >= 18\`（即约 **180 秒**无心跳）触发**恢复**` | `…触发**告警（仅记录）**` |
| S1c | `:39-40` | `> **当前状态（调试试态）**：… **生产部署前必须取消注释。**`（2 行） | 见下「S1c 替换文本」 |
| S1d | `:42-44` | `> **⏳ 架构演变方向**（规划中，见 AGENTS.md）：…（3 行）` | 见下「S1d 替换文本」 |
| S1e | `:59-61` | 旧消息串 / `生产态下执行 system("reboot -f")` / `break 退出 Main_Loop`（3 行） | 见下「S1e 替换文本」 |

**S1c 替换文本**（2 行 → 2 行）：
```
> **行为已确认（2026-09-16，非调试试态）**：心跳丢失 180s 后 syshand **仅记录日志并重置计数器，继续服务**，该分支**不存在** `system("reboot -f")` 调用（已移除，非注释）。
> **MUST NOT 以"生产前取消注释"为由恢复整机重启** —— 既定方向为「应用故障不重启整机」；重启应用归上层保活机制（BSP `app_sysmgr`，见 [app-lifecycle R10](../app-lifecycle/spec.md)）。残留 `//system("reboot")` 仅在 `handler.c:173` / `:215` 其它分支。
```

**S1d 替换文本**（3 行 → 3 行）：
```
> **架构现状**：syshand 已保留为 **OS HAL 抽象层**，其**系统修复（reboot）职责已移除**，心跳监控仅保留为**可观测性**手段。
> 应用进程的启动与崩溃重启由 BSP 的 `app_sysmgr`（AP↔R 跨核管理守护的保活机制）承担，见 [app-lifecycle R10](../app-lifecycle/spec.md)。
> 本仓不实现进程监管；如需变更启动 owner，走 BSP 的 openspec 变更。
```

**S1e 替换文本**（3 行 → 3 行）：
```
- **THEN** syshand 记录 `LOG_TYPE_ERROR`：`"Main APP heartbeat lost; keep serving as OS HAL (no reboot, no exit)"`
- **AND** 同时向 stdout 打印 `"[syshand] heartbeat lost; staying alive (reboot/exit disabled)"`，并重置 `nNoHeartBeatCounter = 0`（约每 180s 一条）
- **AND** `continue` 回到 `Main_Loop` 的 `sem_timedwait` —— MUST NOT 重启系统、MUST NOT 退出（`handler.c:118-127`）
```

- [ ] 1.3 R5 的 **3 处**替换（共 5 行）

| # | canonical 行 | 原 | 改为 |
|---|---|---|---|
| S5a | `:210-212` | `fopen("w+t")` 成功 → 写 PID → `run_app` 的 `ps \| grep -q '[s]yshand'` 补充防护（3 行） | 见下「S5a 替换文本」 |
| S5b | `:215` | `- **WHEN** syshand 从 \`Main_Loop\` 正常返回（心跳超时 \`break\` 或 IPC 初始化失败）` | `- **WHEN** syshand 从 \`Main_Loop\` 返回（**仅致命错误**如 IPC 初始化失败；心跳丢失路径已不退出）` |
| S5c | `:220` | `- **WHEN** \`run_app\` 启动 syshand 前 \`ps \| grep -q '[s]yshand'\` 检测到进程已存在` | `- **WHEN** \`run_app\` 启动 syshand 前用 \`pidof syshand\` 检测到进程已存在（`run_app:129` / \`mu3/run_app:82\`）` |

**S5a 替换文本**（3 行 → 3 行）：
```
- **WHEN** syshand 启动时以 `fopen(_SYSHAND_PID_FILE, "w+t")` 打开 PID 文件
- **THEN** 写入当前 PID（`fprintf(fp, "%d\n", getpid())`），`chmod 0777`；⚠️ `"w"` 语义是**截断或创建**，**文件已存在同样成功** —— 故 `smain.c:7` 的 `fp == NULL` 分支**不可能因"进程已在运行"而触发**（仅路径不可写时触发）。该文件是**供 app 端脚本判断的标记**，**不是自锁**
- **AND** 真正的防重复启动在 `run_app` 侧：启动前用 `pidof syshand`（按 `/proc/<pid>/comm` 精确匹配）检查，若已存在则不重复启动
```

## 2. 明确排除项

- [x] 2.1 不改代码（代码行为正确，是文档错了）
- [x] 2.2 不动 R2 / R3 / R4；不新增 Requirement；不重编号
- [x] 2.3 不追求"顺手修"其它低价值项（`system-hand` 尚有：`:215` 附近的行号 +2 偏移、spec 头 `Version: 0.4` 与 `AGENTS.md` 的 `v0.3` 不一致、R5 `:224` 的 `run_app:174-179` 行号区间）—— 这些留待后续"行号刷新"批次
- [x] 2.4 R5 `:224` 的 mu3/非 mu3 双重检查描述**经核实基本正确**（非 mu3 `run_app:129` + `:175`；mu3 `:82` + `:126` 注释掉 PID 检查），故**不动**

## 3. 验证

- [x] 3.1 ✅ **通过**：`Change 'fix-system-hand-behavior-claims' is valid`
- [x] 3.2 ✅ **通过**：归档前 `9 passed, 0 failed`；归档后 `8 passed, 0 failed`
- [x] 3.3 ✅ **实测**：`WOULD reboot but disabled` = **1**、`生产部署前必须取消注释` = **1**、`### Requirement` = **5**
- [x] 3.4 ✅ **已执行**：→ `system-hand: update` / `~ 2 modified` / 归档为 **`2026-09-17-fix-system-hand-behavior-claims`**（⚠️ 日期为 **09-17**，因执行时系统日期已跨天）
- [x] 3.5 ✅ **实测**：`WOULD reboot but disabled` = **0**、`keep serving as OS HAL` = **1**、`不是自锁` = **1**、`### Requirement` = **5**（MODIFIED，非 ADDED）
- [x] 3.6 **转录完整性检查（最重要）** ✅ **通过 —— 但原判据写错了，已修正**
  - 实测 `git diff --unified=0`：**内容删除 16 行**、**内容新增 15 行**（原判据要求两侧均为 15，**过于严格**）
  - 差值来源：`openspec` 重写 requirements 区块时，会删掉**最后一个 Requirement 之后那个 `---` 分隔线**（连同一个空行），属工具自身规范化，非抄写错误；另有两处 `## Requirements` 前的空行规范化
  - **有效判据**：① `+` 内容行 = **15**（即 8 组替换的行内新增）；② **R1 / R5 块行数不变**。本次两条均成立：R1 `25..65` = **41**、R5 `197..227` = **31**
  - **教训（供后续 delta 作者）**：当 MODIFIED 触及**该 spec 的最后一个 Requirement** 时，尾部 `---` 会被移除 —— 审阅 `git diff` 时不要把这一行当作自己的错误；判据应写成「新增行数 = 替换数」而非「两侧相等」
- [x] 3.7 ✅ **通过**：diff 中除 R1 / R5 区块外，仅出现 `## Requirements` 前两行空行与尾部署 `---` 的规范化；R2 / R3 / R4 全文**逐字未变**

## 4. 待办 / 关联

- [ ] 4.1 `<FOLLOW-UP>` 同 spec 的 `Version: 0.4` 与 `AGENTS.md` 的「spec v0.3」标注不一致 —— 待一次"元数据/行号刷新"批次统一
- [ ] 4.2 `<FOLLOW-UP>` 本轮审核另有约 15 处低价值 file:line 偏移（`comm-ports` / `samplers` / `data-processor` 等）—— 未纳入本变更
