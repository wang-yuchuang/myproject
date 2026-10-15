# Spec: Publib — 公共基础库

**Subsystem**: publib
**Version**: 0.3
**Created**: 2026-07-01
**Last Updated**: 2026-07-02

---

## Purpose

公共基础库，提供跨模块复用的 OS 抽象层（线程 / 互斥锁 / 队列 / 内存池 / 日志 / SQLite 封装）。

**注意**：本库并非"无硬件依赖"——所有模块依赖 POSIX pthread，日志与数据库模块依赖 SQLite 文件 I/O，详见各 R 的"依赖"列。

源文件位于 `src/publib/`，对应头文件位于 `src/include/base_defines/`（如 `iQueue.h` / `iMutex.h` / `iThread.h` / `iNew.h` / `iLog.h` / `iLog4c.h` / `basefunc.h`）。

---
## Requirements
### Requirement: R1 线程安全队列

系统 MUST 提供线程安全的 FIFO 队列，支持动态扩容和阻塞/非阻塞出入队。

**依赖**：pthread（mutex + semaphore）

#### Scenario: 队列基本操作
- **WHEN** 创建队列后执行入队（`Queue_Put`）和出队（`Queue_Get`）操作
- **THEN** 入队元素按 FIFO 顺序正确出队
- **AND** 队列中的元素数量（`nCount`）正确
- **AND** 支持紧急插入（`bUrgent` 参数，插入到队头）

#### Scenario: 队列动态扩容
- **WHEN** 入队元素超过初始容量（`nCapacity`）且创建时指定了 `nIncStep > 0`
- **THEN** 队列自动扩容（`NEW` 新 buffer + `memmove` 拷贝），数据不丢失

#### Scenario: 队列多线程安全
- **WHEN** 多个线程同时操作同一个队列
- **THEN** `pthread_mutex_t hSyncLock` 保证数据不损坏
- **AND** `hCountSem` 信号量支持出队阻塞等待

#### Scenario: 队列索引访问
- **WHEN** 调用 `Queue_GetEx(hq, nWhich, pElement, bPeek)` 按索引读取元素
- **THEN** `nWhich=0` 返回队头元素，`nWhich=-1` 返回队尾元素
- **AND** 其他正整数表示从队头偏移指定位置
- **AND** 此操作不阻塞（无等待），元素不存在时返回 `ERR_QUEUE_EMPTY`

> **已废弃 / 由其他模块承担**：本 spec 早期版本描述的"INI 配置解析（`iCfg_Parser.c`）"是历史死代码——
> `iCfg_Parser.c` 不在 `src/publib/Makefile` 的 OBJS 中，全代码库零引用，运行时不生效。
>
> **真实的配置加载机制**由 [data-processor](../data-processor/spec.md) R1 承担：
> 通过 SQLite 数据库 `/app/config/System.cfg` 加载，实现于 `src/moduals/data_processor/dpr/config_mangmt.c`
> （`Cfg_Parse_Sampler` / `Cfg_Parse_Reporter` / `Cfg_Parse_CommPort` / `Cfg_Parse_Device` 等）。
>
> 本 spec 不再重复描述配置加载需求。

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

### Requirement: R3 互斥锁封装

系统 MUST 提供对 POSIX `pthread_mutex_t` 的封装接口（`Mutex_Create` / `Mutex_Lock` / `Mutex_Unlock` / `Mutex_Destroy`）。

**依赖**：pthread

#### Scenario: 锁操作
- **WHEN** 创建、加锁、解锁、销毁互斥锁
- **THEN** 操作行为与 POSIX pthread_mutex 一致（内部结构 `MUTEX` 持有 `pthread_mutex_t hInternalMutex`）

#### Scenario: 超时锁等待
- **WHEN** 锁已被占用，另一线程以超时参数调用 `Mutex_Lock`
- **THEN** 在超时时间内阻塞等待锁释放
- **AND** 超时后返回 `ERR_MUTEX_TIMEOUT`（无死锁）
- **AND** 支持 `WAIT_INFINITE`（`(unsigned long)-1`）参数，无限等待直到获得锁

#### Scenario: 长时间等待中保持心跳
- **WHEN** `Mutex_Lock` 因锁被占用而进入等待循环（`WAIT_INFINITE` 或超时 > MAX_WAIT_INTERVAL）
- **THEN** 每轮等待（MAX_WAIT_INTERVAL=5s）后调用 `RUN_THREAD_HEARTBEAT()` 保持线程存活
- **AND** 防止线程管理器误判为无响应

#### Scenario: 双实现路径
- **WHEN** 编译时定义了 `__USE_XOPEN2K` 宏
- **THEN** `Mutex_Lock` 使用 `pthread_mutex_timedlock` 实现，内核级阻塞
- **AND** 否则回退到 `pthread_mutex_trylock` + `usleep` 轮询，用户态忙等
- **AND** 两者行为一致，但 timedlock 路径对 CPU 更友好
- **NOTE** `__USE_XOPEN2K` 在当前项目中**未定义**（`iMutex.c:6` 有 `#ifndef __USE_XOPEN2K` 但 `#error` 被注释），因此 `Mutex_Lock` / `Sem_Wait` **默认走 trylock+usleep 轮询路径**（用户态忙等），timedlock 路径仅定义该宏时生效

#### Scenario: 创建预锁定锁
- **WHEN** `Mutex_Create(FALSE)` 创建后立即检查
- **THEN** 返回的锁处于**已锁定**状态（bOpenedLock=FALSE 表示参数为假，创建时立即加锁）
- **AND** `Mutex_Create(TRUE)` 创建后锁处于**未锁定**状态，可直接使用

### Requirement: R3b 信号量封装
系统 MUST 提供 POSIX 信号量封装接口，支持超时等待和批量释放。

**依赖**：POSIX semaphore（`semaphore.h`）

#### Scenario: 信号量基础操作
- **WHEN** 创建信号量后执行 Wait 和 Post
- **THEN** `Sem_Wait` 等待计数器减一（计数器为 0 时阻塞）
- **AND** `Sem_Post` 释放指定数量的信号量
- **AND** `Sem_Destroy` 销毁底层信号量

#### Scenario: 命名信号量
- **WHEN** 通过 `Sem_CreateEx("/_semSysHandWait")` 创建命名信号量
- **THEN** 信号量在系统命名空间中可见
- **AND** 不同进程可通过相同名称访问同一个信号量对象

> **注意**：`Sem_CreateEx(pszName)` 内部使用 `sem_open(pszName, O_CREAT, 0666, 0)`，初始值**硬编码为 0**。当命名信号量已存在时，`O_CREAT` **不会重置其值**——旧值被复用。这与 `Sem_Create(nInitValue)` 不同，后者通过 `sem_init` 总是使用传入的 `nInitValue` 初始化。跨进程场景下若前次实例遗留了非零计数，`Sem_CreateEx` 不会归零。

#### Scenario: System-V 信号量
- **WHEN** 调用 `Creat_SemID` + `Init_Sem` 创建 System-V 信号量
- **THEN** `sem_wait_op`/`sem_post_op` 以原子操作增减信号量值
- **AND** 用于进程间同步（与共享内存配合使用）

### Requirement: R4 线程封装

系统 MUST 提供线程创建和管理抽象接口，包含线程管理器、动态条目表、心跳监控和命名线程。

**依赖**：pthread

> **实现规模提示**：`iThread.c` 约 1020 行，远超简单的 `pthread_create` 封装——含线程管理器（`Thread Manager`）、心跳计数（`dwThreadHeartbeat`）、动态扩容的 `RUN_THREAD_ENTRY` 表、线程名（`szThreadName`）和魔法 ID（`MAKE_THREAD_ID`）。

#### Scenario: 线程生命周期
- **WHEN** 通过 `RunThread_Create` 创建线程执行任务，后等待线程退出
- **THEN** 线程正确执行并返回预期结果
- **AND** 线程管理器维护其心跳和状态

#### Scenario: 线程心跳监控
- **WHEN** 线程通过 `RUN_THREAD_HEARTBEAT()` 宏在循环中定期更新心跳
- **THEN** 线程管理器每 500ms 遍历所有线程表检查 `dwThreadHeartbeat`
- **AND** 连续 60s 无心跳更新 → 管理器通过 `pfnEventHandler` 回调发送 `THREAD_EVENT_NO_RESPONSE`

#### Scenario: 线程事件回调
- **WHEN** 线程事件产生（心跳、无响应、启动完成、停止等）
- **THEN** 注册的 `pfnEventHandler` 回调被调用，接收事件类型和线程句柄
- **AND** 回调返回值控制后续行为：`THREAD_CONTINUE_RUN`=继续, `THREAD_CANCEL_THIS`=取消, `THREAD_CANCEL_ALL`=停止全部

#### Scenario: 线程优雅停止与强制终止
- **WHEN** 系统退出时调用 `RunThread_ManagerExit(timeout, bKill)`
- **THEN** 管理器向每个线程发送停止信号（`nStatus=RUN_THREAD_TO_QUIT`）
- **AND** 等待超时后，若 `bKillTimeoutedThread=TRUE` 则强制终止（`pthread_cancel`）
- **AND** 线程退出后回调收到 `THREAD_EVENT_IS_STOPPED`

#### Scenario: 线程查找定位
- **WHEN** 通过 `RunThread_GetId("sampler_bms")` 按名称查找线程
- **THEN** 返回线程句柄（或 NULL 表示未找到）
- **AND** `RunThread_GetName` 从句柄获取线程名称
- **AND** `THREAD_IS_RUNNING(hSelf)` 宏判断线程是否存活

#### Scenario: 线程数量查询
- **WHEN** 调用 `RunThread_GetThreadCount()`
- **THEN** 返回当前正在运行的线程数量
- **AND** 不包括已停止或已回收的线程

### Requirement: R5 内存管理

系统 MUST 提供统一的内存分配和释放接口（`NEW` / `DELETE` 宏），内部含内存池（sector/block 分配）与 malloc 回退。

**依赖**：malloc/free（`<malloc.h>`）

> **实现规模提示**：`iNew.c` 实现了多 sector 的内存池（`POOL_BLOCK` / `s_MemPool`），仅在池耗尽时回退到 `malloc`。可追踪分配用于泄漏检测。

#### Scenario: 内存分配与释放
- **WHEN** 分配和释放内存
- **THEN** 优先从内存池分配，池满时回退到 `malloc`
- **AND** 释放时归还到对应 sector 或调用 `free`

#### Scenario: 内存池初始化与销毁
- **WHEN** 系统启动时调用 `MemoryPool_Init()`
- **THEN** 按 10 个扇区（16B~10240B）预分配内存块
- **AND** `s_MemPool.bInitOK` 设为 TRUE
- **AND** 系统退出时 `MemoryPool_Exit()` 释放所有扇区

#### Scenario: 内存重分配
- **WHEN** 调用 `Pool_Realloc(oldPtr, newSize)`
- **THEN** 若 `newSize <= GetBlockSize_ByMem(oldPtr)` 则直接返回原指针
- **AND** 否则在新 sector 分配新块，`memcpy` 数据，`Pool_Free` 旧块

#### Scenario: 调试模式内存追踪
- **WHEN** 定义了 `_DEBUG` 且 `Pool_Malloc` 因为所有扇区用满而回退到 malloc
- **THEN** `PrintMap` 输出当前内存池使用分布（每 sector 的 total/used/usage%）
- **AND** 记录回退分配的大小，便于定位内存泄漏
- **AND** `RENEW` 内部使用 `Pool_Realloc`，在调试模式下追踪重分配统计

### Requirement: R6 数据库操作

系统 MUST 提供基于 SQLite 的数据库封装接口，管理三种数据库的打开/关闭/锁定。

**依赖**：SQLite（`3rd_party/sqlite/`）、pthread（每个 DB 一个 `pthread_mutex_t s_sqLiteDBLock`）

> **注意**：`iLog.h` 中声明了两个**不同**的数据库初始化函数，在 `main.c` 中**顺序调用**（先 `InitCreatDataBase()` 再 `Init_Database()`）：
> - `InitCreatDataBase()`：打开/创建所有 `DB_TYPE_MAX` 个数据库文件（`sqlite3_open_v2` + `SQLITE_OPEN_CREATE`），设置 `sqlite3_busy_timeout` 为 20s，建立 `pDbHandle`。纯 I/O 操作，不做表结构或索引恢复。
> - `Init_Database()`：在已打开的 DB 上额外做**日志索引恢复**（`Get_LogIdx_FromVar` → 从 `.logIdx` 文件读取持久化的 `s_nCurrLogIdx`）和**表迁移/创建**（`Data_AppLog` 表 `CREATE TABLE IF NOT EXISTS`、查询 `max(Log_Idx)` 恢复断点续写索引、`Data_RealTimeBmsData` 表创建）。两者**职责不同，非兼容关系**。

#### Scenario: 数据库类型
- **WHEN** 调用 `InitCreatDataBase()` 初始化
- **THEN** 三种数据库被依次打开（AppLog / BmsRunData / CloudData）
- **AND** 文件不存在时自动创建（`SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE`）
- **AND** SQLite busy timeout 设置为 20 秒（旧注释代码曾用 5 秒，`iLog.c:996` / `iDataBase.c:856` 等处仍可见注释掉的 `sqlite3_busy_timeout(db, 5000)`）

#### Scenario: 线程/进程双层锁
- **WHEN** 多个线程/进程同时访问同一数据库
- **THEN** `Lock_DB_Private_FileLocker` 通过文件 `flock` 实现进程互斥
- **AND** `LOCK_SQLITE_DATABASE_SYNC` 通过 `pthread_mutex_trylock` 实现线程互斥
- **AND** 文件锁 + 互斥锁双重保护保证数据一致性

> **两个 flock 锁函数的区别**：
> - `iLog.c` 的 `Lock_SqlDB_FileLocker(nTimeOut_MSec)`：保护**日志索引**（`s_nCurrLogIdx` 持久化到 `/opt/appdata/data/.logIdx`），使用全局 `g_fd_locker_LogIndexFile`。
> - `iDataBase.c` 的 `Lock_DB_Private_FileLocker(pCurrDbInfo, nTimeOut_MSec)`：保护**每个数据库**的文件级互斥，使用 `pCurrDbInfo->nLockFd`（每 DB 独立 fd）。

#### Scenario: 同线程重入
- **WHEN** 同一线程连续两次调用 `Open_Lock_Database`
- **THEN** 递归检测识别（`pthread_equal` 比较 `hLastLockThread`）
- **AND** 第二次打开时不再加锁（`s_bInRecrusive = TRUE`），直接返回已有句柄

#### Scenario: 运行时模式切换
- **WHEN** 系统从初始化阶段切换到运行阶段时调用 `Database_Switch_RunMode`
- **THEN** 所有数据库句柄关闭（`sqlite3_close_v2`）
- **AND** 设置 `s_bIsSqlIn_RuntimMode = TRUE`
- **AND** 后续数据库操作每次独立加锁/解锁，无持久句柄

### Requirement: R7 system-hand 客户端 SDK（跨模块）

本库 MUST 提供 `iBasic_Funs.c` 中面向 system-hand 守护进程的客户端能力：

| 能力 | 函数 | 机制 |
|------|------|------|
| 系统命令执行 | `_SYSTEM` / `_SYSTEM_EX` | 通过 System V 共享内存 + 命名信号量将命令发给 syshand 执行 |
| 心跳上报 | `Do_Proc_HeartBeat` | 定期向 syshand 上报存活信号 |
| 动态库加载 | `LoadDynamicLibrary` / `UnloadDynamicLibrary` | `dlopen` / `dlsym` 封装，与 comm_loader 共用 |

**IPC 协议**（与 `src/utilities/system_hand/handler.c` 对偶，见 [system-hand spec](../system-hand/spec.md)）：
- 共享内存 key：`_KEY_T_SYSHAND = ftok("/app", 15)`
- 命令信号量：`/_semSysHandWait`，应答信号量：`/_semSysHandAnsw`
- 命令结构体：`CMD { iCmdType, szCmd, iCmdRet }`

> **实现细节**：`_SYSTEM` / `_SYSTEM_EX` / `Do_Proc_HeartBeat` 中的 syshand IPC 信号量操作使用**原始 POSIX API**（`sem_open` / `sem_trywait` 直接调用），而非本库的 `Sem_CreateEx` / `Sem_Wait` 封装。这是因为 IPC 信号量是跨进程共享的命名信号量，需要直接持有 `sem_t*` 指针（`psemWaitCmd` / `psemAnswer`）而非 `SEMAPHORE` 结构副本。序列化互斥仍使用 `Mutex_Lock(g_hMutexSystemHandler, ...)`。

#### Scenario: 系统命令执行
- **WHEN** 业务模块调用 `_SYSTEM("reboot")`
- **THEN** 命令通过 IPC 发送到 syshand 进程执行
- **AND** 调用方阻塞等待应答（超时 `_SYSTEM_LOCK_WAIT_TIMEOUT = 30000` ms）

#### Scenario: 心跳上报
- **WHEN** nccu_app 定期调用 `Do_Proc_HeartBeat`
- **THEN** syshand 侧确认进程存活，不触发恢复动作

> **归属说明**：`iBasic_Funs.c` 是混合性质文件（共 2738 行），除上述 system-hand 客户端代码外，还含 OS 工具（`Sleep` / `WaitFiledReadable`）、BCD/时间算法、字符串解析、仪表类型表等。后者属于本库的通用工具职责。

### Requirement: R8 栈宏（iStack.h）

系统 MUST 提供泛型栈的宏定义（头文件实现，无 .c 文件），适用于 PVOID 和 CHAR 两种类型。

**依赖**：无（纯宏展开）

#### Scenario: 栈 Push/Pop
- **WHEN** 调用 `STACK_INIT` 初始化栈后执行 `STACK_PUSH` 和 `STACK_POP`
- **THEN** 元素按 LIFO 顺序正确入栈/出栈
- **AND** `STACK_IS_FULL`/`STACK_IS_EMPTY` 正确检测边界
- **AND** `STACK_SAFELY_POP` 在栈空时返回 FALSE 而非越界

#### Scenario: 栈查看
- **WHEN** 查看栈顶元素而不移除
- **THEN** `STACK_TOP` 返回栈顶元素
- **AND** `STACK_DEPTH` 返回当前栈深度
- **AND** `STACK_POP_BOTTOM` 清空栈并返回栈底元素
- **AND** 无边界检查：若栈已空，`STACK_POP`/`STACK_TOP`/`STACK_POP_BOTTOM` 均可能读取未初始化数据，调用者需自行用 `STACK_IS_EMPTY` 防护

### Requirement: R9 字符串与工具函数

本库 MUST 通过 `basefunc.h` 提供以下跨模块通用的字符串处理、时间转换、文件操作和工具函数。

**依赖**：POSIX（`time.h`、`dlfcn.h`、`sys/select.h`）

| 类别 | 函数 | 说明 |
|------|------|------|
| 字符串安全复制 | `strncpyz` / `strncpyz_f` / `NEW_strdup` / `NEW_straddz` | 带长度限制的安全字符串操作，`_f` 变体支持非法字符替换回调 |
| 字符串处理 | `RemoveStringWhiteSpace` / `Trim_String` / `SplitString` / `GetSplitStringCount` | 空白移除、字符串裁剪、按分隔符拆分 |
| 时间转换 | `TimeToString` / `StringToTime` / `AcurTimeToString` | time_t 与格式化字符串互转 |
| BCD 时间戳 | `GetCurrentBcdTimeStmp` / `BcdTimeStmp2time_t` / `BcdTimeStmp2DoubleTime` / `ElapseMSBcdTimeStmp` | 紧凑 BCD 编码时间戳（年/月/日/时/分/秒/毫秒） |
| ISO 8601 | `Parse_ISO8601TimeStr` / `Parse_ISO8601TimeZone` / `Parse_ISO8601TimeStrEx` | ISO 8601 带时区时间字符串解析 |
| 动态库加载 | `LoadDynamicLibrary` / `UnloadDynamicLibrary` | `dlopen`/`dlsym`/`dlclose` 封装，按符号名批量解析 |
| 文件路径 | `MakeFullFileName` | 拼接根目录+子目录+文件名（最长 256 字节） |
| 文件等待 | `WaitFiledReadable` / `WaitFiledWritable` | `select()` 封装，带超时 |
| BCD 算术 | `Dec2Bcd` / `Bcd2Dec` | 整数与 BCD 码互转 |
| 睡眠 | `Sleep` / `Sleep_Ex` | 毫秒级线程睡眠 |
| 整数数组 | `FindIntItemIndex` | 在整数数组中查找指定值 |
|| 电表类型表 | `All_TypesMetersID[METER_TYPES_MAX][4]` | 4 类电表（柜内/网侧/光伏/辅助）的 ID 表 |
| 浮点时间 | `GetCurrentTime(void)` / `ElapseS(time_t)` / `ElapseMS(double)` | `GetCurrentTime` 返回 double 秒当前时间戳；`ElapseS` 接受 `time_t` 参数计算耗时秒数；`ElapseMS` 接受 `double` 参数计算耗时毫秒 |
|| 日期裁剪 | `ConvDateTime_2DateOnly` | 将 datetime 截断为日期（当日 00:00:00） |
|| 系统特征检测 | `HasEMSFeature(SYSTEM_TYPE)` | 判断系统是否具有 EMS（储能管理系统）功能 |

#### Scenario: 字符串安全复制
- **WHEN** 使用 `strncpyz(dst, src, dstLen)` 复制字符串
- **THEN** 最多复制 `dstLen-1` 字节，末尾强制添加 `\0`
- **AND** `NEW_strdup` `NEW_straddz` 通过内存池分配副本

#### Scenario: 时间格式化
- **WHEN** 调用 `TimeToString(now, TIME_CHN_FMT, buf, len)`
- **THEN** 返回格式如 `"2026-07-13 14:30:00"`
- **AND** `AcurTimeToString` 支持浮点数秒
- **AND** BCD 时间戳用于嵌入式设备紧凑存储
---

## Test Definition

> **现状说明**：以下"白盒测试"列描述的是**目标态**。当前代码库中：
> `src/publib/Makefile` 的 `TEST_EXEC =`（空），全项目零测试文件（所有 test 匹配均在 `3rd_party/`）。
> Unity 测试框架尚未搭建。以下表格记录的是各 R 达到白盒可测的难点和策略，待测试基础设施建立后实施。

| Requirement | 白盒测试 | 方式 |
|------------|:--------:|------|
|| R1 线程安全队列 | ✅ 可测 | 纯逻辑，mock pthread 或 x86 本地多线程 |
|| R2 日志系统 | ⚠️ 中等 | iLog.c 需 mock SQLite；iLog4c.c 需 mock log4c |
|| R3 互斥锁 | ⚠️ 中等 | 依赖 pthread，可在 x86 本地直接测试 |
|| R3b 信号量 | ⚠️ 中等 | 依赖 POSIX semaphore，x86 本地可测试 |
|| R4 线程封装 | ⚠️ 中等 | 依赖 pthread，可在 x86 本地直接测试 |
|| R5 内存管理 | ✅ 可测 | 纯逻辑（malloc 可 mock），适合内存泄漏检测 |
|| R6 数据库 | ⚠️ 中等 | 需 mock SQLite 或提供临时 DB 文件 |
|| R7 system-hand SDK | ❌ 难 | 依赖 System V IPC，需进程对偶测试或 mock IPC 层 |
|| R8 栈宏 | ✅ 可测 | 纯宏展开，无外部依赖 |
|| R9 工具函数 | ✅ 可测 | 纯逻辑，无硬件依赖 |

---

## Key Files

> 源文件在 `src/publib/`，头文件在 `src/include/base_defines/`。

| 文件 | 对应 R | 说明 |
|------|:------:|------|
| iQueue.c | R1 | 线程安全 FIFO 队列（pthread mutex + semaphore + 动态扩容） |
| iLog.c | R2 | **log4c 文件后端 + `LogOut4c` / `LogOut_BattDegr` / `LogCat`**（本文件内的 `LogOut` 副本在 `:616` **已注释**，不再是 DB 日志后端） |
| iLog4c.c | R2 | **log4c 文件日志后端**（log4c，bms/bmsalm/meterfault 分类） |
| iMutex.c | R3 | pthread_mutex 封装 |
| iThread.c | R4 | 线程管理器 + 心跳 + 动态条目表（~1020 行） |
| iNew.c | R5 | 内存池（sector/block）+ malloc 回退 |
| iDataBase.c | R6 | SQLite 线程安全封装（每 DB 一锁 + 可重入）+ **DB 日志后端（`LogOut` 家族；`Data_AppLog` / `Data_DumpLog`）** |
| iBasic_Funs.c | R7 | **system-hand 客户端 SDK**（`_SYSTEM`/心跳/IPC）+ 混合 OS 工具（`Sleep`/BCD/时间/字符串解析/仪表类型表） |
|| iMutex.c (Sem_*) | R3b | POSIX + System-V 信号量封装 |
|| iStack.h | R8 | 泛型栈宏（头文件实现，无 .c） |
|| basefunc.h | R9 | 字符串/时间/BCD/文件/动态库等工具函数 |
|| iCfg_Parser.c | (死代码) | INI 配置解析器，当前未编译无引用 |

> **死代码说明**：`iCfg_Parser.c` 不在 `src/publib/Makefile` 的 OBJS 中，全代码库零引用，运行时不生效。
> 清理前需确认是否被计划用于替代 SQLite 配置加载。

## Known Tech Debts

1. **`iCfg_Parser.c`（1012 行）** — 当前未编译、无引用，判定为死代码
2. **`iBasic_Funs.c`（2738 行）** — 混合 syshand 客户端 SDK + OS 工具函数 + 业务常量（电表 ID 表），未按职责拆分
3. **零测试代码** — 全模块无单元测试
4. **内存池预分配大小** — sector 数量和 block 数量取决于 `MemoryPool_Init` 实现，当前未在源码中找到显式配置常量
5. **`iStack.h`** — 宏实现无边界检查，数组越界风险由调用者负责
6. **`Sem_Wait` #else 路径未初始化变量** — 当 `__USE_XOPEN2K` 未定义时（即当前默认路径），`Sem_Wait` 的 `#else` 分支中 `hSelf`（`HANDLE` 类型）和 `tLast`（`double` 类型）声明后**未初始化**即使用：`RunThread_Heartbeat(hSelf)` 传入未初始化值，`tLast` 在 `TRACE` 和比较条件中读取。这是已知 bug，timedlock 路径（`#ifdef __USE_XOPEN2K`）无此问题。
