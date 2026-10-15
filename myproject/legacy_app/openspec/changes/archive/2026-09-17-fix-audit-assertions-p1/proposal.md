## Why

2026-09-16/17 spec 全量审核的 **P1 批次**。三条断言与代码不符，且**均可经 delta 修正**（无需破例）。

### a. `data-processor` R7 — DB 路径错

| 位置 | 现文 | 代码实测 |
|---|---|---|
| `:16`（Purpose 段，**不可达 ⇒ 手工**） | 「运行时数据写入 `/opt/appdata/data/nccu.data`」 | `NCCU_DATA_PATH=/data`（`run_app:10`）；DB 在 **`/data/nccu.data`** |
| `:280`（R7 内，**可经 delta**） | 「默认路径为 `/opt/appdata/data/nccu.data`」 | `iDataBase.c:691` 兜底 **`/data`** |

> ⚠️ 本 spec 自身即自相矛盾：`:287` 正确写着「实现在 publib **iDataBase.c**」，而 `:16`/`:280` 把 DB 放在 `/opt/appdata/data/`。**锁与索引**在 `/opt/appdata/data/`，**DB 文件**在 `/data/` 顶层。

### b. `publib` R2 + Key Files — DB 日志后端**归属错文件**

| 位置 | 现文 | 代码实测 |
|---|---|---|
| `:62`（R2 内，**可经 delta**） | 「DB 日志 \| `iLog.c` \| …」 | `iLog.c:616` 的 `LogOut` 定义**是注释掉的**；实现在 **`iDataBase.c`**（10 处）|
| `:399` / `:404`（`## Key Files`，**不可达 ⇒ 手工**） | `iLog.c` = DB 日志后端；`iDataBase.c` 仅 R6 | `iLog.c` 实际持有 **log4c 后端 + `LogOut4c`(`:900`) / `LogOut_BattDegr`(`:1316`) / `LogCat`**；`LogOut` 家族在 `iDataBase.c` |

### c. `app-lifecycle` R9 — 停止链**实际不可达**

`src/shells/app_entry` 的流程是：

```
:58  echo $$ > /var/run/app.pid          ← 写 PID
:61  trap '…stoprun…' TERM               ← 设 trap
:64  "${APP_SHELL_PATH}/startrun"        ← 调 startrun（其把 syshand/nccu_app 后台化）
     （脚本到此结束 → shell 退出 → trap 随之消失）
```

⇒ **trap 永远不会触发** ✗；`/var/run/app.pid` 记录的是**已死的 shell PID** ✗。
⇒ 另核实变体差异：**bmu05 走 mu3**，而 `mu3/stoprun:7` 是 `killall -9 nccu_ui` ✗（spec `:357` 只写 `nccu_ui_mm`，那是非 mu3 的 `stoprun:13`）。

> 该缺陷**已在** `keep-app-alive-on-bmu05` 的 `design.md`「记录但不修复」中登记；本变更把 **spec 也拉齐**（spec 仍声称停止链路可用 ✗）。
> 另：`app_entry:14` 的注释「4. 注册 SIGTERM 信号处理器，优雅退出」是我们自己写的、同样误导 —— 本变更**不改代码**，仅在 spec 中记录事实。

## What Changes

- **`data-processor`**：MODIFIED **R7**（1 行：`:280`）+ 手工编辑 Purpose 段 `:16`（1 行）
- **`publib`**：MODIFIED **R2**（1 行：`:62`）+ 手工编辑 Key Files 表（2 行：`:399` / `:404`）
- **`app-lifecycle`**：MODIFIED **R9**（`:343` 1 行、`:347-353` 7→5 行、`:357` 1 行、`:364` 1 行，另加「已知缺陷」blockquote）
- 同时同步 `.code-facts-snapshot.md` 的对应事实
- **不改任何代码** —— 代码行为已是事实，错的是文档

## Capabilities

### New Capabilities

- (无)

### Modified Capabilities

- `data-processor`：R7「SQLite 数据库安全」的默认路径
- `publib`：R2「日志系统」的后端文件归属
- `app-lifecycle`：R9「优雅停止与信号处理」的停止链可达性与 mu3 变体事实

## Impact

- **文档**：`openspec/specs/{data-processor,publib,app-lifecycle}/spec.md`、`openspec/.code-facts-snapshot.md`
- **代码**：无变更
- **关联**：`keep-app-alive-on-bmu05`（其 design.md 已登记 R9 停止链缺陷，且其 R7 delta 与本次 R9 无重叠）；`fix-p1` 与 `keep-app-alive` 归档顺序**无关**
- **风险**：中 —— 含 3 处手工编辑（delta 不可达）与 R9 的**有意行数变化**；验收以"归档后手工编辑存活 + 各块行数可解释"为准
