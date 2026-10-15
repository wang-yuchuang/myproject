# 技术债索引（tech-debts）

> **性质**：索引
> **需求权威源**：暂无（索引类，无需求源；逐条依据指向 spec / 快照 / 归档变更）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`507729a`（dev，2026-09-17 核对）
> **状态**：✅ 已核对（T1至T8 经代码与快照复核）

## 用途与口径

收录**已知、可命名、有出处**的技术债，按类别归组。本表是**索引**：逐条证据以 `openspec/.code-facts-snapshot.md` 与对应 spec/归档变更为准，本表不复制其论证。"处置建议"只给方向与去向，不立验收断言；立项与方案细节走 `openspec/changes/`。

## 债务登记表

| # | 类别 | 债务 | 影响 | 处置建议/去向 | 依据 |
|---|---|---|---|---|---|
| T1 | 结构性 | OpenSpec delta 只能修改 `## Requirements` 区块内的 Requirement 块；`## Purpose`、Requirements 之后的 `## Modules`/`Key Files`/`Test Definition`/`Known Tech Debts`、块间松散段落、spec 头部元数据（Version/Last Updated）均不可经 change 修正 | 已实证后果：`reporters` 的 `## Modules` 表 4 个 Reporter_ID 错映射数月无法修正；`samplers` 编译清单出现三个互相矛盾的值（spec 16 / 快照 7 / 真值 10）；每次改 spec 均需"破例手工编辑"+归档后存活验证 | 方案已规划：把模块/编译清单等权威内容迁入 Requirement（使其可经 delta 维护），分 P0至P3 阶段；⚠️ 本仓 OpenSpec 拒绝纯规划型 change（validate 要求至少一个 delta），**立项时必须同步带上一条真实 delta** | `.code-facts-snapshot.md:448-451`；归档 `2026-09-17-fix-reporter-ids-and-compile-tables/design.md` D1至D4、`tasks.md` 4.1至4.3 |
| T2 | 死代码 | `src/publib/iCfg_Parser.c`（实测 1012 行）不编译（OBJS 外）、无引用；`src/moduals/reporters/modbustcp2_reporter/` 不在 `src/Makefile`（连注释行都没有） | 误导阅读与审计（"有代码就有功能"的误判源之一，见 known-issues K6）；增大仓库噪音 | 确认无回用计划后删除；若 `iCfg_Parser` 将来重新编译，publib spec R2 需恢复（见快照漂移清单中优先级条目） | `wc -l src/publib/iCfg_Parser.c` = 1012；`grep modbustcp2 src/Makefile` 无匹配；`.code-facts-snapshot.md:453` |
| T3 | 测试 | 全仓零测试：45 个 Makefile 的 `TEST_EXEC` 全部为空或整行注释（如 `TEST_EXEC = #test_scu`） | 回归全靠真机手测；重构与平台移植无安全网 | 已有方向：从 publib 的纯函数起步建测试基线，再逐步覆盖 DPR 表达式/告警逻辑（见 publib spec R1/R5 的白盒就绪声明） | `grep -rn 'TEST_EXEC' src/ --include=Makefile`（45 处全空/注释）；`openspec/specs/publib/spec.md` R1/R5 |
| T4 | 平台抽象 | 平台差异靠 `#ifdef IMMU3` 编译宏 + `src/shells/mu3/` 双份启动脚本承载，未配置化 | 双源漂移：改根版漏 mu3（或反之）；新平台接入需再复制一套（关联 known-issues K4） | 债务 A1：用户决策暂不处理、先标记；未来改为运行时配置驱动，配置源可挂 data-processor 的 SQLite 配置体系 | `src/shells/mu3/`（startrun/run_app/stoprun/modem_reset 双份）；`grep -rln IMMU3 src/`（main.c、config_mangmt.c、reporters_loader.c 等 10+ 文件） |
| T5 | 文档治理 | `docs/` 不被 OpenSpec CLI 校验（CLI 只认 `openspec/specs/`），人读文档的准确性无工具保障 | 文档漂移只能靠人工核对发现 | 依靠权威契约头自律（每篇必带头部+对应提交）+ 定期按漂移清单核对；权威边界见 `docs/README.md` 的「权威源声明」 | `docs/_meta/authority-contract.md`；`docs/README.md` 权威源声明表 |
| T6 | 安全/构建 | `build.sh:23-24` 硬编码签名服务器开发网 IP（`http://192.168.34.207:8900`）与签名 token 字面量（`SIGN_TOKEN="token-d9-sdk-001"`） | 离开开发网段无法出包；token 随源码扩散（关联 known-issues K8） | 改为环境变量/构建参数注入，IP 与 token 出配置；生产化前确认签名服务可用性 | `build.sh:23-24,122-123,150-151`；`.code-facts-snapshot.md:444` |
| T7 | 大文件 | `src/publib/iBasic_Funs.c` 实测 2739 行（AGENTS.md 记 2738，以实测为准），混合 syshand 客户端 SDK（`_SYSTEM`/`Do_Proc_HeartBeat`）+ OS 工具函数 + 业务常量 | 单文件改动面大、职责边界模糊，评审与测试定位成本高 | 拆分去向：syshand 客户端 SDK 独立成模块（对接 `system-hand` spec）、OS 工具与业务常量各自归位；拆分前先有 T3 的字符/工具函数测试基线 | `wc -l src/publib/iBasic_Funs.c` = 2739；`openspec/specs/publib/spec.md` R7；AGENTS.md 已知技术债第 2 条 |
| T8 | 配置 | `System.sql` 中存在"声明但未编译"的采样器行（配置声明与 `src/Makefile` 编译清单脱节） | 配置里看得见、运行时实际不存在，排障时易把"配置有"当成"功能在" | `keep-app-alive-on-bmu05` 已列为 Non-Goal（另立变更处理）；处置时以 `src/Makefile` 编译清单为真值对齐 `System.sql` | `keep-app-alive-on-bmu05/design.md` Non-Goals 第 4 条；`.code-facts-snapshot.md:300-316` |
| T9 | 文档治理 | spec-code drift：app-lifecycle R13（trace.log 重定向）与 R10（boot 启动 owner 措辞）已与代码不符；`docs/` 已按代码修正，spec 文本待变更流程同步 | 按 spec 阅读会得到与代码相反的行为预期（如「boot 路径会启动应用」、非 mu3 的 stdout/stderr 也落 trace.log） | 走 openspec change 流程修正 R10/R13 措辞；spec 修正前，`docs/` 相关页已用 ⚠️ 标注 drift 并指向本表 | `docs/10-architecture/ota-and-ab.md`、`docs/60-operations/logs-and-db.md`（drift 注记）；`openspec/specs/app-lifecycle/spec.md` R10/R13 |

## 与 AGENTS.md「已知技术债」的映射

AGENTS.md 的速查清单是本表的摘要视图：其第 2 条（iBasic_Funs 混合职责）→ T7；第 3 条（零测试）→ T3；第 4 条（死代码）→ T2；第 5 条（签名服务器写死）→ T6。第 1 条（reboot 结案）已移入 `known-issues.md` 的「已结案存档」。内容重复时以本表为准（更细），AGENTS.md 只保留一行式速查。

## 类别口径

- **结构性**（T1）：工具链/流程层面的系统性限制，单次改动修不掉，需要专项立项
- **死代码/配置脱节**（T2、T8）：仓库内与真值不符的残留，偿还手段是"对齐或删除"
- **工程能力缺口**（T3至T7）：测试、平台抽象、文档治理、构建安全、代码组织，需要渐进投入

## 快照漂移清单对照

本表条目与 `.code-facts-snapshot.md` **Drift Detection Checklist** 的对应关系（复查时两表联动）：

- T1 ↔ 中优先级第 1 条（🔴 结构性债：delta 不可达区）
- T2 ↔ 中优先级第 2 条（iCfg_Parser 是否仍在 OBJS 外）
- T6 ↔ 高优先级第 5 条（build.sh SIGN_URL 是否仍为开发 IP）
- T8 ↔ samplers 编译清单事实（:300-316）与配置/编译对齐类条目

## 关键数字的验证命令（2026-09-17 实测）

```bash
wc -l src/publib/iCfg_Parser.c src/publib/iBasic_Funs.c   # 1012 / 2739
grep -rn 'TEST_EXEC' src/ --include=Makefile              # 45 处，全为空或整行注释
grep -n 'modbustcp2' src/Makefile                          # 无匹配
grep -n 'SIGN_URL\|SIGN_TOKEN' build.sh                    # :23-24 硬编码
ls openspec/specs/                                         # 7 项，无 DAI
```

## 偿还顺序参考（非强制）

1. T1 最优先：它决定其它 spec 类工作（含 T8 对齐、模块清单维护）的修改成本，且方案已成型
2. T3 测试基线先行于 T7 拆分：先锁行为、再动结构
3. T2/T8 是低成本的"消除误导"动作，可随时搭车
4. T4/T5/T6 跟随平台迁移与生产化节点推进

## 登记与销账

新增债务需带 `file:line` 或 spec/归档依据与核实日期；销账（还清）时注明去向（删除提交、归档变更链接）。与 `known-issues.md` 的分工：**问题**是"现在坏了什么"（用户可感知），**债务**是"代码与流程欠了什么"（工程视角）；同一根源两边都可登记，互相以 K#/T# 交叉引用。不在本表登记：一次性事实错误（走 openspec change 修复即销）、纯风格问题（无行为影响且无依据价值）。

> 本表是索引，逐条证据以 `openspec/.code-facts-snapshot.md` 与对应 spec/归档为准。
