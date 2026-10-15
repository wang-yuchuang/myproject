# 变更历史索引

> **性质**：索引
> **需求权威源**：`openspec/changes/`（变更决策与历史的唯一权威源，本文件只做索引）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：—
> **状态**：✅ 已核对（2026-09-17，与 `openspec/changes/` 目录及各 proposal.md 逐一比对）

> 📌 **每个变更的完整 proposal / design / tasks / spec delta 都在其目录内**，本表只给一句话摘要，无法替代原文。

## 阅读说明

- **排序**：最新在前，同日条目按归档先后倒排。
- **日期**：取自归档目录名前缀（`YYYY-MM-DD-`），即归档日期。
- **类型**：文档修正（只改 spec/docs）｜ 代码修复（改 C/Shell）｜ 构建（构建流水线）｜ spec 新增（补 Requirement）。
- **触及 spec**：该变更 delta 目录下的 capability；`legacy-app-runtime-fix` 等旧能力名已随变更归档停用，见文末说明。
- 想知道"某个行为为什么是这样"，直接进目录读 proposal 的 **Why** 小节，那里保留了当时的证据链。

## 索引表

| 日期 | 变更 | 类型 | 一句话目的 | 触及 spec | 目录 |
|---|---|---|---|---|---|
| 2026-09-17 | fix-audit-assertions-p2 | 文档修正 | 补 data-processor R6 漏列的第 13 个 SETTYPE 枚举值，并去掉 AGENTS.md 的单一版本号断言 | data-processor | `archive/2026-09-17-fix-audit-assertions-p2` |
| 2026-09-17 | fix-audit-assertions-p1 | 文档修正 | 修正三组与代码不符的断言：nccu.data 真实路径在 /data、publib DB 日志后端归属文件、app-lifecycle R9 的已知缺陷记录 | data-processor, publib, app-lifecycle | `archive/2026-09-17-fix-audit-assertions-p1` |
| 2026-09-17 | fix-reporter-ids-and-compile-tables | 文档修正 | 修正 reporters 的 11 处 Reporter_ID 错映射，重写 samplers 编译表（10 编译 / 7 未编译），并首次显式破例手工编辑 delta 不可达段落 | reporters, samplers | `archive/2026-09-17-fix-reporter-ids-and-compile-tables` |
| 2026-09-17 | fix-system-hand-behavior-claims | 文档修正 | 修正 R1/R5 与代码相反的描述：心跳丢失 180s 仅记录告警并继续服务，reboot 调用已从代码移除（非注释） | system-hand | `archive/2026-09-17-fix-system-hand-behavior-claims` |
| 2026-09-16 | fix-legacy-app-runtime-issues | 代码修复 | 修 nccu_app OTA 后启动即退：mu3 启动脚本同步修复、shmget 去掉 IPC_EXCL 冲突、RAUC compatible 改为环境变量可配（默认 bmu02_a00）、去掉 strace 调试残留 | app-lifecycle, legacy-app-runtime-fix | `archive/2026-09-16-fix-legacy-app-runtime-issues` |
| 2026-09-16 | decouple-from-sdk-and-ro-rootfs-adapt | 代码修复 | scripts/ 符号链接换实体文件以脱钩 d9_sdk；/var/volatile 全局替换为 /opt/appdata/data；移除向 /usr/lib 拷贝运行时库的命令 | app-lifecycle, legacy-app-runtime-paths | `archive/2026-09-16-decouple-from-sdk-and-ro-rootfs-adapt` |
| 2026-09-16 | migrate-legacy-app-build | 构建 | 旧 mkenv/make/do_pack 流程改造为 build.sh 统一入口，接入 dm-verity、在线签名、D9APPMD1 镜像与 RAUC bundle 流水线 | app-build-entry, app-image-builder, app-ota-bundle | `archive/2026-09-16-migrate-legacy-app-build` |
| 2026-09-16 | document-app-sysmgr | spec 新增 | app-lifecycle 新增 R10，只读记录 S91app_sysmgr 的 BSP 契约（其 v2 已不承担启动 app_entry 的职责） | app-lifecycle | `archive/2026-09-16-document-app-sysmgr` |
| 2026-09-16 | fix-runtime-path-and-readiness-docs | 文档修正 | 修正三处与真机不符的路径与数值：DB 文件在 /data/ 顶层、锁与索引才在 /opt/appdata/data、syshand 就绪等待约 10.1s | app-lifecycle, publib | `archive/2026-09-16-fix-runtime-path-and-readiness-docs` |
| 2026-09-16 | fix-app-lifecycle-script-refs | 文档修正 | 修正 R4/R6/R9 共 3 条 Requirement 中的脚本名 S91app_mount → S92app_mount（Purpose 段 2 处因 delta 不可达而 DEFERRED） | app-lifecycle | `archive/2026-09-16-fix-app-lifecycle-script-refs` |
| 2026-09-16 | sync-specs-with-code | spec 新增 | samplers 新增 R12/R13/R14，补齐 aircon / auxdev / thermal_control 三个已交付但 spec 零覆盖的采样器 | samplers | `archive/2026-09-16-sync-specs-with-code` |
| 2026-09-16 | keep-app-alive-on-bmu05 | ⏳ 进行中（24/29） | app_entry 收敛为 boot/OTA 单一入口、CCU 通信中断改为只告警不重启整机、pidof 精确存活判定；当前阻塞于 BSP R2（app_sysmgr 门控启动能力未随 rootfs 交付） | app-lifecycle, publib, samplers | `keep-app-alive-on-bmu05`（未归档） |

## 统计

- 归档 11 个，进行中 1 个。
- 按类型：文档修正 6 ｜ 代码修复 2 ｜ 构建 1 ｜ spec 新增 2 ｜ 进行中 1。
- 被触及最多的 capability：app-lifecycle（6 次），其次是 publib（3 次）。
- 活跃时段集中在 2026-09-16 至 09-17 两日：先补齐代码与构建（runtime-issues / decouple / migrate），再连续修正 spec 与代码的偏差。
- 进行中变更的任务清单共 29 项，已完成 24 项，剩余 5 项多数依赖板端验证。

## 归档机制

- 变更完成后经 `openspec archive` 移入 `archive/`，目录名自动加日期前缀。
- 归档只重写 canonical spec 的 Requirements 区块；Purpose / Key Files 等前言段落属 delta 不可达区，修正需显式破例（见 `fix-reporter-ids-and-compile-tables` 的留痕记录）。

## 遗留 capability 目录说明

- `legacy-app-runtime-fix`、`legacy-app-runtime-paths`、`app-build-entry`、`app-image-builder`、`app-ota-bundle` 是早期变更创建的能力目录，现**不在** `openspec/specs/` 下，随对应变更归档停用，仅在追溯旧 delta 时有意义。
- 当前有效的 7 个 canonical capability 见 [spec-index.md](spec-index.md)。

## 相关入口

- 各 spec 的需求明细 → `openspec/specs/<capability>/spec.md`
- 代码事实基线与漂移清单 → `openspec/.code-facts-snapshot.md`
- 进行中变更的任务清单 → `openspec/changes/keep-app-alive-on-bmu05/tasks.md`
- 变更流程与 openspec CLI 约定 → `openspec/config.yaml` 与 AGENTS.md（不在此重复）
- 文档中心总入口 → [docs/README.md](../README.md)
