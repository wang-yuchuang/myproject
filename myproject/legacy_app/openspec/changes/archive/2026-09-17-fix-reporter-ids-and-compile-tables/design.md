## Decisions

### D1：把"可达性"作为 delta 设计的第一约束

本变更第一次遇到"想改的内容 delta 改不到"。处理原则：

> **先判定可达性，再决定机制。** 一个 change 里可以同时包含
> ① 经 delta 的修改（可达）与 ② 破例手工编辑（不可达），但**必须显式区分并在文档中留痕**。

可达性判据（据 `specs-apply.js`）：

| 内容所处位置 | 可达？ |
|---|:--:|
| `## Requirements` 内的 `### Requirement:` 块之间（含正文、Scenario、表格、脚注） | ✅ 可 `MODIFIED` |
| `## Purpose`（`## Requirements` 之前） | ❌ |
| `## Modules` / `## Key Files` / `## Test Definition` / `## White-box Test Ready`（`## Requirements` 之后） | ❌ |
| `## Requirements` 内、Requirement 块**之间**的松散段落 | ❌（且归档时是否保留取决于工具，**有风险**）|

### D2：破例手工编辑的**四条约束**

1. **最小范围** —— 只碰 delta 不可达的段落；可达的行一律走 delta
2. **可复核** —— 每处改动都在本变更文档中列出（见 `proposal.md` H1/H2 的两张表）
3. **写进 canonical 的注** —— 在 `reporters` 的 `## Modules` 表后加「Reporter_ID 真值来源」注，让读者知道真值在哪，而不只是看到被改过的数字
4. **归档后必须验证手工段落仍在** —— 因为 `openspec archive` 会重写 Requirements 区块，虽然不应触及其它节，但**必须实测**（见 `tasks.md` 3.6）

### D3：`samplers` 的松散段落（`## Requirements` 内、块之间）**风险最高**

该段落位于 R11 与 R12 之间，**在** `## Requirements` 区块内、但**不是** `### Requirement:` 块。它能否被 `openspec archive` 保留，取决于工具如何重建区块骨架 —— **未经验证**。

处置：本次照改，但把"归档后该段落是否仍在"列为**硬性验证项**（`tasks.md` 3.7）。若归档把它丢掉或改形，则记录并在 C 方案中一并解决。

### D4：C 方案（spec 布局重构）的边界

C 的目标：**把权威内容迁进 Requirement，使其可经 delta 维护**。建议的迁移对象与顺序：

| 优先级 | 迁移对象 | 建议形态 |
|---|---|---|
| P0 | 模块/编译清单（`## Modules` 表、samplers 编译表） | 各 capability 新增一条「R<n> 模块清单与构建状态」Requirement（含"真值来源 `src/Makefile`"与 Scenario） |
| P1 | 测试就绪声明（`## Test Definition` / `## White-box Test Ready`） | 并入对应 Requirement 的 Scenario 或单列一条 |
| P2 | `## Key Files` | 保留为附录，但**明确标注"非规范、仅导航"**（避免与 Requirement 冲突时被误当权威）|
| P3 | `## Purpose` 的叙述性内容 | 保留为 preamble；**约定 Purpose 内不写可失效的事实**（如具体脚本名、行号），只写稳定定位 |

P3 是关键：`Purpose` 永远不可达，因此应在**写作规范**上限制其内容类型，而非指望工具支持。

## Risks / Trade-offs

| 风险 | 等级 | 缓解 |
|---|---|---|
| 归档重写 Requirements 时把手工编辑的段落覆盖/丢弃 | **高** | `tasks.md` 3.6 / 3.7 硬性 grep 验证；不符则回退重来 |
| 手工编辑破例被后续效仿，导致 canonical 被随意手改 | 中 | D2 的四条约束 + D1 的可达性判据；C 方案落地后此类破例应归零 |
| delta 需整块复现 R1(58) + R5(39) + R10(25) + R12(26) = 148 行，抄写偏差会污染 canonical | 中 | 机械提取+替换+自检；替换均为**单行等长**，故验收判据为「新增行数 = 7 且四块行数不变」 |
| 11 处 ID 修正后，`reporters_loader.c` 的互斥 if-else 链是否也需改？ | 低 | **不需要** —— 代码用的是 `nReporterID` 与配置比对（运行期读 `System.cfg`），错的是**文档的数字**，不是代码逻辑。已在 `proposal.md` 明确"不改代码" |

## Context

补充背景：这些错映射长期未被发现，是因为它们**在阅读上自洽**（ID 与 Requirement 顺序一致，看起来合理）。发现它的唯一途径是**去查 `System.cfg` / `System.sql`** —— 这条经验已写入 `D1` 的可达性判据之外的审核方法：**凡 spec 出现具体 ID / 路径 / 行号，必须回溯到配置或源码核对，不能以其自身自洽为准。**
