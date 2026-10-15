## 1. Delta 编写（app-lifecycle 新增 R10）

- [x] 1.1 建立 `specs/app-lifecycle/spec.md` delta：`## ADDED Requirements` 含 **R10** 一条（新增 capability 为既有 `app-lifecycle`，故用 ADDED 而非 MODIFIED）
- [x] 1.2 R10 覆盖 `S91app_sysmgr` / `app_sysmgr`：位置与形态（脚本 1122 B / 二进制 52624 B / 4 线程 / fd 1,2→`/dev/console` / fd 3-15 rpmsg / PID 文件）、四项职责（BOOT_READY×3 握手、R 核信息查询、RAUC `MARK_GOOD`、跨核心跳）、与 app 启动链的边界
- [x] 1.3 写入三条**否定性契约**：① MUST NOT 启动 `app_entry`；② 应用启动 owner 为 `S92app_mount`(boot) / `app-post-install`(OTA)；③ 跨核心跳与应用心跳是两套独立机制、MUST NOT 混用
- [x] 1.4 记录 v1→v2 能力移除的**历史事实**（v1 `/data/app_sysmgr` 含 `/app/app_entry`、`Started app_entry (pid=%d)`、`/var/run/sysmgr.sock`；v2 对 `app_entry`/`nccu_app`/`syshand`/`run_app` 的 `strings` 匹配数均为 0），并标注「供追溯」
- [x] 1.5 6 个 Scenario：boot 早期拉起 / Phase 1 BOOT_READY / Phase 2 R 核信息查询 / RAUC 槽位标记 / 跨核心跳 / 不承担应用启动职责

## 2. 明确排除项

- [x] 2.1 **不新建 capability `app-sysmgr`**（理由见 design.md D1：`app_sysmgr` 是 BSP 组件，与 `S92app_mount` 同属「被记入 app-lifecycle 的 BSP boot 契约」一类）
- [x] 2.2 **不描述实现细节**：不写内部线程职责划分、rpmsg 消息格式、重试计数器具体数值（design.md D2）
- [x] 2.3 不改动 `app-lifecycle` 既有 R1-R9 的任何文字；不重编号
- [x] 2.4 不改任何代码（`app_sysmgr` 属 BSP 仓）；不要求 BSP 做任何改动
- [x] 2.5 不搬运 `.code-facts-snapshot.md` 的运维细节（历史残留文件、日志路径等）—— 快照是事实基线，spec 是契约，二者分工不混

## 3. 验证

- [ ] 3.1 `openspec validate document-app-sysmgr` 通过
- [ ] 3.2 `openspec validate --all` 全绿
- [ ] 3.3 归档**前**记录基线：`grep -c '^### Requirement' openspec/specs/app-lifecycle/spec.md` → 预期 **9**（R1-R9）
- [ ] 3.4 执行 `openspec archive document-app-sysmgr -y`
- [ ] 3.5 归档**后**复验：同一命令 → 预期 **10**（新增 R10）；且 `grep -n 'R10'` 命中新 Requirement 标题
- [ ] 3.6 **转录完整性检查**：`git diff openspec/specs/app-lifecycle/spec.md` 应仅含 **R10 块的纯新增**（无删行、无 R1-R9 任何行内改动）。允许出现 `openspec` 自身的空行规范化（该工具重写 canonical 时会规范化 `## Requirements` 周边空行并保证 EOF 换行 —— 详见已归档变更 `fix-app-lifecycle-script-refs` 的 tasks.md 3.7）；若出现 R1-R9 的**行内**改动则必须回退重做
- [ ] 3.7 复核 R9 之后追加 R10 的位置正确，且 R10 的 6 个 Scenario 全部保留

## 4. 待办 / 关联

- [ ] 4.1 `<FOLLOW-UP>` BSP 侧若计划恢复「启动 `app_entry`」能力，须先与 `S92app_mount` 的 boot 接线对齐 owner 语义（否则形成两个启动者）；已在 `.code-facts-snapshot.md` 漂移清单登记
- [ ] 4.2 `<FOLLOW-UP>` BSP 侧改名残留（`S92app_mount` 自身头注释 `# /etc/init.d/S91app_mount`、`app-post-install` 6 处注释、`etc/keys/app-trust/README.md:14`）需在主仓 openspec 立项清理 —— 纯注释、不影响功能
- [ ] 4.3 `<DEFERRED>` `app-lifecycle` Purpose 段仍有 2 处 `S91app_mount`（`:18`/`:20`），delta 机制无法覆盖（`## Purpose` 属 preamble，非 Requirement）；待 canonical 整体重写时处理。详见 `fix-app-lifecycle-script-refs` 的 tasks.md 4.1
