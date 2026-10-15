## 1. Delta 编写（**机械生成**，不许人工重抄 —— 见 design.md D2）

> 两个 delta 文件：
> - `specs/app-lifecycle/spec.md` ← `## MODIFIED Requirements` + R8
> - `specs/publib/spec.md` ← `## MODIFIED Requirements` + R2
>
> 生成方式：从 canonical **逐行提取**目标区间 → 做下列 **3 处精确替换** → 用 `diff` 自检只显示这 3 处。

- [ ] 1.1 `app-lifecycle` R8：提取 canonical `openspec/specs/app-lifecycle/spec.md` 的 **第 280-339 行**（60 行），仅替换下面 2 行
  - **1.1a 步骤 5**（canonical `:294`）
    - 原：``| 5 | `sqlite3_open_v2` | 打开/创建 `/opt/appdata/data/nccu.data` |``
    - 新：``| 5 | `sqlite3_open_v2` | 打开/创建 `$NCCU_DATA_PATH/nccu.data`（运行时 `/data/nccu.data`；`run_app` 设 `NCCU_DATA_PATH=/data`）|``
  - **1.1b 步骤 12**（canonical `:301`）
    - 原：``…当 `nMaxWait > 10` 时退出，共 11 × 100ms = ~1.1s 超时；main.c:314-326）|``
    - 新：``…当 `nMaxWait > 100` 时退出，共 101 × 100ms = ~10.1s 超时；main.c:314-326）|``
- [ ] 1.2 `publib` R2：提取 canonical `openspec/specs/publib/spec.md` 的 **第 58-126 行**（69 行），仅替换 1 行
  - **1.2a 后端表 DB 日志行**（canonical `:64`）
    - 原：``| DB 日志 | `iLog.c` | SQLite 数据库（表 `Data_AppLog` / `Data_DumpLog`，路径 `/opt/appdata/...`） | SQLite |``
    - 新：``| DB 日志 | `iLog.c` | SQLite 数据库文件 `$NCCU_DATA_PATH/<name>.data`（运行时为 `/data/{AppLog,BmsRunData,CloudData}.data`；表 `Data_AppLog` / `Data_DumpLog`）。**DB 文件在 `/data/`，其锁与索引在 `/opt/appdata/data/`** | SQLite |``
- [ ] 1.3 **自检**：对每个 delta 执行 `diff <提取块> <替换后文本>`，**必须只显示 3 处变更总数（app-lifecycle 2 处、publib 1 处）**，不得有任何其它行差异

## 2. 明确排除项

- [ ] 2.1 **不改代码**：代码行为正确（E1-E4），错的是文档
- [ ] 2.2 不把「挂载拓扑」写进 spec —— 属事实基线，已在 `openspec/.code-facts-snapshot.md` 的 `### 运行时可写路径与 DB/锁文件` 记录
- [ ] 2.3 不重构 R2/R8 的其它内容；不新增 Requirement；不重编号
- [ ] 2.4 不动 `app-lifecycle` Purpose 段的 2 处 `S91app_mount`（delta 机制无法覆盖，属既有 `<DEFERRED>`）
- [ ] 2.5 不顺带修正 `app-lifecycle` R8 步骤 13 「`WatchDog_Open`（当前禁用）」——该表述与 `keep-app-alive-on-bmu05` D6 一致且正确，无需改动

## 3. 验证

- [ ] 3.1 `openspec validate fix-runtime-path-and-readiness-docs` 通过
- [ ] 3.2 `openspec validate --all` 全绿
- [ ] 3.3 归档**前**记录基线（⚠️ **不能用 `nMaxWait > 10` 作判据** —— `nMaxWait > 100` 会匹配它，属子串陷阱；改用不会碰撞的片段）：`grep -c '共 11 × 100ms' openspec/specs/app-lifecycle/spec.md` → 预期 **1**；`grep -c 'opt/appdata/data/nccu.data' openspec/specs/app-lifecycle/spec.md` → 预期 **1**；`grep -c '/opt/appdata/...' openspec/specs/publib/spec.md` → 预期 **1**
- [ ] 3.4 执行 `openspec archive fix-runtime-path-and-readiness-docs -y`
- [ ] 3.5 归档**后**复验：`grep -c '共 11 × 100ms'` → **0**、`grep -c '共 101 × 100ms'` → **1**；`grep -c 'opt/appdata/data/nccu.data'` → **0**、`grep -c 'NCCU_DATA_PATH/nccu.data'` → **1**；`grep -c '/opt/appdata/...' openspec/specs/publib/spec.md` → **0**、`grep -c '\$NCCU_DATA_PATH/<name>.data' openspec/specs/publib/spec.md` → **1**
- [ ] 3.6 **转录完整性检查（最重要）**：`git diff openspec/specs/app-lifecycle/spec.md openspec/specs/publib/spec.md` 应**只显示 3 行行内变更**（R8 两行 + R2 一行），**不得出现任何其它行内改动**。允许 `openspec` 自身的空行规范化（判据见已归档变更 `fix-app-lifecycle-script-refs` 的 tasks.md 3.7）；若出现预期外的**行内**改动，必须 `git checkout --` 回退后重做
- [ ] 3.7 归档后确认 R8 仍为 **60 行**、R2 仍为 **69 行**（整块替换未增减行数）

## 4. 待办 / 关联

- [ ] 4.1 `<ORDER>` **必须在 `keep-app-alive-on-bmu05` 归档之前完成**（design.md D4）：否则 R7 描述新代码、R8 仍写旧就绪值，spec 内部再度自相矛盾
- [ ] 4.2 `<FOLLOW-UP>` 建立「代码改动必须同步 spec」的检查项 —— 本次第 3 处漂移即由提交 `c8906429` 改代码未改 spec 造成；建议纳入 `openspec/.code-facts-snapshot.md` 的 drift checklist 或提交流程
