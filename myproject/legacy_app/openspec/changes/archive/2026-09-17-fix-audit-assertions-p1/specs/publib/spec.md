## MODIFIED Requirements

### Requirement: R2 日志系统

系统 MUST 提供线程安全的日志记录能力，包含两个独立后端实现：

| 后端 | 文件 | 存储介质 | 依赖 |
|------|------|---------|------|
| DB 日志 | `iDataBase.c` | SQLite 数据库文件 `$NCCU_DATA_PATH/<name>.data`（运行时为 `/data/{AppLog,BmsRunData,CloudData}.data`；表 `Data_AppLog` / `Data_DumpLog`）。**DB 文件在 `/data/`，其锁与索引在 `/opt/appdata/data/`**。⚠️ 实现**不在 `iLog.c`** —— 该文件内的 `LogOut` 副本在 `:616` 已注释 | SQLite |
| 文件日志 | `iLog4c.c` | 分类文件日志（bms / bmsalm / meterfault / bmshisalm 等 category） | log4c（`3rd_party/log4c/`） |

#### Scenario: 日志写入
- **WHEN** 写入不同级别的日志（INFO/WARING/ERROR/MILESTONE）
- **THEN** 日志正确写入对应后端的存储介质
- **AND** 日志包含模块名（如 `PUBL`/`MAIN`/`DPR`/`SAMP`/`REPT`）、子模块名和时间戳

#### Scenario: 多线程并发日志
- **WHEN** 多个线程同时写入日志
- **THEN** SQLite 后端依赖 `sqlite3_config(SQLITE_CONFIG_SERIALIZED)`（见 `main.c:242`）保证线程安全
- **AND** log4c 后端依赖库自身的线程安全保证

#### Scenario: 日志索引跨进程安全
- **WHEN** syshand 和 nccu_app 双进程同时写入应用日志
- **THEN** `Lock_SqlDB_FileLocker` 通过文件的 `flock` 互斥保护日志索引读写
- **AND** 日志索引（`s_nCurrLogIdx`）持久化到 `/opt/appdata/data/.logIdx`
- **AND** `Lock_SqlDB_FileLocker` 接受超时参数（通常 10 秒），超时后返回 FALSE 避免死锁
- **AND** 日志循环覆盖：达到 10000 条上限后从 1 开始覆盖

#### Scenario: 运行时模式切换
- **WHEN** 系统初始化完成后调用 `Set_SqliteDB_2RunTime_Mode`
- **THEN** SQLite DB 句柄关闭（`sqlite3_close`）
- **AND** 后续日志操作每次独立加锁/解锁，避免句柄共享

#### Scenario: 日志导出与删除
- **WHEN** 调用 `LogCat("/tmp/app.log", FALSE)`
- **THEN** SQLite 中的日志记录导出到指定文本文件
- **AND** `Log_Delete` 按类型清理过期日志（系统日志/转储日志/MBE 日志）

#### Scenario: 数据转储
- **WHEN** 调用 `DumpOut(DUMP_SRC_OCPP, DUMP_TYPE_WRITE, data)`
- **THEN** 通信原始数据写入 `Data_DumpLog` 表
- **AND** 支持 OCPP / ES_iOCPP 两个数据源，最大单条 65535 字节（`MAX_DUM_DATASIZE`，源码拼写缺 'P'，应为 `MAX_DUMP_DATASIZE`，定义于 `iLog.h:63`）

#### Scenario: log4c 多类别日志
- **WHEN** 使用 `LOG_BMS_CAN_INFO` / `LOG_BMS_ALM_CAN_INFO` / `LOG_METER_FAULT_INFO` 等宏
- **THEN** 日志写入 log4c 对应类别（bms / bmsalm / meterfault 等）
- **AND** 各类别可独立配置输出目标和格式（通过 `/opt/appdata/config/log4crc` 配置）
- **AND** 自定义时间格式 `dated_format` 以北京时间（UTC+8）输出，含微秒精度

#### Scenario: 分段日志输出
- **WHEN** 单条日志消息超过 950 字节
- **THEN** `log_message` 将其分段（`MAX_SINGLE_LOG_LEN=950`），每段独立调用 `log4c_category_log`
- **AND** 每段包含相同的前缀（`[file:line]`）

#### Scenario: MBE 和电池衰减日志
- **WHEN** 使用 `LogOut_MBE` 写入 MBE 日志
- **THEN** 数据存储在 MBE 专用日志表中，循环上限 500 条
- **AND** `LogOut_BattDegr` 记录电池完全充/放电事件的衰减日志

#### Scenario: 混合日志输出（LogOut4c）
- **WHEN** 调用 `LogOut4c(MAIN_MODULE_PUBLIB, "sub", LOG_TYPE_INFO, fmt, ...)`
- **THEN** 日志同时写入 SQLite DB（`Data_AppLog`）和 log4c 文件日志
- **AND** 这是两个日志后端共用的统一封装接口

#### Scenario: LogOut 函数族差异
- **WHEN** 选择不同的日志写入接口
- **THEN** `LogOut` 返回 `BOOL`，根据运行时模式选择性加锁，支持 `#ifdef TRACE_2DBLOG` 扩展 buffer（1024 vs 512）
- **AND** `LogOut_A` 返回 `void`，始终调用 `Open_Lock_Database`（Always-lock 变体），固定 512 字节 buffer
- **AND** `LogOut_Ex` 返回 `void`，在 `!s_bDbInitOK` 时自动调用 `InitCreatDataBase()` + `Init_Database()` 重初始化（Extension 变体，容错自愈）
- **AND** 三者均写入 `Data_AppLog` 表，日志循环上限由 `APP_LOG_SIZE`（定义于 `iLog.h:40`，值=10000）控制，`iLog.c` 与 `iDataBase.c` 共享此常量

