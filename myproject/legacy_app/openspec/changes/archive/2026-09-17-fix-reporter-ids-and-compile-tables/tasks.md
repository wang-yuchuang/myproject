## 1. Delta 编写（**机械生成**，只覆盖 delta 可达的 7 行）

> delta 文件：`specs/reporters/spec.md` ← `## MODIFIED Requirements` + **R1 + R5 + R10 + R12**
> 提取区间（canonical，**手工编辑在 `## Modules`，不影响 Requirements 区间**）：
> R1 = `35..92`（58 行）、R5 = `185..223`（39 行）、R10 = `305..329`（25 行）、R12 = `347..372`（26 行）
> **7 处替换全部为单行等长** ⇒ 验收判据：`+` 内容行 = 7，且四块行数不变。

- [ ] 1.1 R1 的 **4 处**（独立开关表）

| # | canonical 行 | 原（行首片段） | 改为 |
|---|---|---|---|
| S-R1a | `:64` | `\| 2 (Web API) \| 1/47 \| …` | `\| 4 (Web API) \| 1/47 \| …` |
| S-R1b | `:66` | `\| 11 (ess_maintenance) \| 1/50 \| ✓ \|` | `\| 12 (ess_maintenance) \| 1/50 \| ✓ \|` |
| S-R1c | `:67` | `\| 12 (maintain_backend/mbe-mqtt) \| 1/51 \| ✓ \|` | `\| 3 (maintain_backend/mbe-mqtt) \| 1/51 \| ✓ \|` |
| S-R1d | `:69` | `\| 14 (es_iocpp) \| 1/57 \| ⚠️ 使用 \`valGet.val.ulValue\`…` | `\| 2 (es_iocpp) \| 1/57 \| ⚠️ 使用 \`valGet.val.ulValue\`…` |

- [ ] 1.2 R5 / R10 / R12 各 **1 处**（正文 ID）

| # | canonical 行 | 原（关键片段） | 改为 |
|---|---|---|---|
| S-R5 | `:187` | `…供本地浏览器查看和管理设备（Reporter ID = **2**）。` | `…（Reporter ID = **4**）。` |
| S-R10 | `:315` | `**子模块 B — 维护后端 MQTT 通道**（…mbe-mqtt/\`，Reporter ID = **12**）：` | `… = **3**）：` |
| S-R12 | `:349` | `…通过 Infypower 私有 iOCPP 协议上报数据（Reporter ID = **14**）。` | `…（Reporter ID = **2**）。` |

- [ ] 1.3 自检：`diff <提取块> <替换后文本>` 必须**只显示 7 处单行替换**；四块行数 58 / 39 / 25 / 26 不变

## 2. 破例手工编辑（delta **不可达**的段落）——**已完成**

- [x] 2.1 `reporters/spec.md` `## Modules` 表 **4 处 ID 修正**：`:381` Web 服务 `2→4`、`:382` ESS 运维 `11→12`、`:383` 维护后端 `12→3`、`:385` ES iOCPP `14→2`
- [x] 2.2 `reporters/spec.md` 表后新增「**Reporter_ID 真值来源**」注（指向 `src/config/System.sql:4320-4332`，并列出完整 13 项映射、指出**无 ID 5**）
- [x] 2.3 `samplers/spec.md` `:12` Purpose 段「共 16 个」→「**共 10 个**」+ 指向 `src/Makefile:36-47`
- [x] 2.4 `samplers/spec.md` `:23` 目录数「16」→「**17**」+ 注明 `thermal_control/` 无 `_sampler` 后缀
- [x] 2.5 `samplers/spec.md` `:25-44` 编译表重写：**10 ✅ / 7 ❌**，补 `sampler_thermal.so`，未编译项标注原因（Makefile 注释 / 未纳入 DIRS）
- [x] 2.6 `samplers/spec.md` `:544-561` 松散段落重写：删除「未详细记录的采样器（**R12–R16**）」「以下 **9 个**均正常编译」两处错误说法，改为准确的"已记录 R5–R14 / 未编译无 Requirement"两分清单
- [x] 2.7 ✅ `AGENTS.md:35`「7 个编译」→「**10 个编译**」；同时修正 `.code-facts-snapshot.md` 同源错误（原写 `36-44` / 「7 个编译」。已补全 3 个漏列模块并标注另有 5 个不在 DIRS、共 17 个子目录）

## 3. 验证

- [x] 3.1 ✅ **通过**：`Change 'fix-reporter-ids-and-compile-tables' is valid`
- [x] 3.2 ✅ **通过**：归档前 `9 passed`；归档后 `8 passed`
- [x] 3.3 ✅ **实测**：`| 2 (Web API)` = **1**、`Reporter ID = 2）` = **1**
- [x] 3.4 ✅ **已执行**：`reporters: update` / `~ 4 modified` / 归档为 **`2026-09-17-fix-reporter-ids-and-compile-tables`**
- [x] 3.5 ✅ **实测**：`| 2 (Web API)` = **0** ✓、`| 4 (Web API)` = **1** ✓、`Reporter ID = 14）` = **0** ✓、`Reporter ID = 3）` = **1** ✓
- [x] 3.6 ✅ **手工编辑存活（关键）**：`Reporter_ID 真值来源` = **1** ✓ —— 归档**未**破坏 Requirements 之外的节。且 `:381/:382/:383/:385` 新值 `4 / 12 / 3 / 2` 均在 ✓
- [x] 3.7 ✅ **松散段落存活**：`采样器清单与文档覆盖状态` = **1** ✓、`共 10 个` = **1** ✓；**`未详细记录的采样器` = 1 属预期**（⚠️ 原判据要求 0，**是我写错了** —— 该短语出现在**我自己新增的 corrections 注**中被引用，非残留）。归档**未**丢弃该段落 ✓
- [x] 3.8 ✅ **转录完整性**：`+` 内容行 = **7** ✓；四块行数守恒：R1 `33..90` = **58**、R5 `183..221` = **39**、R10 `303..327` = **25**、R12 `345..370` = **26** ✓。额外变更仅 2 处**空行规范化**（`## Requirements` 前后各一），属工具行为
- [x] 3.9 ✅ **通过**：6 个节全部保留（Purpose / Requirements / Modules / Test Definition / Key Files / Known Tech Debts）；除 2.1/2.2 的既定改动外未发现其它内容变更

### 3.10 归档后追加发现（供后续 delta 作者）

- ⚠️ **本变更的两次硬中止都是「我自己写错判据」**：① 期待 `未详细记录的采样器` = 0，但该短语被我自己的 corrections 注引用；② 假设「154 行 / 表头 63/103/129」而算术实为 **153 / 62/102/128**（多出的空行也是为迁就错数字而插）。
- ✅ **结论：写 delta 验收判据时，必须检查本次新增文本自身是否含被判定的旧字符串**（引用旧值做说明是常见做法，会让 grep 判据失效）。
- ✅ **另一条确认**：当 MODIFIED 触及**最后一个 Requirement** 时，其后的 `---` 分隔线可能被移除；当触及**区块首部**时，`## Requirements` 前后空行会被规范化 —— 两者均为工具行为，不应计入转录错误。

## 4. C 方案（后续独立变更）

- [ ] 4.1 `<FOLLOW-UP>` 立项 `restructure-specs-for-delta-reachability`：把模块/编译清单、测试就绪声明等**权威内容迁入 Requirement**，使其可经 delta 维护（迁移对象与优先级见 `design.md` D4）
- [ ] 4.2 `<FOLLOW-UP>` 在 `openspec/config.yaml` 的约定中**补写可达性判据**与"破例手工编辑"的四条约束（D1/D2），并把 `Purpose` 段的写作规范改为"**不写可失效的具体事实**"（D4 P3）
- [ ] 4.3 `<FOLLOW-UP>` `.code-facts-snapshot.md` 漂移清单新增："**Requirements 之外的节**（Modules / Key Files / Test）是否与实际一致"—— 本变更已证明这类内容会长期漂移且无人能经 change 修正
