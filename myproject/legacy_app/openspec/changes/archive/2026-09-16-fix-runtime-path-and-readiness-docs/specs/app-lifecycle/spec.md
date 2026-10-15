## MODIFIED Requirements

### Requirement: R8 nccu_app 初始化序列

nccu_app 主进程 MUST 按严格顺序完成初始化，任何步骤失败都将导致进程退出。

**实现**：`src/main/main.c`

**预初始化**（步骤 1-16，在子系统加载之前）：

| 步骤 | 函数 | 失败行为 |
|------|------|---------|
| 1 | `Main_ProcessCmdLine` | 解析命令行参数 |
| 2 | `MemoryPool_Init` | 初始化内存池 |
| 3 | `Lock_SqlDB_FileLocker` | 等待其他进程释放 DB 锁（最多 30 次重试，每次 1s）|
| 4 | `sqlite3_config(SERIALIZED)` | SQLite 多线程安全模式 |
| 5 | `sqlite3_open_v2` | 打开/创建 `$NCCU_DATA_PATH/nccu.data`（运行时 `/data/nccu.data`；`run_app` 设 `NCCU_DATA_PATH=/data`）|
| 6 | `InitCreatDataBase` | 数据库表结构初始化 → 失败 `return -1` |
| 7 | `Init_Database` | 日志表创建 → 失败 `return -1` |
| 8 | `Database_Switch_RunMode` | 切换到运行时模式 |
| 9 | `Init_CreadDataDB` | 数据 DB 初始化 → 失败 `return -1` |
| 10 | `test_log_init` | log4c 初始化 → 失败 `return -1` |
| 11 | `Create_Charger` | 创建 CHARGER 上下文 → 失败 `return -1` |
| 12 | 等待 `syshand.run` | 轮询 `/opt/appdata/data/syshand.run`（`nMaxWait` 从 0 起，每次 `Sleep_Ex(100)` 后自增，当 `nMaxWait > 100` 时退出，共 101 × 100ms = ~10.1s 超时；main.c:314-326）|
| 13 | `WatchDog_Open` | 硬件看门狗（当前禁用）|
| 14 | `Install_SigHandlers` | 注册信号处理器 |
| 15 | `setjmp` | 设置故障跳转点 |
| 16 | `RunThread_ManagerInit` | 线程管理器初始化 → 失败 `return -1` |

#### Scenario: 6 步子系统加载（Main_LoadModuals）
- **WHEN** 预初始化完成后调用 `Main_LoadModuals`
- **THEN** 按以下顺序加载：
  1. `Init_CreateDataProcessor` — DPR 数据中心初始化
  2. `LoadConfigAndData` — 从 SQLite `System.cfg` 加载配置和运行时数据
  3. `Init_LoadDAI` — 数据访问接口加载
  4. `Init_Load_AllCommPorts` — 9 个通信模块动态加载（`dlsym`）
- **AND** 在步骤 4 和 5 之间调用 `Set_SqliteDB_2RunTime_Mode` 切换 SQLite 到运行时模式
  5. `Init_Load_AllSamplers` — 采样器模块加载
  6. `Init_Load_AllReporters` — 上报器模块加载
- **AND** 任何步骤失败 → 记录错误日志、`return FALSE` → main 返回 -1

#### Scenario: 事件主循环
- **WHEN** `Main_LoadModuals` 成功后进入主循环（`while(!s_pChargeThis->bExit)`）
- **THEN** 每次循环调用 `Do_Proc_HeartBeat()` 向 syshand 发送心跳
- **AND** 从事件队列（`hqEvent`）取出事件，100ms 非阻塞超时
- **AND** 有事件时调用 `dataprocessor.DoEvent()` 处理
- **AND** 处理 `bResetCmdPending` 系统重置请求（最多等待 10s 后 `_SYSTEM("reboot")`）
- **AND** 线程管理器心跳计数器递减，归零时触发 PANIC 退出

#### Scenario: 故障退出（longjmp）
- **WHEN** 信号处理器触发 `longjmp(s_JumpBufFaultError, nExitCode)`
- **THEN** 跳回 main 中 `setjmp` 返回点
- **AND** 调用 `RunThread_ManagerExit(10000, TRUE)` 等待线程退出（10s 超时，强制 cancel）
- **AND** 调用 `_exit(nExitCode)`（不调用 `exit`，避免 atexit 钩子）

#### Scenario: 优雅退出与资源释放
- **WHEN** 主循环正常退出（bExit=TRUE）
- **THEN** 调用 `RunThread_ManagerExit(10000, TRUE)` 停止所有线程
- **AND** 调用 `Main_Exit` 清理资源
- **AND** 调用 `MemoryPool_Exit` 释放内存池
- **AND** 调用 `Exit_LogRepo` 关闭日志
- **AND** `sqlite3_close(db)` + `ExitDatabase()` + `sqlite3_shutdown()`
