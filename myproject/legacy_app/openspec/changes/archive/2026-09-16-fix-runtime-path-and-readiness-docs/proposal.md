## Why

canonical spec 里有 **3 处**与代码/真机不符，全部集中在「运行时路径」与「就绪等待值」：

| # | 位置 | 现文 | 应为 | 性质 |
|---|---|---|---|---|
| 1 | `app-lifecycle` R8 步骤 5 | `sqlite3_open_v2` 打开/创建 **`/opt/appdata/data/nccu.data`** | `$NCCU_DATA_PATH/nccu.data`（运行时 `/data/nccu.data`） | **错误** |
| 2 | `publib` R2 后端表 | DB 日志「路径 **`/opt/appdata/...`**」 | DB 文件在 `/data/`；仅其**锁与索引**在 `/opt/appdata/data/` | **不精确**（会误导读者去 `/opt/appdata` 下找 DB） |
| 3 | `app-lifecycle` R8 步骤 12 | `nMaxWait > 10` … 共 11 × 100ms = **~1.1s** | `nMaxWait > 100` … **~10.1s** | **漂移**（代码已改、spec 未同步） |

### 证据链（4 条独立，全部指向同一结论）

| # | 证据 | 结论 |
|---|---|---|
| E1 | `iDataBase.c` **全部 6 处** DB 路径构造均用 `getenv("NCCU_DATA_PATH")`（`:66` `:220` `:687` `:890` `:955` `:1024` `:1165`） | DB 路径 = `$NCCU_DATA_PATH/<name>.data` |
| E2 | `iDataBase.c:691` 环境变量缺失时**硬编码兜底 `/data`**（原作者注释：`if env not provided, should use default PATH`） | 默认根就是 `/data` |
| E3 | **所有平台变体**的启动脚本均设 `NCCU_DATA_PATH="/data"`：`startrun:15` / `mu3/startrun:15` / `run_app:10` / `mu3/run_app:10` / `run_app.debug:18` / `mu3/run_app.debug:18` / `start_ui:9` / `log_cat:7` / `start_web_server:34` / `server_main.conf:142` | **无一变体**使用 `/opt/appdata/data` |
| E4 | 真机 `stat`：`/data/nccu.data` 存在（inode `22`）；`/opt/appdata/data/nccu.data` → **`No such file or directory`** | 现象与代码一致 |

**反证搜索**：grep `src/` 全部 `/opt/appdata/data/` 用法（约 120 处 / 27 文件）→ 全是**锁**（`.applog.lock` 等）、**PID**、**`.logIdx`**、**升级临时件**（`upgradefile` / `app_nccu.tar.gz` / `firm_tmp_extract/`）、**`keepalived_*`**、**`ext_rtinfo`** → **无一处构造 DB 路径** ✓

### 挂载拓扑（易被误判之处）

```
/data          ← mount /dev/mmcblk0p45            (S00data_mount: USERDATA_POINT="/data")
/opt/appdata   ← mount --bind /data/rootfs/opt/appdata
```
二者**同分区但不同目录**（`/data/rootfs/` 的兄弟关系）。因此"反正都在 `/data` 分区上"**不能**推出"路径可互换" —— `/opt/appdata/data/nccu.data` 与实际文件 `/data/nccu.data` 是**两个不同目录下的两个不同文件名**。

### 第 3 条的来源（值得记录）

`nMaxWait > 10` → `> 100` 是**本仓库今天的提交 `c8906429`** 改的（把 `syshand.run` 就绪等待从 ~1.1s 延长到 ~10.1s），**改代码时未同步 spec**。这是"代码改动必须同步 spec"的缺口实例，也说明本变更应在 `keep-app-alive-on-bmu05` 归档**之前**完成，否则那份 spec 会把旧值固化。

## What Changes

- `app-lifecycle`：**MODIFIED R8** —— 仅改 2 行（步骤 5 的 DB 路径、步骤 12 的就绪等待值），其余 58 行逐字保留
- `publib`：**MODIFIED R2** —— 仅改 1 行（后端表的 DB 路径单元格），其余 68 行逐字保留
- **不改任何代码**：本变更是纯文档校正，代码已是正确行为
- **不新增 Requirement、不重编号**

## Capabilities

### New Capabilities

- (无)

### Modified Capabilities

- `app-lifecycle`：R8「nccu_app 初始化序列」修正 DB 路径表述与 `syshand.run` 就绪等待值
- `publib`：R2「日志系统」修正 DB 日志后端的存储路径表述，明确「DB 在 `/data/`、锁与索引在 `/opt/appdata/data/`」

## Impact

- **文档**：`openspec/specs/app-lifecycle/spec.md`、`openspec/specs/publib/spec.md`（由 `openspec archive` 依据 delta 生成）
- **代码**：无变更
- **依赖**：无变更
- **关联**：`openspec/.code-facts-snapshot.md` 的 `### 运行时可写路径与 DB/锁文件` 已先行修正（含同一套 E1-E4 证据）；`keep-app-alive-on-bmu05`（应在它归档前完成本变更）
- **风险**：低 —— 仅 3 行文本变更；MODIFIED 是整块替换，故 delta 用「逐行提取 + 精确替换」生成，并以归档后 `git diff` 仅含这 3 行变更作为验收判据
