## Why

7 份 canonical spec 在 2026-07-14 定型后，代码又前进了 18 个提交（`fb9687a1` → `9b3a00b3`）。其中 samplers 子系统新增/扩展了 **3 个已编译、已配置、已在镜像中运行**的采样器，但 spec 没有任何 Requirement 覆盖它们：

| 采样器 | Sampler_ID | 产物 | 编译 | 配置 | spec 覆盖 |
|---|---|---|---|---|---|
| `aircon_sampler` | 11 | `sampler_aircon.so` | `src/Makefile:45` | `System.sql:4346` | 仅 Modules 表 + 脚注 |
| `auxdev_sampler` | 12 | `sampler_auxdev.so` | `src/Makefile:46` | `System.sql:4347` | 仅 Modules 表 + 脚注 |
| `thermal_control` | 13 | `sampler_thermal.so` | `src/Makefile:47` | `System.sql:4348` | **零提及** |

这三个模块承载了本轮新增的液冷/风冷热管理业务（同飞 TF 故障位图、U3Z 水冷机 + IMLU2 灯带、公式化 `T_decision` 策略），属于**已交付但未归档**的能力，是当前 spec 与代码之间最大的覆盖缺口。

同时，本仓库的 `openspec/changes/archive/` 一直是空的——两个标记为 Complete 的变更从未归档，canonical spec 是**手写**的，这正是上一轮出现 `## Overview` / `### R1:` 骨架错位的根因。本次变更同时作为**首次归档演练**，把 delta 正式合并进 canonical spec。

## What Changes

- 在 `samplers` capability 下**新增 3 条 Requirement**（R12 / R13 / R14），覆盖 aircon / auxdev / thermal_control 的采集范围、设备与参数映射、关键机制
- **不改动任何 C 代码**，不改动既有 R1-R11
- 审计结论：`reporters` capability **无需改动** —— R6-R9（IEC104/101/61850/OCPP）已各自带 `> **编译状态**：…未纳入常规编译` 标注，`ess_maintenance` 已作为 R10 的子模块覆盖

## Capabilities

### New Capabilities

- (无 — 三个采样器均归属既有 `samplers` capability，与现有 R5-R11 同构)

### Modified Capabilities

- `samplers`: 新增 R12「空调采样（aircon_sampler）」、R13「辅助设备采样（auxdev_sampler）」、R14「热管理策略采样（thermal_control）」三条 Requirement

## Impact

- **文档**：`openspec/specs/samplers/spec.md`（由 `openspec archive` 依据 delta 生成）
- **代码**：无变更
- **依赖**：无变更
- **流程**：`openspec/changes/archive/` 首次产生内容，spec 生成回到 CLI 管控路径
