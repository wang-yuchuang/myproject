# legacy_app 文档中心

> **性质**：人读文档（叙述 / 图 / 操作手册）。
> **需求权威源**：[`openspec/specs/`](../openspec/specs/) —— 本文档中心**不复制 MUST/SHALL 级需求断言**，只做叙述、索引与操作说明。
> **代码事实基线**：[`openspec/.code-facts-snapshot.md`](../openspec/.code-facts-snapshot.md)
> **AI agent 入口**：[`AGENTS.md`](../AGENTS.md)
> **对应提交**：—（文档中心索引，版本无关）
> **状态**：🟢 主体完成（全部 10 个目录已建立内容）

---

## 权威源声明（防漂移）

本仓库存在多个文档面，各自权威边界如下。**任何冲突以此表为准**：

| 内容类型 | 唯一权威源 | 本文档中心的关系 |
|---|---|---|
| 需求 / 验收断言（R1..Rn + Scenario） | `openspec/specs/<capability>/spec.md` | **只链接、只叙述，不复制** |
| 代码事实（模块数 / 行号 / 路径 / 常量） | 代码本体；基线与漂移清单见 `openspec/.code-facts-snapshot.md` | 引用时**必须带 `对应提交`**，数字以代码为准 |
| 变更决策与历史 | `openspec/changes/`（含 `archive/`） | `90-reference/` 只做索引 |
| 面向 AI agent 的速查 | `AGENTS.md` | 不重复其内容 |
| 人读叙述 / 图 / 操作手册 | **本 `docs/`** | 本文档中心的独占领域 |

> ⚠️ **OpenSpec CLI 不校验 `docs/`**（它只认 `openspec/specs/`）。因此本目录的准确性依赖每篇文件头部的「权威契约头」与定期核对，而非工具。

---

## 文档地图

| 目录 | 内容 | 主要读者 | 状态 |
|---|---|:--:|:--:|
| [`00-overview/`](00-overview/README.md) | 系统定位、业务背景（储能后备管理）、术语表 | 新人 / 产品 | ✅ 3 页 + 2 图 |
| [`10-architecture/`](10-architecture/README.md) | 进程与线程模型、启动链、数据流、运行时目录、A/B 与 OTA 架构（图为主） | 新人 / 开发 | ✅ 6 页 + 12 图 |
| [`20-features/`](20-features/README.md) | **功能与规格摘要**：采集 / 处理 / 上报 / 通信 / 平台管理 | 开发 / 产品 | ✅ 5 页 + 6 图 |
| [`30-interfaces/`](30-interfaces/README.md) | 接口契约：HAL 6 函数、DAI、syshand IPC、SQLite schema、上报协议 | 开发 / 集成 | ✅ 6 页 + 2 图 |
| [`40-configuration/`](40-configuration/README.md) | `System.cfg` 参数模型、DeviceModel、i18n | 开发 / 实施 | ✅ 3 页 + 2 图 |
| [`50-build-deploy/`](50-build-deploy/README.md) | 构建、签名、dm-verity、RAUC 打包、工厂烧录 | 交付 / 构建 | ✅ 5 页 + 1 图 |
| [`60-operations/`](60-operations/README.md) | 启停、日志与 DB、升级与回滚、排障、掉电恢复 | 运维 / 现场 | ✅ 4 页 + 2 图 |
| [`70-platforms/`](70-platforms/README.md) | 平台变体矩阵：BMU05 / EMU01 / IMMU3 | 开发 / 交付 | ✅ 2 页 + 2 图 |
| [`80-verification/`](80-verification/README.md) | 验收清单、硬件验证步骤 | 测试 / 交付 | ✅ 2 页 + 1 图 |
| [`90-reference/`](90-reference/README.md) | 功能清单（对外）、已知问题 / 技术债 / 变更历史索引 | 产品 / 全员 | ✅ 5 页 |
| [`_meta/`](_meta/authority-contract.md) | 写作规范、模板、权威契约头（不进读者主路径） | 文档维护者 | ✅ 已建立 |

---

## 图表清单（手写 SVG）

> 共 30 张手写 SVG；规范见 [_meta/svg-style.md](_meta/svg-style.md)

系统总览（索引：[what-is-this.md](00-overview/what-is-this.md) / [scope-and-boundaries.md](00-overview/scope-and-boundaries.md)）
- [overview-context 系统上下文](00-overview/svg/overview-context.svg)
- [scope-boundary 职责边界](00-overview/svg/scope-boundary.svg)

启动流程（索引：[boot-chain.md](10-architecture/boot-chain.md)）
- [boot-01 应用启动全链路](10-architecture/svg/boot-01-power-on-chain.svg)
- [boot-02 谁负责启动](10-architecture/svg/boot-02-start-ownership.svg)
- [boot-03 nccu_app 初始化](10-architecture/svg/boot-03-nccu-init.svg)
- [boot-04 startrun / run_app 分支](10-architecture/svg/boot-04-startrun-branch.svg)

代码架构（索引：[process-model.md](10-architecture/process-model.md) / [data-flow.md](10-architecture/data-flow.md) / [runtime-layout.md](10-architecture/runtime-layout.md) / [ota-and-ab.md](10-architecture/ota-and-ab.md)）
- [arch-01 双进程模型](10-architecture/svg/arch-01-process-model.svg)
- [arch-02 数据流](10-architecture/svg/arch-02-data-flow.svg)
- [arch-03 运行时目录布局](10-architecture/svg/arch-03-runtime-layout.svg)
- [arch-04 A/B 槽位与 OTA](10-architecture/svg/arch-04-ota-ab.svg)

代码拓扑（索引：[module-topology.md](10-architecture/module-topology.md)）
- [topo-01 构建拓扑](10-architecture/svg/topo-01-build-topology.svg)
- [topo-02 链接依赖](10-architecture/svg/topo-02-module-links.svg)
- [topo-03 加载器与动态库](10-architecture/svg/topo-03-loader-dlsym.svg)
- [topo-04 DPR 内部](10-architecture/svg/topo-04-dpr-internals.svg)

功能地图与功能页（索引：[README.md](20-features/README.md)）
- [features-map 功能地图](20-features/svg/features-map.svg)
- [data-acquisition 数据采集](20-features/svg/data-acquisition.svg)
- [data-processing 数据处理](20-features/svg/data-processing.svg)
- [data-reporting 数据上报](20-features/svg/data-reporting.svg)
- [communications 通信](20-features/svg/communications.svg)
- [platform-management 平台管理](20-features/svg/platform-management.svg)

接口契约（索引：[dai.md](30-interfaces/dai.md) / [syshand-ipc.md](30-interfaces/syshand-ipc.md) / [sqlite-schema.md](30-interfaces/sqlite-schema.md)）
- [syshand-ipc 系统命令 IPC](30-interfaces/svg/syshand-ipc.svg)
- [sqlite-schema 两个 SQLite 库](30-interfaces/svg/sqlite-schema.svg)

配置模型（索引：[system-cfg.md](40-configuration/system-cfg.md) / [device-model-and-params.md](40-configuration/device-model-and-params.md)）
- [config-load-flow 配置加载流程](40-configuration/svg/config-load-flow.svg)
- [param-model 参数模型](40-configuration/svg/param-model.svg)

构建与交付（索引：[build.md](50-build-deploy/build.md) / [rauc-ota.md](50-build-deploy/rauc-ota.md) / [factory-flashing.md](50-build-deploy/factory-flashing.md)）
- [factory-flashing 出厂烧录 vs OTA](50-build-deploy/svg/factory-flashing.svg)

运维排障（索引：[lifecycle.md](60-operations/lifecycle.md) / [troubleshooting.md](60-operations/troubleshooting.md) / [recovery.md](60-operations/recovery.md)）
- [troubleshooting 故障排查树](60-operations/svg/troubleshooting.svg)
- [recovery 故障恢复决策树](60-operations/svg/recovery.svg)

平台变体（索引：[variant-matrix.md](70-platforms/variant-matrix.md) / [immu3-to-bmu05-migration.md](70-platforms/immu3-to-bmu05-migration.md)）
- [variant-matrix 机型差异矩阵](70-platforms/svg/variant-matrix.svg)
- [variant-mechanism 变体机制](70-platforms/svg/variant-mechanism.svg)

验证流程（索引：[acceptance-checklist.md](80-verification/acceptance-checklist.md)）
- [verification-flow 验证流程](80-verification/svg/verification-flow.svg)

---

## 四种阅读路径

### ① 新人上手 / 培训（约 2–3 小时）
```
00-overview/what-is-this.md      → 这系统是干什么的
00-overview/glossary.md          → 术语先对齐（BMU/PCS/CCU/DPR/sampler/reporter…）
10-architecture/process-model.md → 谁启动谁（双进程 + 启动链）
10-architecture/data-flow.md     → 数据从哪来到哪去
20-features/README.md            → 五大功能总览
AGENTS.md                        → 速查与代码探索指南
```

### ② 功能与规格（开发 / 产品）
```
20-features/*                    → 每篇叙述 + 链到 openspec/specs/<cap>/spec.md
30-interfaces/*                  → 契约细节（HAL / DAI / IPC / DB / 协议）
40-configuration/*               → 设备行为如何由 System.cfg 驱动
90-reference/feature-catalog.md  → 对外功能清单（弱化内部实现）
```

### ③ 运维与交付（现场 / 交付）
```
50-build-deploy/*                → 出包、烧录
60-operations/lifecycle.md       → 启停与升级
60-operations/troubleshooting.md → 排障
60-operations/recovery.md        → 掉电 / slot 恢复
80-verification/*                → 验收与硬件验证
```

### ④ 产品 / 客户
```
00-overview/what-is-this.md
90-reference/feature-catalog.md  → 功能清单（对外）
70-platforms/variant-matrix.md   → 机型差异（只写对外可见差异）
```

---

## 迁移状态（待办）

- [x] 根 [`README.md`](../README.md) 已改为**指向本文档中心**的简短指针（`docs/README.md` 现为文档入口）；陈旧内容已废弃（`S91app_mount` 已改名、syshand "丢失则重启系统" 已不成立）
- [x] 评估 `code-analysis/legacy_app.md`（2026-07-01 生成）：并入 `90-reference/` 或标记为历史归档（已删除；唯一有效内容「3rd_party 库表」抢救为 `30-interfaces/third-party-libs.md`）
- [ ] `AGENTS.md` 与本文档中心互链，避免内容重复（注：`AGENTS.md` 目前未引用 `docs/`，互链尚未建立）

---

## 维护约定

1. 每篇文件**必须**带 [`_meta/authority-contract.md`](_meta/authority-contract.md) 定义的头部。
2. `20-features/` 只写叙述与图，**不得**出现 "系统 MUST…" 这类需求断言 —— 那是 spec 的职责。
3. 凡引用数字（模块数、ID、行号、路径），必须回代码核对，并更新头部的「对应提交」。
4. 变更流程：改代码/ spec 时，在同一变更里同步受影响的 `docs/` 页面（见 `_meta/authority-contract.md`）。
