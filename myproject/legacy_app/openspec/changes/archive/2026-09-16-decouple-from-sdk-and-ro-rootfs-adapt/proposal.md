## Why

legacy_app 的构建脚本（`scripts/`）全部是指向 `d9_sdk/source/test_app/scripts/` 的符号链接，没有 d9_sdk 权限的开发人员无法独立构建。同时 legacy_app 源码中有 119 处硬编码 `/var/volatile/` 路径引用和 `cp -f ... /usr/lib/` 运行时库拷贝操作，在 D9 SDK 只读 rootfs 环境下全部失败。

## What Changes

### 构建独立性
- 将 `scripts/` 下 5 个符号链接脚本替换为实体文件副本，断开对 d9_sdk 的路径依赖
- 更新 `scripts/UPSTREAM.md` 记录来源和同步策略

### 只读 rootfs 运行时适配
- 源码全局替换 `/var/volatile/` → `/opt/appdata/data/`（25+ 个 C/Shell 源文件）
- `run_app` 脚本将 `LD_LIBRARY_PATH` 增加 `/app/extensions` 路径
- 移除 `run_app` 中所有 `cp -f /app/extensions/lib*.so /usr/lib/` 命令（通过 LD_LIBRARY_PATH 加载）
- 同步修复 `startrun`、`mu3/run_app`、`mu3/startrun`、`run_app.debug`、`mu3/run_app.debug`

## Capabilities

### Modified Capabilities

- `app-image-builder`: 构建脚本自包含，不再依赖 d9_sdk 符号链接
- `legacy-app-boot`: 运行时路径从 `/var/volatile/` 迁移到 `/opt/appdata/data/`，库加载方式从 cp 到 /usr/lib 改为 LD_LIBRARY_PATH

## Impact

- **scripts/**: 5 个文件从符号链接变为实体文件，内容与 d9_sdk 上游同步
- **src/ 25+ 文件**: 路径字符串 `/var/volatile/` → `/opt/appdata/data/`
- **src/shells/run_app**: LD_LIBRARY_PATH 增加 extensions，移除 6 行 cp 命令
- **src/shells/startrun + mu3/**: LD_LIBRARY_PATH 同步增加 extensions
- **向后兼容**: 新构建的 app.img 与 d9_sdk 的 S91app_mount 完全兼容

---

## ⚠️ 归档前修订（2026-09-16）：delta 被重写并缩小

本变更于 2026-09-16 归档前，其 spec delta **被重写**。原因：原 delta 写于本仓 OpenSpec 化之前，指向两个**不存在**的 capability，且大部分内容已被后续 canonical spec 吸收。

### 原 delta 及其处置

| 原 delta 条目（位于 `specs/legacy-app-runtime-paths/spec.md`，`## MODIFIED`） | 覆盖判定 | 处置 |
|---|---|---|
| `Build scripts self-contained`（`scripts/` 不得依赖外部符号链接） | ❌ 无任何 canonical 覆盖（specs 中 `symlink`/`UPSTREAM` 命中 0） | **保留** → 重写为 `app-lifecycle` **R11**（`## ADDED`） |
| `Runtime paths on read-only rootfs` | ⚠️ 结果已被覆盖：`system-hand` 已记 PID 路径、`app-lifecycle` R7 `:260` 已记 `LD_LIBRARY_PATH` 含 `/app/extensions`；且该条目自身 Scenario 写着「`cp ... /etc/log4crc` succeeds (via bind mount)」，**与其自身 proposal 的「/etc 只读」相互矛盾** | **删除** |

### 修订说明

1. **`## MODIFIED` → `## ADDED`**：原 delta 的宿主 `legacy-app-runtime-paths` 在 `openspec/specs/` 下不存在。`specs-apply.js:145-148` 明确规定：「target spec does not exist; only ADDED requirements are allowed for new specs」。若照原样归档会**直接报错**
2. **宿主改为 `app-lifecycle`**：这是目前唯一的真实能力划分；R11 编号接在 R10 之后
3. **补 `R<n>` 前缀**：原 delta 用英文裸名（`### Requirement: Build scripts self-contained`），违反 `openspec/config.yaml:17` 的约定
4. **本 Proposal 正文的 `Capabilities → Modified Capabilities` 段（写 `app-image-builder` + `legacy-app-boot`）与 `Impact` 段保持原样**，作为历史记录；实际生效的 delta 见上表

### 后续影响

本 Proposal 与 `Impact` 段中的 `S91app_mount` 已过时 —— 该脚本已改名为 `S92app_mount`（见已归档变更 `fix-app-lifecycle-script-refs`）。作历史记录保留原文，不做回改。