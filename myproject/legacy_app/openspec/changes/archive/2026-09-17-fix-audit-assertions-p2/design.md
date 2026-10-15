## Context

`fix-audit-assertions-p1` 归档时，其 `tasks.md` 4.3 `<REMAINING>` 登记了一批「低价值、可合并一批」的剩余审核项：

```
samplers/comm-ports 约 15 处 file:line 偏移、comm-ports 错误码 21 vs 20、
data-processor INTERSET_TYPE 13 vs 12、spec 头 Version 0.4 与 AGENTS.md 的 v0.3 标注不一致
```

本变更只结清其中**已经过直接核对**的 2 项（`INTERSET_TYPE` 13 vs 12、版本标注不一致）+ 1 项同类的入口文档陈旧（停止链）。其余保持登记。

## Decisions

### D1 为什么 R6 走 delta、`AGENTS.md` 走直编

- R6 位于 canonical 的 `## Requirements` 段内 ⇒ **delta 可达** ⇒ 按既定规则走 **change + archive**（规则见 `fix-audit-assertions-p1`/`fix-system-hand-behavior-claims` 的先例）。
- `AGENTS.md` 不是 capability spec，不在 delta 覆盖面内 ⇒ 按 `fix-reporter-ids-and-compile-tables` 的先例**直接编辑**，并在 proposal / tasks 中逐行登记。

### D2 只改 `:255` 一行，且按**代码顺序**插入

代码枚举顺序为：`RAW_SP` → `LOGIC_SP` → `INTERN_SETT` → `RAW_ST` → `INTERN_CTRL` → `SENDMSG` → `RAW_BACK_CTRL` → `RAW_BACK_SETT` → **`RESTORE_SETTINGS_FROMDB`** → `CLEAR_FACTOY_DEBUG_DATA` → `OVERWRITE_SET_BEFORE_CLONE` → `READ_VALUE` → `Reread_PCS_Param`。
清单按此顺序插入新值、不重排，保持与 `dataprocessor_model.h:70-85` **一一对应**，便于后续 drift 时逐项比对。同时把「支持的设置类型（`INTERSET_TYPE` 枚举）」补为「**共 13 种**」，与 `## Key Files` 的「13 类型」自洽。

### D3 `AGENTS.md` 改为**版本无关**措辞

不写死「v0.4」（任何一次 spec 升级都会再次过期），改为指向各 spec 头部：

- `:5` → `**Specs**: \`openspec/specs/\`（7 个子系统，版本见各 spec 头部）`
- `:124` → `## 子系统关键事实（spec 全量审核结果）`

### D4 停止链的入口文档同步（`:65-66`）

`app-lifecycle` R9 已改为「⚠️ 当前不可达」，但 `AGENTS.md` 的启动链仍画成 trap 生效。按**同一事实源**在 `:65-66` 加简短警示，不重写链路图（保持入口文档的可读性）：

- `PID → /var/run/app.pid` 后补「（⚠️ 脚本随即退出，实为死 PID）」
- `trap SIGTERM → stoprun` 后补「（⚠️ 当前不可达）」

### D5 未纳入项与「已证伪」项

- 约 15 处 file:line 偏移：来源报告同时产出了被证伪的错误码断言，**可信度不足**，逐条核对成本高于其价值 —— 保持登记。
- 「`comm-ports` 错误码 21 vs 20」：**判定为报告错误**。复核证据：canonical spec 中不存在「20 个」字样；报告引用的枚举名 `_eErrRet` 在代码/ spec 中均不存在。在 proposal 中显式记录，避免后续审核重复引用同一伪断言。
- `app_entry:14` 代码注释：属代码，且本仓当前约束为「暂不大改」，留待下次触碰该脚本。

## Risks / Trade-offs

| 风险 | 级别 | 缓解 |
|---|---|---|
| delta 转录不完整导致归档丢内容 | 低 | 全文复制 R6（canonical `219..261`），只替换 `:255` 一行；归档后**行数判据**：R6 = 43 行（不变） |
| `AGENTS.md` 直编未被 delta 覆盖、可能被后续归档误判为漂移 | 低 | 在 proposal/tasks 显式登记 3 处直编；`AGENTS.md` 本就不在任何 delta 覆盖范围内 |
| 再次声称版本号 | 低 | D3 选择版本无关措辞，从源头消除 |
