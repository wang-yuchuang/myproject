## Why

`openspec/specs/app-lifecycle/spec.md` 共 **9 处**引用了 `S91app_mount` 这个脚本名，而该文件在 **bmu05 上根本不存在** —— 它已改名为 `S92app_mount`，`S91` 槽位被新脚本 `S91app_sysmgr` 占用。

canonical spec 因此对"启动时谁挂载 `/app`、谁调用 `app_entry`"给出**错误答案**，并与同一份 spec 的 R7 自相矛盾（R7 已由 `keep-app-alive-on-bmu05` 改为 `S92app_mount`）。

### 真机与源码证据（2026-09-16 实测）

| # | 证据 | 内容 |
|---|---|---|
| E1 | bmu05 `/etc/init.d/` 实测 | 与 app 相关的只有 `S91app_sysmgr`（1122 B, Sep 11 06:33）与 `S92app_mount`（20196 B, Sep 6 08:22）；**不存在 `S91app_mount`** |
| E2 | `S92app_mount` 自身第 2 行 | `# /etc/init.d/S91app_mount` —— 改名后未同步自己的头注释 |
| E3 | BSP 源码 `source/buildroot/def_buildroot_rootfs/etc/init.d/` | 只有 `S92app_mount`，无 `S91app_mount` |
| E4 | 旧 build 变体产物 | `ap1_bmu02_a00_buildroot`、`ap1_bmu03_a00_buildroot`、`ap1_bmu05_a00_buildroot`(ospi2_emmc) 内为 `S91app_mount` |
| E5 | 新 build 变体产物 | `ap1_bmu05_a00_buildroot`(emmc_only) 与 (ospi2_xip_emmc) 内为 `S92app_mount` —— 印证改名时点 |
| E6 | `usr/lib/rauc/app-post-install` | 6 处注释写"与 S91app_mount 一致 / 保持一致"（同属改名残留） |
| E7 | `etc/keys/app-trust/README.md:14` | "S91app_mount 根据 app 分区 metadata…" |
| E8 | 子仓 `source/test_app/AGENTS.md:28` | 已明确记载：**"`app_entry` 头部注释仍写 S91app_mount，实际脚本已改名 S92app_mount，注释过时勿照抄"** |

**改名原因**：新脚本 `S91app_sysmgr`（`/usr/sbin/app_sysmgr`，AP↔R 跨核管理守护）需要占用 S91 槽位并抢在 app 挂载之前启动，老的 `S91app_mount` 因此让位到 **S92**（跑在 sysmgr 之后）。

### 9 处引用的分布

| 位置 | 行 | 本次是否处理 |
|---|---|---|
| Purpose 段（阶段三 + 范围说明） | 18, 20 | ❌ 见下方"已知限制" |
| **R4** dm-verity 完整性校验 | 136, 141 | ✅ MODIFIED |
| **R6** RAUC OTA Bundle 生成 | 221 | ✅ MODIFIED |
| **R7** 运行时启动链 | 229, 246 | ➖ 已由 `keep-app-alive-on-bmu05` 的 R7 delta 覆盖（该 delta 内 `S91app_mount` 计数为 **0**） |
| **R9** 优雅停止与信号处理 | 350, 366 | ✅ MODIFIED |

## What Changes

- 在 `app-lifecycle` capability 下**修改 3 条既有 Requirement**（R4 / R6 / R9），仅将 `S91app_mount` 替换为 `S92app_mount`，**语义、Scenario、行为描述一字不改**
- **不改动任何 C 代码、任何 shell 脚本** —— 这是纯文档校正
- **不触碰 R7**：R7 已在 `keep-app-alive-on-bmu05` 中 MODIFIED（该 delta 已全部使用 `S92app_mount`）。两个 change 同时 MODIFY 同一 Requirement 既无必要也会造成归档顺序耦合
- **不新增 Requirement**，不动 R1/R2/R3/R5/R8

### 已知限制（需后续处理，本变更不覆盖）

**Purpose 段第 18、20 行无法通过 delta 机制修正。** `openspec` 的 delta 仅支持 `ADDED / MODIFIED / REMOVED / RENAMED Requirements` 四种操作，作用域限定在 `## Requirements` 区块内（`dist/core/specs-apply.js:164-169` 使用 `extractRequirementsSection()` 取区块），`## Purpose` 属于前言（preamble）而非 Requirement。而项目约定（`openspec/config.yaml`）明确要求 canonical spec **不得手工编辑**。

因此 Purpose 段那 2 处 `S91app_mount` 只能等**下一次 canonical 整体重写**时一并处理。已登记为 `tasks.md` 中的 `<DEFERRED>` 项，并在 `design.md` D3 中给出可选方案。

## Capabilities

### New Capabilities

- (无)

### Modified Capabilities

- `app-lifecycle`：修改 R4「dm-verity 完整性校验」、R6「RAUC OTA Bundle 生成」、R9「优雅停止与信号处理」三条 Requirement 中的 `S91app_mount` → `S92app_mount`

## Impact

- **文档**：`openspec/specs/app-lifecycle/spec.md`（由 `openspec archive` 依据 delta 生成）
- **代码**：无变更
- **BSP 侧**：无变更（本变更不修改 BSP 契约文件，仅校正本仓 spec 对它们的称呼）
- **依赖**：无变更
- **风险**：极低 —— 纯字符串替换，无行为语义变化；但**归档前需确认 `keep-app-alive-on-bmu05` 的归档顺序**（两者作用于同一文件的不同 Requirement，顺序无关，先归档任一均可）
