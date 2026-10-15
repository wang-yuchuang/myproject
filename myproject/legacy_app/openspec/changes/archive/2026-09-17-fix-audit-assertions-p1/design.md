## Context

P1 批次处理三条"与代码不符"的断言。它们的**性质各不相同**，因此处置方式也不同：

| 条目 | 性质 | 为何放在同一变更 |
|---|---|---|
| `data-processor` R7 路径 | 事实错误（路径） | 同一类：审核发现的断言错误 |
| `publib` R2 归属 | 事实错误（文件归属） | 且与 R7 同源 —— 都是"运行时路径/文件归属"族的残余 |
| `app-lifecycle` R9 停止链 | **能力描述与实际可达性不符** | 更严重：spec 声称一个**当前不可用**的链路可用 |

## Goals / Non-Goals

**Goals**

- 消除三处会让读者"照着做却做不通"的断言
- 对 R9，把"**目标行为**"与"**当前实际行为**"显式分开 —— 既不删掉设计意图，也不谎称它可用

**Non-Goals**

- 不改任何代码（代码行为是事实）
- 不修 `app_entry` 的 trap（正解需前台 supervisor，属 `app-supervision`）
- 不追 `samplers`/`comm-ports` 的约 15 处 file:line 偏移（低价值，另立批次）
- 不顺带改 `app_entry:14` 的误导注释（属代码，另记 `<FOLLOW-UP>`）

## Decisions

### D1：R9 必须把「目标行为」与「当前行为」**分列**，而不是二选一

两种极端都不可取：

| 做法 | 问题 |
|---|---|
| 保留原文（声称 trap 生效） | **谎报能力可用** ✗ —— 实施者会依赖它 |
| 删除 Scenario | 丢失**设计意图** ✗ —— 未来做 supervisor 时不知道目标是什么 |

**选用**：Scenario 标题标「⚠️ 当前不可达」，正文写**实际发生什么**；再用一条 `**AND** 目标行为（设计意图）：…` 保留意图 ✓ 另加 `> **已知缺陷**` blockquote 指出正解方向 ✓

### D2：R9 的块行数**有意变化**（27 → 28），并预先声明

前几次变更的验收判据是"块行数守恒"，因为替换都是等长的。本次 R9 的 S9b 由 7 行压到 5 行、S9e 追加 3 行 ⇒ 净 **+1**。

**因此本变更的判据改为"行数变化可逐项解释"**（S9b −2、S9e +3 ⇒ +1），而非"必须守恒"。这与 `fix-system-hand-behavior-claims` 得出的教训一致：**判据要匹配本次变换的真实形状**，不能机械套用上一次的。

### D3：三处手工编辑同样遵循"破例四条约束"

沿用 `fix-reporter-ids-and-compile-tables` 建立的约束（最小范围 / 可复核 / 写入 canonical 注 / 归档后验证）：

- 手工编辑仅 4 行（`data-processor:16`、`publib:399/400/404`），全在 delta 不可达的位置
- 每处都在 `tasks.md` §2 列出并在 §3.6 设存活验证
- `publib` 的 `## Key Files` 修正**顺带消解了 spec 内部矛盾**：`data-processor:287` 早已写着「实现在 publib iDataBase.c」，与本 spec 的 Key Files 说法相冲突 —— 本次统一

### D4：R9 的 mu3 变体差异必须写明，因为**bmu05 恰好走 mu3**

`mu3/stoprun:7` 是 `killall -9 nccu_ui`，非 mu3 的 `stoprun:13` 才是 `nccu_ui_mm`。而生产构建 `MU_TYPE=mu3` ⇒ **bmu05 上实际执行的是 `nccu_ui`**。

只写 `nccu_ui_mm` 会让读者以为"UI 进程叫 nccu_ui_mm"，在 bmu05 上排查时找错目标。故显式分列两个变体并标注哪个实际生效。

## Risks / Trade-offs

| 风险 | 等级 | 缓解 |
|---|---|---|
| R9 行数变化被误判为转录错误 | 中 | D2 预先声明净 +1 与逐项来源；验收判据改为可解释性 |
| 手工编辑被归档覆盖 | 中 | §3.6 设 3 条存活 grep；不符则回退 |
| 读者把 R9 的"当前不可达"理解为"该功能已废弃" | 低 | Scenario 保留「目标行为（设计意图）」并指向 `app-supervision` |
| 与 `keep-app-alive-on-bmu05` 的 R9 改动重叠 | 低 | 该变更只改 R7 与 `publib`/`samplers` 的 delta（无 R9）；两者无 MODIFY 冲突，归档顺序无关 |
