## 1. Delta 校正（R4 / R6 / R9）

- [x] 1.1 建立 `specs/app-lifecycle/spec.md` delta：`## MODIFIED Requirements` 内含 R4 / R6 / R9 三条的**完整正文**（MODIFIED 是整块替换，必须自带全文）
- [x] 1.2 R4「dm-verity 完整性校验」：Scenario `运行时 verity 挂载（SDK 侧，非本仓库）` 的 WHEN 行 + 其后的「注意」脚注，共 **2 处** `S91app_mount` → `S92app_mount`
- [x] 1.3 R6「RAUC OTA Bundle 生成」：Scenario `RAUC 安装（设备端）` 末行 `MUST 先执行 S91app_mount stop` → **1 处**改为 `S92app_mount`
- [x] 1.4 R9「优雅停止与信号处理」：Scenario `SIGTERM 触发停止` 的 WHEN 行 + Scenario `PID 文件管理` 的 THEN 行，共 **2 处**改为 `S92app_mount`
- [x] 1.5 全文逐字比对确认：除上述 5 处标识符外，与 canonical 原文**零差异**（正文 / Scenario / 表格 / 脚注均未改动）

## 2. 明确排除项

- [x] 2.1 **不触碰 R7**：canonical `:229`、`:246` 的 2 处已由 `keep-app-alive-on-bmu05` 的 R7 delta 覆盖（`grep -c S91app_mount` 该 delta = **0**）。两个 change 作用于不同 Requirement，归档顺序无关
- [x] 2.2 不修 BSP 侧改名残留（`S92app_mount` 头注释、`app-post-install` 6 处注释、`app-trust/README.md:14`、子仓 `test_app/AGENTS.md` 之外的引用）—— 属 BSP 仓，需在主仓 openspec 另立变更
- [x] 2.3 不改任何 C 代码 / shell 脚本 / 构建脚本；不新增 Requirement；不重编号
- [x] 2.4 不顺带修改 R6 中写死的 `compatible=bmu02_a00`（独立议题，见 design.md D1）

## 3. 验证

- [x] 3.1 ✅ **通过**：`Change 'fix-app-lifecycle-script-refs' is valid**
- [x] 3.2 ✅ **通过**：`Totals: 12 passed, 0 failed (12 items)`（7 spec + 5 change）
- [x] 3.3 ✅ **实测 9**，位于行 `18 / 20 / 136 / 141 / 221 / 229 / 246 / 350 / 366`（与 proposal.md 的分布表完全一致）
- [x] 3.4 ✅ **已执行**：`openspec archive fix-app-lifecycle-script-refs -y` → `app-lifecycle: update` / `Totals: + 0, ~ 3, - 0` / 归档为 `2026-09-16-fix-app-lifecycle-script-refs`
- [x] 3.5 ✅ **实测 4**（与"`keep-app-alive-on-bmu05` 尚未归档"一致）：剩余于 `Purpose :18/:20` + `R7 :227/:244`；同时 `S92app_mount` 计数由 **0 → 5**，正是预期的 5 处替换
- [x] 3.6 ✅ **R7 未被触碰**：其内 2 处 `S91app_mount` 仍在 `:227` / `:244`，行段 `221–279` 与原文一致（`keep-app-alive-on-bmu05` 未归档，故 canonical R7 应保持原样）
- [x] 3.7 **转录完整性检查（最重要）** ✅ **通过 —— 但原判据写错了，已修正**
  - 归档后 `git diff --unified=0 openspec/specs/app-lifecycle/spec.md` 实测 = **5 处 `S91`→`S92` 单行替换**（R4 `:136/:141`、R6 `:221`、R9 `:350/:366`）**+ 3 处纯空行规范化**（`## Requirements` 前后各去 1 空行、EOF 补 1 空行）
  - 后 3 处是 **`openspec` 自身的输出格式**，不是抄写错误。铁证：`samplers/spec.md`（更早经同一工具归档）具有**完全相同**的结构 —— `45:空 / 46:--- / 47:## Requirements / 48:### Requirement: R1`，与本 spec 改后的 `24:--- / 25:## Requirements / 26:### Requirement: R1` 逐位一致
  - 原判据"不得出现任何其它增删"**过于严格**（写任务时我误假设工具只改 Requirement 行）。**有效判据应为：不得出现任何行内内容差异 + R4/R6/R9 块行数不变**（由 3.8 覆盖）
  - **重要教训（供后续 delta 作者参考）**：`openspec` 重写 canonical 时会顺手规范化 `## Requirements` 周围的空行并保证 EOF 换行。审阅 `git diff` 时**不要把这三处当作自己的错误**
- [x] 3.8 ✅ **R4/R6/R9 块行数均未变**：R4 `110–140` = **31**、R6 `181–220` = **40**、R9 `341–367` = **27**，与归档前的 `112–142` / `183–222` / `343–369` 长度一一相等（仅整体上移 2 行）

## 4. 延后项

- [ ] 4.1 `<DEFERRED>` **Purpose 段 2 处引用无法经 delta 修正**（delta 仅支持 Requirement 级操作，`## Purpose` 属 preamble）。canonical `:18`、`:20` 的 `S91app_mount` 留待下一次 canonical 整体重写时处理；方案对比见 `design.md` D3。**实测：归档后全文 `S91app_mount` = 4（Purpose 2 + R7 2），非 2** —— R7 那 2 处待 `keep-app-alive-on-bmu05` 归档后自会消失，最终存留为 2（仅 Purpose），属预期行为、不是归档失败
- [ ] 4.2 `<FOLLOW-UP>` 向主仓 openspec 提出 BSP 侧改名残留清理变更（`S92app_mount` 自身头注释 / `app-post-install` / `app-trust/README.md`）

## 5. 归档时 CLI 输出的提示（非阻塞，均已知）

- [x] 5.1 ✅ `⚠ Why section should not exceed 1000 characters`（`proposal.md`）—— **接受该警告**。根因：Why 段承载 E1–E8 八条证据表，是本变更的论证主体；拆到 `design.md` 会削弱 Why 的自洽性，且两处现存完整版本、互为索引，读者可交叉核对。若后续要求零警告，可在下次文档整顿时把证据表整体迁至 `design.md`，Why 内只留一行摘要
- [x] 5.2 ✅ `10 incomplete task(s) found. Continuing due to --yes flag.` —— 归档时验证任务尚未执行，属**预期时序**（验证只能在归档后做）；现已全部回填实测结果
