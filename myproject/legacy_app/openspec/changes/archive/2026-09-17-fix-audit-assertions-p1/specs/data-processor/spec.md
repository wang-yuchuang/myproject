## MODIFIED Requirements

### Requirement: R7 SQLite 数据库安全

系统 MUST 通过文件锁 + SQLite busy_timeout 双重机制保护运行时数据库（`nccu.data`）的并发访问安全。

**依赖**：publib R6（`Lock_SqlDB_FileLocker`）

#### Scenario: 活动锁定机制
- **WHEN** DPR 或其他模块需要访问运行时数据库
- **THEN** 使用 `Open_Lock_SqliteDB()` 获取已锁定的数据库句柄，操作完成后调用 `Close_UnLock_SqliteDB()` 释放
- **AND** 底层通过 publib 的 `Lock_SqlDB_FileLocker`（`flock` 文件锁）实现进程间互斥
- **AND** SQLite 内部通过 `sqlite3_busy_timeout(20000)`（20 秒）设置忙等待超时

#### Scenario: 数据库路径
- **WHEN** 访问运行时数据库
- **THEN** 路径由 `getenv(ENV_VARIABLE_DATA_PATH)` 获取根目录，拼接 `"nccu.data"` 文件名
- **AND** 默认路径为 **`/data/nccu.data`**（代码兜底 `iDataBase.c:691`）；运行时由 `NCCU_DATA_PATH` 决定（`run_app:10` 设为 `/data`）。⚠️ **DB 文件在 `/data/` 顶层，其锁与索引在 `/opt/appdata/data/`**

#### Scenario: 被注释的 Reopen_AndLockSqliteDB
- **WHEN** 查看 `dpr.h` 声明
- **THEN** `sqlite3* Reopen_AndLockSqliteDB(CHARGER* pCharger)` 和 `void UnLock_SqliteDB(void)` 有声明
- **AND** 但 `event_manager.c` 中实现已被**完整注释**（第 155-206 行），未编译
- **AND** 注释代码显示的预期流程：`sqlite3_open_v2` → 关闭旧句柄（最多等待 5 秒） → `sqlite3_busy_timeout(20000)` → `sync()`
- **AND** 当前实际使用的是 `Open_Lock_SqliteDB`/`Close_UnLock_SqliteDB`（来源和实现在 publib iDataBase.c）

#### Scenario: 写入事务保护
- **WHEN** 批量写入历史数据（`Refresh_HisParam`）或告警记录（`HandleEvent`）
- **THEN** 使用 `sqlite3_exec(db, "BEGIN TRANSACTION;", ...)` 和 `"COMMIT;"` 包裹批量操作
- **AND** 写入前 `Open_Lock_SqliteDB()`，写入后 `Close_UnLock_SqliteDB()`

