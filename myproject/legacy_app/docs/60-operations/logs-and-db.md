# 运行日志与数据文件（Logs & DB Runbook）

> **性质**：操作手册
> **需求权威源**：`openspec/specs/app-lifecycle/spec.md`（R7/R8/R13）+ `openspec/specs/system-hand/spec.md`（R1/R5）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`（「运行时可写路径与 DB/锁文件」节）
> **对应提交**：`97cac555`
> **状态**：🟠 部分

## 日志目录：`/opt/appdata/log`

- `TRACE_LOG` 指向 `/opt/appdata/log/trace.log`（`run_app:5,47`）。排障时第一个看的文件。
- 应用 stdout/stderr（fd 1、2）与 `iLog4c` 追踪是否落盘是**变体相关**的：bmu05（mu3）`mu3/run_app:147` 有 `>>$TRACE_LOG 2>>$TRACE_LOG` 生效；非 mu3 变体 `run_app:192` 该重定向被注释，stdout/stderr **不**落 trace.log（`iLog4c` 仍落）。⚠️ spec R13 仍要求该重定向（spec-code drift，见 tech-debts.md）。
- 目录由 `run_app:50` 幂等 `mkdir -p` 创建；`/opt/appdata` 被清空后应用会自行重建骨架。
- ⚠️ `AppLog.data` 是 SQLite 库文件，不是文本日志——对它 `grep -c` 得到的是二进制页里的偶然匹配，不能当事件计数（snapshot `:114`）。

## 日志轮转与 `.logIdx`

- 日志序号索引 `.logIdx` 位于 `/opt/appdata/data/`（snapshot `:109`）；`loginfo.N` 系列文件位于 `/data/` 顶层（snapshot `:111`）。
- 轮转级别与行为由 `log4crc` 配置驱动（见下节）；轮转参数的权威出处是仓库内 `log4crc` 模板，不在本文展开。

## `log4crc`

- 出厂模板在 `/app/extensions/log4crc`，首次启动由 `run_app` 复制到 `/opt/appdata/config/log4crc`（app-lifecycle R7「run_app 环境准备」），运行期以该覆写为准。
- `/app` 受 dm-verity 只读保护，改日志行为要改 `/opt/appdata/config/log4crc`，不要动 `/app` 内副本。

## 数据文件：`/data/*.data` 与 `loginfo.N`

| 文件 | 位置 | 说明 |
|---|---|---|
| `nccu.data` | `/data/` 顶层 | 主数据 DB（`iDataBase.c:66` 起 6 处 `getenv("NCCU_DATA_PATH")`） |
| `AppLog.data` / `BmsRunData.data` / `CloudData.data` | `/data/` 顶层 | 业务/日志/运行数据 SQLite 库 |
| `loginfo.N` | `/data/` 顶层 | 日志信息序列文件 |
| `System.cfg` | `/app/config/`（只读） | 配置 DB。`/opt/appdata/config/` 有覆写副本但 **System.cfg 不从中读取**（仅 `log4crc` 生效，见 [40-configuration/system-cfg.md](../40-configuration/system-cfg.md)） |

- DB 落点由 `NCCU_DATA_PATH=/data` 决定（`run_app:10`；环境变量缺失时代码兜底 `/data`，`iDataBase.c:691`）。
- ⚠️ `/opt/appdata` 是 bind mount，源为 `/data/rootfs/opt/appdata`：同分区、不同目录。`/opt/appdata/data/nccu.data` **不存在**，按该路径找库必然扑空（snapshot `:100-114`）。

## 锁文件与 PID 文件：`/opt/appdata/data/`

| 文件 | 用途 | 备注 |
|---|---|---|
| `.{applog,bmsrundata,clouddata}.lock` | 三个 DB 的锁 | `iDataBase.c:34/39/44` |
| `.syscmd_locker` | 系统命令 flock 目标 | 残留无害，**勿删**：unlink 破坏跨进程互斥（`app_entry:52-54`） |
| `syshand.pid` / `nccu.pid` | 进程 PID | stoprun 的停止依据（system-hand R5：syshand.pid 是标记不是自锁） |
| `/var/run/app.pid` | app_entry PID | ⚠️ 死 PID（app-lifecycle R9），勿用于停止 |

- 清理可写区时保留 `.lock`、`.logIdx` 与 `.syscmd_locker`。

## 排查用命令（设备上只读核对）

```sh
tail -n 200 /opt/appdata/log/trace.log        # 启动/崩溃第一现场
ls -l /data/*.data                            # 四个 DB 应在 /data 顶层
ls -l /opt/appdata/data/                      # 只有锁、PID、.logIdx、syshand.run
grep -E ' /data | /opt/appdata ' /proc/mounts # 同设备两次出现 = bind mount 实锤
stat /opt/appdata/data/nccu.data              # 预期 No such file or directory
pidof nccu_app; pidof syshand                 # 进程存活
```

- `readlink -f` 检测不出 bind mount；判定要看 `/proc/mounts` / `findmnt` / `mountpoint -q`（snapshot `:104` 方法教训）。

## `log_view` 工具

- 日志查看器 `log_view`（`src/utilities/log_viewer/` 编译产物），由 `log_cat` 包装调用（`log_cat:18`）；用法与参数以脚本 `src/shells/log_cat` 为准。
