## 1. 构建脚本实体化

- [x] 1.1 将 scripts/ 下 5 个符号链接替换为实体文件
- [x] 1.2 更新 scripts/UPSTREAM.md 记录上游来源和同步策略
- [x] 1.3 验证 d9_sdk 路径移除后构建通过

## 2. /var/volatile 路径迁移

- [x] 2.1 C/C++ 源文件全局替换 `/var/volatile/` → `/opt/appdata/data/`（25+ 文件）
- [x] 2.2 Shell 脚本全局替换 `/var/volatile` → `/opt/appdata/data`（run_app, startrun, stoprun, log_cat, mu3/*）
- [x] 2.3 验证编译通过，二进制中无残留 `/var/volatile` 路径

## 3. 动态库加载方式改造

- [x] 3.1 run_app 的 LD_LIBRARY_PATH 增加 `/app/extensions`
- [x] 3.2 移除 run_app 中所有 `cp -f /app/extensions/lib*.so /usr/lib/` 命令
- [x] 3.3 同步修复 startrun、mu3/run_app、mu3/startrun 的 LD_LIBRARY_PATH
- [x] 3.4 同步清理 mu3/run_app.debug、run_app.debug 中的 cp 命令

## 4. 验证

- [x] 4.1 独立构建通过（app.img + RAUC bundle）
- [x] 4.2 OTA 升级到设备成功
- [x] 4.3 syshand 自动启动
- [x] 4.4 PID 文件写入 /opt/appdata/data/
- [x] 4.5 无 /usr/lib cp 错误

## 5. 归档前修订（2026-09-16）：delta 重写

- [x] 5.1 删除原 delta `specs/legacy-app-runtime-paths/spec.md`（`## MODIFIED` 指向不存在的 capability —— 照原样归档会报错）
- [x] 5.2 新建 delta `specs/app-lifecycle/spec.md`：`## ADDED Requirements` + **R11 构建脚本自包含**（仅保留 1 条真正未被覆盖的条目）
- [x] 5.3 删除原 `Runtime paths on read-only rootfs` 条目 —— 结果已被 `system-hand`（PID 路径）与 `app-lifecycle` R7 `:260`（`LD_LIBRARY_PATH` 含 `/app/extensions`）覆盖；且其 Scenario 与自身 proposal 矛盾（`/etc` 是否可写）
- [x] 5.4 补 `R<n>` 前缀（R11，接在 R10 之后），宿主改为 `app-lifecycle`

## 6. 归档验证（**需在 `fix-legacy-app-runtime-issues` 之前归档**，见下）

- [ ] 6.1 `openspec validate decouple-from-sdk-and-ro-rootfs-adapt` 通过
- [ ] 6.2 归档**前**基线：`grep -c '^### Requirement' openspec/specs/app-lifecycle/spec.md` → 预期 **10**（R1-R10）
- [ ] 6.3 `openspec archive decouple-from-sdk-and-ro-rootfs-adapt -y` → 预期 `app-lifecycle: update`、`~ 1 modified`
- [ ] 6.4 归档**后**复验：同一 grep → **11**；`grep -n 'Requirement: R11'` 命中
- [ ] 6.5 **转录完整性检查**：`git diff openspec/specs/app-lifecycle/spec.md` 应**只显示 R11 块的纯追加**（在 R10 之后），且 R1-R10 无任何行内改动。允许 `openspec` 自身的空行规范化

> **归档顺序约束**：R11（本变更）与 R12/R13/R14（`fix-legacy-app-runtime-issues`）都追加到 `app-lifecycle`。**必须先归档本变更**，否则编号会乱序。

## 7. 待办

- [ ] 7.1 `<FOLLOW-UP>` 原 delta 中的 `Runtime paths on read-only rootfs`（`/var/volatile/` → `/opt/appdata/data/` 的迁移）作为**过程**未进 canonical，仅结果被记录 —— 若需要「运行时不得写 rootfs」这类**禁令性契约**，可在后续 change 中以新 Requirement 形式补入