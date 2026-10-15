## Why

2026-09-16/17 spec 全量审核的 **P2 批次** —— 结清 `fix-audit-assertions-p1` 归档时登记的剩余低价值项（其 `tasks.md` 4.3 `<REMAINING>`）中**已经过直接核对**的 2 项。

### a. `data-processor` R6 — `INTERSET_TYPE` 值清单**少一个**

| 位置 | 现文 | 代码实测 |
|---|---|---|
| `spec.md:255`（R6 内，**delta 可达**） | 列出 **12** 个 `SETTYPE_*` 值 | `dataprocessor_model.h:70-85` 定义 **13** 个 |

缺的是 **`SETTYPE_RESTORE_SETTINGS_FROMDB`**（`dataprocessor_model.h:79`，注释 `//restore settings from SD card`，位于 `SETTYPE_RAW_BACK_SETT` 与 `SETTYPE_CLEAR_FACTOY_DEBUG_DATA` 之间）。

> ⚠️ 本 spec 自身即自相矛盾：`:317`（`## Key Files`，**delta 不可达**）已正确写着「`INTERSET_TYPE` 枚举（**13 类型**）」，只有 R6 的散文清单漏列。

### b. `AGENTS.md` — 声称**单一 spec 版本** v0.3

| 位置 | 现文 | 事实 |
|---|---|---|
| `AGENTS.md:5` | `**Specs**: \`openspec/specs/\` v0.3` | 7 个 spec **并非同一版本** |
| `AGENTS.md:124` | `## 子系统关键事实（spec v0.3 验证结果）` | 同上 |

实测各 spec 头 `**Version**`：`comm-ports` / `samplers` / `system-hand` = **0.4**；`publib` / `data-processor` / `reporters` / `app-lifecycle` = **0.3**。
`AGENTS.md` 是**跨子系统**入口文档，MUST NOT 断言单一版本（会随任一 spec 升级立刻过期）。

### c. `AGENTS.md` — 停止链仍写成**可用**（与修正后的 R9 矛盾）

`app-lifecycle` R9 已于 `fix-audit-assertions-p1` 修正为「**当前不可达**」：`app_entry` 调 `startrun` 后即退出，SIGTERM trap 消失，`/var/run/app.pid` 是**死 PID**（canonical `:343` / `:350` / `:362` / `:366`）。
但 `AGENTS.md:65-66` 仍把 `PID → /var/run/app.pid` 与 `trap SIGTERM → stoprun` 画成生效链路 —— 同一类「文档声称行为不成立」。

## What Changes

- **`data-processor`**：MODIFIED **R6**（1 行：`:255`；补 `SETTYPE_RESTORE_SETTINGS_FROMDB`，并标注「共 13 种」）
- **`AGENTS.md`（直接编辑，非 capability spec）**：`:5`、`:124` 去掉单一版本断言；`:65-66` 标注死 PID + trap 不可达
- **不改任何代码** —— 代码行为已是事实，错的是文档

## Capabilities

### New Capabilities

- (无)

### Modified Capabilities

- `data-processor`：R6「跨模块数据访问」的 `INTERSET_TYPE` 值清单

## Impact

- **文档**：`openspec/specs/data-processor/spec.md`、`AGENTS.md`
- **代码**：无变更 ⇒ 无构建/烧录验证需求
- **关联**：`fix-audit-assertions-p1`（本变更为其 P2 续作，结清 4.3 的 2 个已验证项）；`keep-app-alive-on-bmu05`（R9 停止链缺陷的登记方，本变更只同步入口文档）
- **风险**：低 —— 1 处 delta（1→1 行）+ 3 处 `AGENTS.md` 元数据，无行为影响

### 未纳入（保持登记，不修）

- **`samplers` / `comm-ports` 约 15 处 file:line 偏移** —— 与下条同源报告产出，**未经逐条核对**，不得直接套用（见 `fix-audit-assertions-p1` tasks 4.3）。
- **「`comm-ports` 错误码 21 vs 20」—— 经复核无法证实，判定为报告错误，不予修改**：spec 内不存在「20 个」字样，报告引用的枚举名 `_eErrRet` 亦不存在。
- **`src/shells/app_entry:14` 代码注释**（「4. 注册 SIGTERM 信号处理器，优雅退出」）与事实不符 —— 属**代码**，待下次触碰该脚本时修正（本变更只修文档）。
