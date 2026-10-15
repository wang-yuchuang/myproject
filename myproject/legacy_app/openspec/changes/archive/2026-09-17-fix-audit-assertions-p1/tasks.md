## 1. Delta 编写（**机械生成**）

> 三个 delta 文件：
> - `specs/data-processor/spec.md` ← `## MODIFIED Requirements` + **R7**（canonical `265..293`，**29 行**）
> - `specs/publib/spec.md` ← `## MODIFIED Requirements` + **R2**（canonical `56..124`，**69 行**）
> - `specs/app-lifecycle/spec.md` ← `## MODIFIED Requirements` + **R9**（canonical `341..367`，**27 行**）

- [ ] 1.1 `data-processor` R7 的 **1 处**（1→1）
  - 原：``- **AND** 默认路径为 `/opt/appdata/data/nccu.data` ``
  - 新：``- **AND** 默认路径为 **`/data/nccu.data`**（代码兜底 `iDataBase.c:691`）；运行时由 `NCCU_DATA_PATH` 决定（`run_app:10` 设为 `/data`）。⚠️ **DB 文件在 `/data/` 顶层，其锁与索引在 `/opt/appdata/data/`** ``

- [ ] 1.2 `publib` R2 的 **1 处**（1→1）
  - 原：``| DB 日志 | `iLog.c` | SQLite 数据库文件 `$NCCU_DATA_PATH/<name>.data`（运行时为 `/data/{AppLog,BmsRunData,CloudData}.data`；表 `Data_AppLog` / `Data_DumpLog`）。**DB 文件在 `/data/`，其锁与索引在 `/opt/appdata/data/`** | SQLite |``
  - 新：``| DB 日志 | `iDataBase.c` | SQLite 数据库文件 `$NCCU_DATA_PATH/<name>.data`（运行时为 `/data/{AppLog,BmsRunData,CloudData}.data`；表 `Data_AppLog` / `Data_DumpLog`）。**DB 文件在 `/data/`，其锁与索引在 `/opt/appdata/data/`**。⚠️ 实现**不在 `iLog.c`** —— 该文件内的 `LogOut` 副本在 `:616` 已注释 | SQLite |``

- [ ] 1.3 `app-lifecycle` R9 的 **4 处 + 1 处追加**

  | # | canonical 行 | 变换 | 内容 |
  |---|---|---|---|
  | S9a | `:343` | 1→1 | 「系统 MUST 在收到停止信号时优雅停止…」→ 加「⚠️ **该能力当前不可达**（见下方 Scenario 与"已知缺陷"）」 |
  | S9b | `:347-353` | **7→5** | Scenario 标题加「（⚠️ 当前不可达）」；正文改为「trap 不会触发」的事实 + 「目标行为（设计意图）」 |
  | S9c | `:357` | 1→1 | `killall -9 nccu_ui_mm` → 按变体：**非 mu3** `nccu_ui_mm`（`stoprun:13`）/ **mu3（bmu05 实际生效）** `nccu_ui`（`mu3/stoprun:7`） |
  | S9d | `:364` | 1→1 | `/var/run/app.pid`（供 stop 使用）→ 补「⚠️ **但 app_entry 随即退出，该 PID 实际为死 PID**」 |
  | S9e | 追加于 `:367` 后 | +3 | `> **已知缺陷（2026-09-17 核实）**：…`（trap 不可达 + 需前台 supervisor；交 `app-supervision`） |

  **S9b 替换文本**（7 行 → 5 行）：
  ```
  #### Scenario: SIGTERM 触发停止（⚠️ 当前不可达）

  - **WHEN** `S92app_mount stop` 读取 `/var/run/app.pid` 并向该 PID 发送 SIGTERM
  - **THEN** ⚠️ **trap 不会触发**：`app_entry` 在 `:64` 调用 `startrun` 后**立即结束**（`startrun` 将 syshand 与 nccu_app 后台化），shell 退出则 trap 随之消失，`/var/run/app.pid` 成为**死 PID**
  - **AND** 目标行为（设计意图）：trap 依次执行 ① 打印停止日志 ② 调用 `${APP_SHELL_PATH}/stoprun` ③ 删除 `/var/run/app.pid` ④ `exit 0`
  ```

  **S9e 追加文本**（3 行）：
  ```
  > **已知缺陷（2026-09-17 核实）**：`app_entry` 是**非驻留**脚本（调用 `startrun` 后即退出），因此 SIGTERM trap **不可达**、`/var/run/app.pid` 为死 PID ⇒ **当前停止链路不可用**。
  > 正解需**前台 supervisor**（持有子进程并转发信号），属 `app-supervision` 范畴；已记录于 `keep-app-alive-on-bmu05` 的 `design.md`。
  > ⚠️ `src/shells/app_entry:14` 的注释「4. 注册 SIGTERM 信号处理器，优雅退出」同样与此事实不符（本变更不改代码，仅记录）。
  ```

- [ ] 1.4 自检：`diff <提取块> <替换后文本>` 只显示上述替换；`data-processor` R7 29→29、`publib` R2 69→69、`app-lifecycle` R9 **27→28**（S9b −2、S9e +3）

## 2. 破例手工编辑（delta **不可达**）——**已完成**

- [x] 2.1 `data-processor/spec.md:16`（Purpose 段）：DB 路径改为 `$NCCU_DATA_PATH/nccu.data`（运行时 `/data/nccu.data`）+ 注明锁在 `/opt/appdata/data/`
- [x] 2.2 `publib/spec.md:399`（Key Files）：`iLog.c` 说明改为「log4c 后端 + `LogOut4c`/`LogOut_BattDegr`/`LogCat`；`LogOut` 副本在 `:616` 已注释」
- [x] 2.3 `publib/spec.md:404`（Key Files）：`iDataBase.c` 补「**+ DB 日志后端（`LogOut` 家族）**」
- [x] 2.4 `publib/spec.md:400`（Key Files）：`iLog4c.c` 措辞统一为「log4c **文件日志**后端」
- [ ] 2.5 `.code-facts-snapshot.md`：同步以上三处事实（待做）

## 3. 验证

- [ ] 3.1 `openspec validate fix-audit-assertions-p1` 通过
- [ ] 3.2 `openspec validate --all` 全绿
- [ ] 3.3 归档**前**基线（⚠️ 用不会碰撞的片段）：
  - `grep -c '默认路径为 `/opt/appdata/data/nccu.data`' openspec/specs/data-processor/spec.md` → 预期 **1**（R7 内尚未改）
  - `grep -c '| DB 日志 | `iLog.c`' openspec/specs/publib/spec.md` → 预期 **1**（R2 内尚未改）
  - `grep -c 'killall -9 nccu_ui_mm' openspec/specs/app-lifecycle/spec.md` → 预期 **1**
  - `grep -c 'trap 被触发，依次执行' openspec/specs/app-lifecycle/spec.md` → 预期 **1**
- [ ] 3.4 `openspec archive fix-audit-assertions-p1 -y` → 预期三个 capability 各 `~ 1 modified`
- [ ] 3.5 归档**后**：`grep -c '| DB 日志 | \`iDataBase.c\`' openspec/specs/publib/spec.md` → **1**；`grep -c '该能力当前不可达' openspec/specs/app-lifecycle/spec.md` → **1**；`grep -c '默认路径为 \*\*\`/data/nccu.data\`\*\*'` → **1**
- [ ] 3.6 **手工编辑存活验证**：`grep -c 'log4c 文件后端 + \`LogOut4c\`' openspec/specs/publib/spec.md` → **1**；`grep -c 'DB 日志后端（\`LogOut\` 家族' openspec/specs/publib/spec.md` → **1**；`grep -c '运行时 \`/data/nccu.data\`' openspec/specs/data-processor/spec.md` → **1**
- [ ] 3.7 **行数判据**：`data-processor` R7 = **29**（不变）、`publib` R2 = **69**（不变）、`app-lifecycle` R9 = **28**（−2 +3，**有意变化**）
- [ ] 3.8 **转录完整性**：`git diff --unified=0 openspec/specs/` 的 `+` 内容行应为 **1(dp) + 1(pl) + (1+5+1+1+3)(al) = 13**；`-` 内容行应为 **1 + 1 + (1+7+1+1) = 12**。允许 `openspec` 自身的空行规范化与末条 Requirement 后的 `---` 移除（判据见已归档 `fix-system-hand-behavior-claims` tasks.md 3.6）

## 4. 待办 / 关联

- [ ] 4.1 `<FOLLOW-UP>` `.code-facts-snapshot.md` 的 `## system-hand` 与 `## publib` 节同步本次事实（DB 路径、LogOut 归属）
- [ ] 4.2 `<FOLLOW-UP>` `app_entry:14` 的注释（「4. 注册 SIGTERM 信号处理器，优雅退出」）与事实不符 —— 属代码注释，待下次触碰该脚本时修正
- [ ] 4.3 `<REMAINING>` 审核剩余项（**低价值、可合并一批**）：`samplers`/`comm-ports` 约 15 处 file:line 偏移、`comm-ports` 错误码 21 vs 20、`data-processor` INTERSET_TYPE 13 vs 12、spec 头 Version 0.4 与 `AGENTS.md` 的 v0.3 标注不一致
