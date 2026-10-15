## Why

本轮 spec 审核发现两类问题。第一类是数据错（**可立即修正**），第二类暴露了一个**结构性障碍**（更重要）。

### H1 — `reporters`：**11 处 Reporter_ID 错映射**

真值来源 `src/config/System.sql:4320-4332`（`Charger_Reporter` 表）：

| 位置 | 现文 | 应为 |
|---|:--:|:--:|
| `:64` R1 独立开关表 | `2 (Web API)` | **4** |
| `:66` R1 独立开关表 | `11 (ess_maintenance)` | **12** |
| `:67` R1 独立开关表 | `12 (maintain_backend/mbe-mqtt)` | **3** |
| `:69` R1 独立开关表 | `14 (es_iocpp)` | **2** |
| `:187` R5 正文 | `Reporter ID = 2` | **4** |
| `:315` R10 正文 | `Reporter ID = 12` | **3** |
| `:349` R12 正文 | `Reporter ID = 14` | **2** |
| `:381` Modules 表 | Web 服务 `2` | **4** |
| `:382` Modules 表 | ESS 运维 `11` | **12** |
| `:383` Modules 表 | 维护后端 `12` | **3** |
| `:385` Modules 表 | ES iOCPP `14` | **2** |

**错误规律**：独立开关表与 Requirement 正文里的 ID 是**按"Requirement 出现顺序"推定的**，而非查配置所得。⚠️ 注意 `:51-56` 的平台互斥表（`6/7/8/9`）**是对的** —— 所以不是全局偏移，是逐处错。

### H2 — `samplers`：编译清单三个版本互相矛盾

| 来源 | 声称 | 真值 |
|---|:--:|:--:|
| `samplers/spec.md:12`/`:25` | **16** 个 `.so` | **10** |
| `.code-facts-snapshot.md:311` | 7 个 | **10** |
| `AGENTS.md:35` | 7 个 | **10** |
| **`src/Makefile:36-47`** | — | **10 编译 + 7 未编译** |

且 spec 的编译表**漏了 `sampler_thermal.so`**（R14 的模块），并把 7 个未编译模块列成编译产物；另有一段**松散段落**（`:544-561`）写着"未详细记录的采样器（**R12–R16**）…以下 **9 个**均正常编译"—— 两处均错。

### H3 🔴 结构性发现：**canonical 里三类内容无法被 change delta 修改**

读 `specs-apply.js` 确认：delta 的作用域**只有 `## Requirements` 区块**（`extractRequirementsSection()` + 按 `### Requirement:` 块操作）。因此：

| # | 内容类别 | 受影响实例 |
|---|---|---|
| 1 | `## Purpose`（preamble） | app-lifecycle 的 2 处 `S91app_mount` + 1 处 reboot 技术债 |
| 2 | **Requirements 之后的节**：`## Modules` / `## Key Files` / `## Test Definition` / `## White-box Test Ready` | reporters `:376-390` 的 4 处错 ID；samplers 的编译表 |
| 3 | **`## Requirements` 内、Requirement 块之间的松散段落** | samplers `:544-561` 那段 |

**这解释了长期现象**：v0.3 升级把 Requirements 修得很准，但 Modules / Key Files 表**从未被 revision 过** —— 因为它们**结构上就改不了**。

## What Changes

- **`reporters`（经 delta，7 行）**：`## MODIFIED Requirements` —— R1 独立开关表 4 行 + R5 / R10 / R12 正文各 1 行
- **`reporters`（破例手工编辑，4 行 + 1 注）**：`## Modules` 表 4 处 ID + 增加"Reporter_ID 真值来源"说明
- **`samplers`（破例手工编辑）**：`:12` 与 `:25` 的"16 个"→ 10；`:23` 的目录数 → 17；重写编译表（10 ✅ / 7 ❌，补 `sampler_thermal.so`）；重写松散段落（`:544-561`）
- **`AGENTS.md`（直接编辑）**：`:35` 的"7 个编译" → 10
- **不改任何代码**
- **纳入 `.code-facts-snapshot.md` 的漂移清单**：新增"delta 不可达内容"检查项

### ⚠️ 关于"破例手工编辑"的显式记录

项目约定（`openspec/config.yaml`）写明 canonical **不得手工编辑**。本变更**有意破例**，理由与边界：

1. **这些内容结构上无法经 delta 修改**（H3）—— 不破例就只能让 11 处错 ID 与错误编译清单**永久留存**，而它们**会直接误导实施**（照错 ID 配模块）
2. **破例范围最小化**：仅限 delta **不可达**的段落；所有 delta 可达的行仍走 `## MODIFIED`（7 行）
3. **全部留痕**：本 `proposal.md` / `design.md` 记录每处改动与理由；canonical 内也加了「真值来源」注
4. **归档后必须复核**：`openspec archive` 只重写 Requirements 区块，手工编辑的段落**应被保留**——归档后需 grep 验证（见 `tasks.md`）

## Capabilities

### New Capabilities

- (无)

### Modified Capabilities

- `reporters`：R1 独立开关表、R5 / R10 / R12 正文的 Reporter_ID 校正（delta）+ `## Modules` 表校正（手工）
- `samplers`：Purpose 段与编译清单、松散段落的编译事实校正（手工，无 delta 可达内容）

## Impact

- **文档**：`openspec/specs/reporters/spec.md`、`openspec/specs/samplers/spec.md`、`AGENTS.md`、`openspec/.code-facts-snapshot.md`
- **代码**：无变更
- **关联**：本变更同时是 **C 方案（重构 spec 布局，使权威内容进入 Requirement）** 的动因与依据
- **风险**：中 —— 含手工编辑，且归档会重写 Requirements 区块；以"归档后 grep 复核手工段落仍在 + delta 行数守恒"为验收判据
