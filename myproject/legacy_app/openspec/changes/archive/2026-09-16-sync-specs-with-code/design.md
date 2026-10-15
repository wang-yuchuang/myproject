## Context

`openspec/specs/` 的 7 份 spec 是**手写**产物（从未经过 `openspec archive`），内容基线停在 2026-07-14。其后代码前进了 18 个提交，samplers 子系统新增了三块已交付能力。本次既要补内容，也要把生成路径切回 CLI 管控。

## Goals / Non-Goals

**Goals**
- 让 `samplers` capability 覆盖全部 10 个已编译采样器
- 用一次真实的 change + archive 建立可复制的归档样板

**Non-Goals**
- 不重写既有 R1-R11 的正文（除骨架改名外，上一轮已完成且已通过校验）
- 不为未编译的历史模块（`aems` / `imeu` / `pv_inverter` / `charger` / `can2_metershare` / `cardreader` / `inter_slave`）新增 spec
- 不触碰 C 代码

## Decisions

### D1：三个采样器放进既有 `samplers` capability，而非各建新 capability

与现有结构保持一致 —— 当前 R5-R11 已是「采样器框架 + 逐个采样器」的混合模型，每个采样器一条 Requirement。新开 3 个 capability 会让 `specs/` 目录碎片化，且框架类需求（R1-R4）无法复用。

*替代方案*：每个采样器一个 `specs/<name>/spec.md`。否决，理由同上。

### D2：用 `ADDED` 而非 `MODIFIED`

`spec-structure.js` / `specs-apply.js:69-91` 要求 MODIFIED 必须匹配 canonical spec 中已存在的 Requirement 名。aircon / auxdev / thermal_control 在 canonical spec 中**没有** Requirement（只有表格提及），因此只能用 ADDED。

### D3：`reporters` 不做 delta

审计发现 R6-R9 已各自带编译状态说明（`reporters/spec.md:229,255,272,285`），`ess_maintenance_reporter` 已作为 R10 子模块 A 覆盖（`:309-322`）。**"spec 描述了未编译模块"不是缺陷，而是有意的现状标注**。故 reporters 无 delta。

### D4：`openspec/project.md` 不创建

`legacy-cleanup.js:190,251,496-510` 显示本版 OpenSpec 将 `openspec/project.md`（与 `openspec/AGENTS.md`）视为 **legacy**，内容应迁移到 `openspec/config.yaml` 的 `context:` 字段。因此项目上下文改由 config.yaml 承载，不新建 project.md。

## Risks / Trade-offs

| 风险 | 缓解 |
|---|---|
| Requirement 名带 `R<n>` 前缀，若后续重排编号会与 canonical 名失配 | 保持「只追加、不重排」约定；改名需走 RENAMED delta |
| delta 正文引用了代码行号，代码变动后会漂移 | 这是既有一贯风格（R1-R11 同样引用行号）；由 `/code-doc-sync` 定期对齐 |

## Migration Plan

1. `openspec validate sync-specs-with-code` → 通过
2. `openspec archive sync-specs-with-code -y` → canonical spec 追加 R12/R13/R14
3. `openspec validate --all` → 复验 10/10
