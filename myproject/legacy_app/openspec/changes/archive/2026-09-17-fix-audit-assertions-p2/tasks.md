## 1. Delta 编写（`data-processor` R6）

> canonical 块：`openspec/specs/data-processor/spec.md` 的 **R6** = `219..261`（**43 行**），delta 只做 **1 处 1→1 行**替换。

- [ ] 1.1 `:255` 的替换（1→1）
  - 原：`- **THEN** 支持的设置类型（\`INTERSET_TYPE\` 枚举）：\`SETTYPE_RAW_SP\`（原始状态参数）、…、\`SETTYPE_RAW_BACK_SETT\`（设置回读）、\`SETTYPE_CLEAR_FACTOY_DEBUG_DATA\`（清除出厂数据）、…`
  - 新：`- **THEN** 支持的设置类型（\`INTERSET_TYPE\` 枚举，共 13 种）：…、\`SETTYPE_RAW_BACK_SETT\`（设置回读）、\`SETTYPE_RESTORE_SETTINGS_FROMDB\`（从数据库恢复设置）、\`SETTYPE_CLEAR_FACTOY_DEBUG_DATA\`（清除出厂数据）、…`
- [ ] 1.2 自检：`diff <canonical R6 提取块> <delta R6 块>` 只显示上述 1 行差异；块行数 **43 → 43**

## 2. 直接编辑（非 capability spec）

- [ ] 2.1 `AGENTS.md:5` 去掉 `v0.3` → `（7 个子系统，版本见各 spec 头部）`
- [ ] 2.2 `AGENTS.md:124` 去掉 `v0.3` → `（spec 全量审核结果）`
- [ ] 2.3 `AGENTS.md:65` `/var/run/app.pid` 行补「（⚠️ 脚本随即退出，实为死 PID）」
- [ ] 2.4 `AGENTS.md:66` `trap SIGTERM → stoprun` 行补「（⚠️ 当前不可达）」

## 3. 验证

- [ ] 3.1 `openspec validate fix-audit-assertions-p2 --strict` → 绿
- [ ] 3.2 归档**前**基线：
  - `grep -c 'SETTYPE_RESTORE_SETTINGS_FROMDB' openspec/specs/data-processor/spec.md` → **0**（canonical 尚未含）
  - `grep -c 'v0\.3' AGENTS.md` → **2**（尚未编辑）
- [ ] 3.3 `openspec archive fix-audit-assertions-p2 -y` → `data-processor ~ 1 modified`
- [ ] 3.4 归档**后**：
  - `grep -c 'SETTYPE_RESTORE_SETTINGS_FROMDB' openspec/specs/data-processor/spec.md` → **1**
  - `grep -c '共 13 种' openspec/specs/data-processor/spec.md` → **1**
  - `grep -c 'v0\.3' AGENTS.md` → **0**
  - `grep -c '实为死 PID' AGENTS.md` → **1**；`grep -c '当前不可达' AGENTS.md` → **1**
- [ ] 3.5 **行数判据**：归档后 canonical R6 = **43 行**（不变）
- [ ] 3.6 `openspec validate --all` → 全绿（含 `keep-app-alive-on-bmu05`）

## 4. 收尾

- [ ] 4.1 提交：`docs(openspec): archive fix-audit-assertions-p2 (data-processor R6 INTERSET_TYPE 13, AGENTS.md version/trap)`
- [ ] 4.2 `<FOLLOW-UP>` `src/shells/app_entry:14` 注释与事实不符 —— 属代码，待下次触碰该脚本时修正
- [ ] 4.3 `<DEFERRED>` `samplers`/`comm-ports` 约 15 处 file:line 偏移 —— 保持登记（来源报告可信度不足）
- [ ] 4.4 `<DISPROVEN>` 「`comm-ports` 错误码 21 vs 20」—— 已证伪，不再追踪（证据见 design D5）
