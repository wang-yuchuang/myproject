# Spec: Data-Processor — 数据处理中心

**Subsystem**: data-processor
**Version**: 0.3
**Created**: 2026-07-01
**Last Updated**: 2026-07-14

---

## Purpose

数据处理中心（DPR），系统核心枢纽。接收 samplers 采集的数据，进行表达式运算、规则判断和事件处理，生成告警和历史记录，最后交 reporters 上报。

**架构位置**：samplers → **DPR** → reporters

**配置机制**：通过 SQLite 数据库 `/app/config/System.cfg` 加载（非 INI 文件），实现于 `config_mangmt.c`。运行时数据写入 **`$NCCU_DATA_PATH/nccu.data`（运行时 `/data/nccu.data`）** —— ⚠️ **DB 文件在 `/data/` 顶层，其锁与索引在 `/opt/appdata/data/`**（两者同分区但不同目录）。

**注意**：DPR 的线程/互斥锁/内存池/日志/数据库封装等基础设施由 [publib](../publib/spec.md) 提供，本 spec 仅描述 DPR 自身业务逻辑。

---
## Requirements
### Requirement: R1 配置管理

系统 MUST 通过 SQLite 数据库（`/app/config/System.cfg`）加载设备、采样器、上报器、通信端口、告警、历史参数等全部配置。

**依赖**：SQLite3（`3rd_party/sqlite/sqlite3.h`）

#### Scenario: 配置解析回调机制
- **WHEN** 系统启动调用 `InitCfgMagt(HANDLE hCharger)` 初始化配置管理器
- **THEN** `s_pCharger` 静态指针被设置，供后续 `GetParam_ByPR` 等函数使用
- **AND** 每类配置由对应的 `Cfg_Parse_*` 回调解析，回调原型为 `CFG_ROW_PARSER`：`(sqlite3_stmt*, void*, char**) → CFG_ERRCODE`
- **AND** 共 12 个解析函数：`Cfg_Parse_ChargerRevision`、`Cfg_Parse_Resources`、`Cfg_Parse_Sampler`、`Cfg_Parse_Reporter`、`Cfg_Parse_DevModel`、`Cfg_Parse_CommPort`、`Cfg_Parse_Device`、`Cfg_Parse_STUParam`（状态参数）、`Cfg_Parse_CTRParam`（控制参数）、`Cfg_Parse_SETParam`（设置参数）、`Cfg_Parse_Alarm`、`Cfg_Parse_HisParam`

#### Scenario: 配置错误处理
- **WHEN** 配置解析过程中发生错误
- **THEN** 返回 `CFG_ERRCODE` 枚举值：`CFG_ERR_OK=0`（成功）、`CFG_ERR_PARSER`（解析失败）、`CFG_ERR_NO_MEMORY`（内存不足）、`CFG_ERR_BADCONFIG`（配置格式错误）
- **AND** 错误配置行不影响其他有效行（逐行解析，单行错误仅记录日志）

#### Scenario: 参数查找（GetParam_ByPR）
- **WHEN** 调用 `GetParam_ByPR(PARAM_TYPE emType, int nDevId, int nParamId)` 查找参数
- **THEN** 按 `emType` 分发到不同参数表：`PARAM_TYPE_STATUS` → `pDevice->params_Status`、`PARAM_TYPE_CONTROL` → `pDevice->params_Control`、`PARAM_TYPE_SETTING` → `pDevice->params_Setting`
- **AND** 遍历 `pCharger->devices` 匹配 `nDevId`，再在对应参数表中匹配 `nParamId`
- **AND** 找到返回 `PARAMETER*`，未找到返回 `NULL`

#### Scenario: 告警与设备查找
- **WHEN** 调用 `GetAlarm_ByPR(int nDevId, int nAlarmId)` 查找告警
- **THEN** 遍历设备的 `alarms` 数组，匹配 `nAlarmId`
- **AND** 同样提供 `GetDevModel_ById(int nDevModelId)`、`GetDevce_ById(int nDeviceId)`、`GetSamplerById(int nSamplerId)` 查找函数

#### Scenario: 参数值校验（Check_ParamValue）
- **WHEN** 调用 `Check_ParamValue(PARAMETER* pParam, VAR_VALUE* pValue, int nSizeValue, BOOL bBackwardCompatible_4Blob)` 校验参数值
- **THEN** 首先检查大小：`nRealValSize != nSizeValue` 时返回 `ERR_PARAMVAL_SIZE(1)`，除非 `bBackwardCompatible_4Blob=TRUE` 且类型为 `VALUE_TYPE_BLOB`（向后兼容，返回 `ERR_PARAMVAL_OK_BUT_HAS_EXEPT(255)`）
- **AND** `pValue == NULL` 时返回 `ERR_PARAMVAL_RANGE(2)`（config_mangmt.c:171）
- **AND** 对 `VALUE_TYPE_LONG`/`VALUE_TYPE_ULONG`/`VALUE_TYPE_DATETIME`/`VALUE_TYPE_ENUM`/`VALUE_TYPE_FLOAT` 检查 `fMaxLimit`/`fMinLimit` 范围，越界返回 `ERR_PARAMVAL_RANGE(2)`
- **AND** 对 `VALUE_TYPE_BLOB` 检查 `blobType` 在 `BLOB_TYPE_UPGRADE_CCU` 到 `BLOB_TYPE_END` 范围内
- **AND** 校验通过返回 `ERR_PARAMVAL_OK(0)`

> **注意**：publib spec R1 中的"INI 配置解析"（`iCfg_Parser.c`）是历史死代码。真实的配置加载由本 R1 承担。

### Requirement: R2 事件管理

系统 MUST 提供事件注册、派发和历史恢复的集中管理机制，支持 samplers 和 reporters 注册自定义事件处理器。

**依赖**：publib R3（互斥锁 `Mutex_Create`/`Mutex_Lock`/`Mutex_Unlock`）

#### Scenario: 事件处理器注册
- **WHEN** 模块调用 `Register_EventHandler(char* pszFuncName, HANDLE hModule, EVENT_HANDLER pfnHandler, UINT nEventFlag, UINT nMsgFlag, BOOL bIsUnregister=FALSE)`
- **THEN** 处理器写入 `s_TableEvHandlers[s_nEventHandlerTotal]`，字段包括 `szName`（通过 `strncpy` 复制）、`hModuleSelf`、`pfnEvHandler`、`nEvTypeFlag`、`nMsgTypeFlag`
- **AND** `s_nEventHandlerTotal` 递增
- **AND** 操作受 `s_hEventHandMutex` 保护，锁超时 5000ms

#### Scenario: 注册错误码
- **WHEN** 注册时发现重复（同 module + 同 handler + 同 flag 组合）
- **THEN** 返回 `ERR_DPR_REPEATED_EVHAND`
- **WHEN** 注册数量达到 `MAX_EVENT_HANDLER=256`
- **THEN** 返回 `ERR_DPR_TOOMANY_EVHAND`
- **WHEN** `hModule == NULL`
- **THEN** 返回 `ERR_DPR_INVALID_ARG`
- **WHEN** 互斥锁创建或加锁失败
- **THEN** 返回 `ERR_DPR_MUTEX_ERR`

#### Scenario: 事件处理器去注册
- **WHEN** 调用 `Register_EventHandler(..., bIsUnregister=TRUE)` 且匹配到已注册处理器
- **THEN** 使用 `memmove` 将后续元素前移填补空位，`s_nEventHandlerTotal` 递减
- **AND** 后续事件不再派发给已注销的处理器
- **WHEN** 未找到匹配的处理器
- **THEN** 返回 `ERR_DPR_INVALID_ARG`

#### Scenario: 事件派发（HandleEvent）
- **WHEN** 调用 `HandleEvent(CHARGER* pCharger, HANDLE hSender, EVENT_ARG* pEventArg)`
- **THEN** 遍历 `s_TableEvHandlers`，匹配 `nEvTypeFlag`/`nMsgTypeFlag` 的处理器被回调
- **AND** 告警事件（`EVENT_TYPE_ALARM`）额外持久化到 SQLite `Data_Alarms` 表，使用循环索引 `s_nCurrAlarmIdx`（达到 `MAX_ALARM_RECORDS` 后回绕到 1）

#### Scenario: 事件索引管理
- **WHEN** 系统运行中产生各类事件
- **THEN** 通过 9 个独立循环索引计数器管理持久化记录编号：`s_nCurrEventIdx`（通用事件）、`s_nCurrAlarmIdx`（告警）、`s_nCurrChargeRecIdx`（充电记录）、`s_nCurrChargeMeterValIdx`（电表读数）、`s_nCurrESStatDataIdx`（ES状态数据）、`s_nCurrBmsRunDataValIdx`（BMS运行数据）、`s_nCurrHealthDataValIdx`（健康数据）、`s_nCurrMeterKwhValIdx`（电表kWh）、`s_nCurrAccumulatedDataIdx`（累计数据）

#### Scenario: 历史事件恢复（RestoreHisEvent）
- **WHEN** 系统启动时调用 `RestoreHisEvent(CHARGER* pCharger)`
- **THEN** 从 SQLite `Data_Events` 表查询未完成事件（`Event_Status = EVENT_STATUS_OCCUR`）
- **AND** 恢复到内存事件队列，确保重启后不丢失中断前的事件状态

### Requirement: R3 告警管理

系统 MUST 提供告警的产生（Begin）、解除（Cease）和抑制（Suppress）的完整生命周期管理，基于表达式运算驱动状态转换。

**依赖**：R4（表达式运算 `Exp_Calculate`）

#### Scenario: 告警状态定义
- **WHEN** 系统运行时
- **THEN** 每条告警 `ALARM` 有三种状态：`ALARM_STATUS_ACTIVE`（活跃）、`ALARM_STATUS_CEASED`（已解除）、`ALARM_STATUS_SUPPRESSED`（已抑制）

#### Scenario: 告警产生（Alarm_Begin）
- **WHEN** `Refresh_Alarm` 评估 `alarmCondExpression` 为 TRUE
- **THEN** 调用 `Alarm_Begin`，仅当 `pAlarm->emStatus != ALARM_STATUS_ACTIVE` 时触发
- **AND** 状态置为 `ALARM_STATUS_ACTIVE`，`tmLastOccured = time(NULL)`
- **AND** 构造 `EVENT_ALARM` 结构（字段 `nDevId`、`nAlarmId`、`status`、`tmStart`、`tmEnd=0`、`fExParamVal`），`emMsgType = EVENT_MSG_BEGIN`
- **AND** 调用 `HandleEvent` 派发事件

#### Scenario: 告警解除（Alarm_Cease）
- **WHEN** `Refresh_Alarm` 评估 `alarmCondExpression` 为 FALSE
- **THEN** 调用 `Alarm_Cease`，当 `pAlarm->emStatus != ALARM_STATUS_CEASED` 时触发
- **AND** **包括 `ALARM_STATUS_SUPPRESSED` 状态**的告警也会被解除（2021-09-09 修复，`data_processing.c:33`）
- **AND** 状态置为 `ALARM_STATUS_CEASED`，`tmLastCeased = time(NULL)`，`emMsgType = EVENT_MSG_END`

#### Scenario: 告警抑制（Alarm_Suppress）
- **WHEN** `Refresh_Alarm` 评估 `alarmCondExpression` 为 TRUE 且 `alarmMaskExpression` 也为 TRUE
- **THEN** 调用 `Alarm_Suppress`，仅当 `pAlarm->emStatus == ALARM_STATUS_ACTIVE` 时触发
- **AND** 状态置为 `ALARM_STATUS_SUPPRESSED`，`tmLastCeased = time(NULL)`（抑制也视为已解除时间）
- **AND** `emMsgType = EVENT_MSG_STATUS_CHANGED`

#### Scenario: 告警表达式评估流程
- **WHEN** `Refresh_Alarm(CHARGER* pCharger, DEVICE* pDev, ALARM* pAlarm)` 被周期调用
- **THEN** 若 `alarmCondExpression.nTotal <= 0` 则直接返回（无条件表达式不评估）
- **AND** 评估 `alarmCondExpression`：结果为 FALSE → 调用 `Alarm_Cease`；结果为 TRUE → 检查是否有 `alarmMaskExpression`
- **AND** `alarmMaskExpression` 为 TRUE → 调用 `Alarm_Suppress` 并 return；否则调用 `Alarm_Begin`

#### Scenario: EVENT_ALARM 结构体字段
- **WHEN** 产生告警事件时
- **THEN** `EVENT_ALARM` 包含：`nDevId`（设备ID）、`nAlarmId`（告警ID）、`status`（当前状态）、`tmStart`（告警开始时间）、`tmEnd`（结束时间，Begin 时为 0）、`fExParamVal`（附加参数值，取自 `pAlarm->pExParam->currValue.fValue`，为 NULL 时为 0.0）

### Requirement: R4 表达式运算

系统 MUST 支持运行时表达式运算引擎，用于告警条件判断、参数计算和显示/可控/可设条件判断。

**依赖**：`run_expression.h` / `run_expression.c`

#### Scenario: 运算符与元素类型
- **WHEN** 表达式引擎解析或计算表达式
- **THEN** 支持 12 种运算符（`TypesUsedByExp` 枚举）：`EXP_NOT(0)`、`EXP_MUL(1)`、`EXP_DIV(2)`、`EXP_ADD(3)`、`EXP_SUB(4)`、`EXP_GREATER(5)`、`EXP_LESS(6)`、`EXP_GREATER_EQUAL(7)`、`EXP_LESS_EQUAL(8)`、`EXP_EQUAL(9)`、`EXP_AND(10)`、`EXP_OR(11)`，以 `EXP_MAX_OPERATOR(12)` 标记运算符结束
- **AND** 解析阶段使用 `EXP_LF_BRACKET`（`(`）和 `EXP_RT_BRACKET`（`)`）
- **AND** 元素类型包括：`EXP_ETYPE_VARIABLE`（变量）、`EXP_ETYPE_CONST`（常量）、`EXP_ETYPE_OPERATOR`（运算符）、`EXP_ETYPE_DUMMY`（占位）、运行时 `EXP_ETYPE_INVALID`（值无效）、`EXP_ETYPE_ERROR`（表达式错误）
- **AND** `EXP_ETYPE_VARIABLE_RAW` 虽未在 `TypesUsedByExp` 枚举中定义，但在 `EXP_IS_VAR` 宏中引用（`run_expression.h:63`），用于区分原始变量类型

#### Scenario: 表达式计算（Debug/Release 双版本）
- **WHEN** 调用 `Exp_Calculate(nDevId, pSigName, pExp, nElements, pResult)` 宏
- **THEN** Debug 模式展开为 `Exp_CalculateD(nDevId, pSigName, pExp, nElements, pResult, __FUNCTION__, __LINE__)`，额外传递调用位置用于诊断
- **AND** Release 模式展开为 `Exp_CalculateR(nDevId, pExp, nElements, pResult)`，省略信号名和调用位置
- **AND** 返回 `EXP_ETYPE_CONST` 表示成功（结果为常量），返回 `EXP_ETYPE_INVALID` 表示值无效，返回 `EXP_ETYPE_ERROR` 表示表达式错误

#### Scenario: 常量提取宏
- **WHEN** 从 `EXP_ELEMENT` 中提取常量值
- **THEN** `EXP_GET_CONST_VALUE(pExpEle)` 返回 `EXP_CONST` 类型值
- **AND** `EXP_GET_CONST_INT(pExpEle)` 强转为 `int`、`EXP_GET_CONST_LONG(pExpEle)` 强转为 `long`、`EXP_GET_CONST_FLOAT(pExpEle)` 强转为 `float`
- **AND** `EXP_SET_CONST_VALUE(pExpEle, v)` 设置常量值

#### Scenario: 变量判断宏
- **WHEN** 判断元素是否为变量类型
- **THEN** `EXP_IS_VAR(pVar)` 宏检查 `byElementType == EXP_ETYPE_VARIABLE || byElementType == EXP_ETYPE_VARIABLE_RAW`
- **AND** 用于区分常量和变量，决定是否需要从设备参数中获取运行时值

#### Scenario: 表达式解析（ExpressionParser）
- **WHEN** 调用 `ExpressionParser(char* szBuf, CLUSTER_EXP_ELEMENT* pExpSet)` 将字符串表达式解析为表达式树
- **THEN** 解析成功后 `pExpSet` 中填充 `CLUSTER_EXP_ELEMENT` 数组，后续 `Exp_Calculate` 可直接使用
- **AND** 首次解析后无需重复解析（调用方设置 `bParseExpSet=FALSE`）

### Requirement: R5 数据处理主循环

系统 MUST 通过独立线程周期性刷新所有设备的状态参数、控制参数、设置参数、告警和历史数据。

**依赖**：R3（告警）、R4（表达式）、publib R4（线程 `Thread_Create`）

#### Scenario: DATA_PROCESSOR 结构体接口
- **WHEN** 主进程通过 `DATA_PROCESSOR` 结构体访问 DPR 功能
- **THEN** 结构体包含：`info`（`DATAPROC_INFO`，含 `emModuleType`、`nDataProcID`、`szDataProcName[32]`）、`hThread`（线程句柄）、`bExit`（退出标志）
- **AND** 函数指针：`LoadConfigAndData`（加载配置）、`DoDataProcessing`（启动处理线程）、`DoEvent`（处理单个事件）、`Get`（Intern_Get）、`Set`（Intern_Set）、`CalculateExp`（表达式计算）、`Register_EvHandler`（注册事件处理器）

#### Scenario: 主处理循环（_ThreadEntry_DataProcessing）
- **WHEN** `DoDataProcessing` 启动 `_ThreadEntry_DataProcessing` 线程
- **THEN** 循环间隔 `Sleep(200)`（200ms），每轮调用 `RUN_THREAD_HEARTBEAT()` 保持线程存活
- **AND** 遍历所有设备，依次调用 `Refresh_StatusParam`、`Refresh_CtrlParam`、`Refresh_SettParam`
- **AND** 遍历所有告警，调用 `Refresh_Alarm`
- **AND** 遍历所有历史参数，调用 `Refresh_HisParam`
- **WHEN** `bExit` 为 TRUE
- **THEN** 线程退出循环

#### Scenario: 状态参数刷新（Refresh_StatusParam）
- **WHEN** 刷新数据源为 `PARAM_SRC_CA`（计算型）的状态参数
- **THEN** 调用 `Exp_Calculate` 计算 `equalExpression`
- **AND** 返回 `EXP_ETYPE_CONST` 时更新值：`lastValue` ← `currValue`、`tmLastValueChanged` ← `tmCurrValueRefreshed`、`tmCurrValueRefreshed = _NOW_`
- **AND** 按 `emValueType` 分发赋值：`VALUE_TYPE_FLOAT` → `currValue.fValue`、`VALUE_TYPE_ULONG`/`VALUE_TYPE_ENUM` → `currValue.ulValue`、`VALUE_TYPE_LONG` → `currValue.lValue`
- **AND** 返回非 CONST 时置 `bIsValueInvalid = TRUE`
- **AND** 额外评估 `displayExpression` → 设置 `bDisplayAble`

#### Scenario: 控制/设置参数刷新
- **WHEN** 调用 `Refresh_CtrlParam(DEVICE* pDev, PARAMETER* pParam)`
- **THEN** 评估 `displayExpression` → `bDisplayAble`，评估 `controlableExpression` → `bCtrlSetAble`
- **AND** 无表达式时默认 `bCtrlSetAble = TRUE`
- **WHEN** 调用 `Refresh_SettParam(DEVICE* pDev, PARAMETER* pParam)`
- **THEN** 评估 `displayExpression` → `bDisplayAble`，评估 `settableExpression` → `bCtrlSetAble`

#### Scenario: 历史参数记录（Refresh_HisParam）
- **WHEN** 历史参数值变化超过阈值 `MAX(1.0, pParam->fTriggle_GreaterThan)` 或缓存满 `MAX_TEMP_STORED_HISVALS`
- **THEN** 将数据写入 SQLite `Data_HisStatusData` 表（`BEGIN TRANSACTION`...`COMMIT` 批量插入）
- **AND** 每条记录使用循环索引 `s_nCurrHisDataIdx`，达到 `MAX_HISDATA_RECORDS` 后回绕到 1
- **AND** 每 60 秒强制写一次（即使值未超阈值）
- **AND** `pParam->bIsValueInvalid` 为 TRUE 时跳过历史记录

### Requirement: R6 跨模块数据访问

系统 MUST 通过 `Intern_Get`/`Intern_Set` 函数指针为 samplers 和 reporters 提供统一的跨模块数据访问接口。

**依赖**：R1（配置管理 `GetParam_ByPR`）、R7（SQLite 安全 `Open_Lock_SqliteDB`）

#### Scenario: Intern_Get 接口
- **WHEN** 模块调用 `Intern_Get(HANDLE hSender, INTERGET_TYPE emGetTp, int nDevId, int nParamId, VAR_VALUE* pBuff, int nBuffSize)`
- **THEN** 返回 `ERRCDDE_DPR` 枚举值（14 种错误码：`ERR_DPR_OK`、`ERR_DPR_INVALID_ARG`、`ERR_DPR_NO_MEMORY`、`ERR_DPR_WRONG_SIZE`、`ERR_DPR_RAW_INVALID`、`ERR_DPR_TIMEOUT`、`ERR_DPR_SAMPLER_COMMFAIL`、`ERR_DPR_MUTEX_ERR`、`ERR_DPR_REPEATED_EVHAND`、`ERR_DPR_TOOMANY_EVHAND`、`ERR_DPR_QUEUE_FULL`、`ERR_DPR_DB_ERR`、`ERR_DPR_NO_DATA`、`ERR_DPR_NOTSUPPORTED`）
- **WHEN** `pModual == NULL || pBuff == NULL`
- **THEN** 返回 `ERR_DPR_INVALID_ARG`

#### Scenario: 参数类型查询分发
- **WHEN** `emGetTp = GETTYPE_SP_RAW_VAL` 或 `GETTYPE_SP_VAR`
- **THEN** 调用 `GetParam_ByPR(PARAM_TYPE_STATUS, nDevId, nParamId)` 获取状态参数
- **WHEN** `emGetTp = GETTYPE_CL_VAR`
- **THEN** 调用 `GetParam_ByPR(PARAM_TYPE_CONTROL, nDevId, nParamId)` 获取控制参数
- **WHEN** `emGetTp = GETTYPE_ST_VAR`
- **THEN** 调用 `GetParam_ByPR(PARAM_TYPE_SETTING, nDevId, nParamId)` 获取设置参数
- **WHEN** `emGetTp = GETTYPE_READBACK_VAR`
- **THEN** 返回设置参数的 `ReadValue`（回读值）
- **AND** 所有路径：`nValSize > nBuffSize` 时返回 `ERR_DPR_WRONG_SIZE`

#### Scenario: 特殊查询类型
- **WHEN** `emGetTp = GETTYPE_APPVER`
- **THEN** 返回 `s_pCharger->info.szSWVer`，`nBuffSize < MAX_LEN_MU2SWVER` 时返回 `ERR_DPR_WRONG_SIZE`
- **WHEN** `emGetTp = GETTYPE_COMMPORT`
- **THEN** 遍历 `s_pCharger->commports` 匹配 `nCommPort == nDevId`，`nBuffSize != sizeof(COMMPORT)` 时返回 `ERR_DPR_WRONG_SIZE`
- **WHEN** `emGetTp = GETTYPE_ALMSTATE`
- **THEN** 调用 `GetAlarm_ByPR(nDevId, nParamId)` 返回告警 `emStatus`
- **WHEN** `emGetTp = GETTYPE_METERVALUE_REC`
- **THEN** 查询 SQLite `Data_ChargeMeterValues` 表（`WHERE Gun_Idx = nDevId AND Charge_Begin_Time = nParamId`），使用 `Open_Lock_SqliteDB`/`Close_UnLock_SqliteDB` 保护，无数据返回 `ERR_DPR_NO_DATA`
> **注意**：`GETTYPE_CHARGE_REC` 已在 `INTERGET_TYPE` 枚举中定义，但 `Intern_Get` 未实现对应分发逻辑（10 种类型中仅 9 种有 handler）。

#### Scenario: Intern_Set 接口
- **WHEN** 模块调用 `Intern_Set(HANDLE hSender, INTERSET_TYPE emSetTp, void* pBuff, int nBuffSize)`
- **THEN** 支持的设置类型（`INTERSET_TYPE` 枚举，共 13 种）：`SETTYPE_RAW_SP`（原始状态参数）、`SETTYPE_LOGIC_SP`（逻辑状态参数）、`SETTYPE_INTERN_SETT`（内部设置参数）、`SETTYPE_RAW_ST`（设置参数刷新不同步）、`SETTYPE_INTERN_CTRL`（内部控制参数）、`SETTYPE_SENDMSG`（发送消息）、`SETTYPE_RAW_BACK_CTRL`（控制回读）、`SETTYPE_RAW_BACK_SETT`（设置回读）、`SETTYPE_RESTORE_SETTINGS_FROMDB`（从数据库恢复设置）、`SETTYPE_CLEAR_FACTOY_DEBUG_DATA`（清除出厂数据）、`SETTYPE_OVERWRITE_SET_BEFORE_CLONE`（克隆前覆盖）、`SETTYPE_READ_VALUE`、`SETTYPE_Reread_PCS_Param`
- **AND** 通过 `INTER_QUEUE_CMD` 结构体排队处理，包含 `emType` 和 `INTERSET_DATA`（`nDevId`、`nParamId`、`varValue`、`bRawInvalid`、`nValueSize`、`nTimeOut`）

#### Scenario: 模块名称解析（GetModuleName）
- **WHEN** 日志或诊断需要打印模块名称
- **THEN** `GetModuleName` 按 `MODULE_INFO.emModuleType` 分发：`MODULE_TYPE_SAMPLER` → `SAMPLER_INFO.szSamplerName`、`MODULE_TYPE_REPORTER` → `REPORTER_INFO.szReporterName`、默认 → `DATAPROC_INFO.szDataProcName`
- **AND** `MODULE_INFO` 结构体的 `emModuleType` 字段 MUST 位于第一个位置（用于多态指针转换）

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

## Test Definition

> **现状说明**：以下"白盒测试"列描述的是**目标态**。当前代码库中所有 Makefile 的 `TEST_EXEC =`（空），全项目零单元测试。
> Unity 测试框架尚未搭建。以下表格记录的是各 R 达到白盒可测的难点和策略。

| Requirement | 白盒测试 | 方式 |
|------------|:--------:|------|
| R1 配置管理 | ⚠️ 中等 | 需 mock SQLite（`sqlite3_prepare_v2`/`sqlite3_step`），或提供临时 System.cfg 文件 |
| R2 事件管理 | ✅ 可测 | 纯逻辑（handler 表管理），mock 互斥锁即可，`MAX_EVENT_HANDLER` 边界条件易测 |
| R3 告警管理 | ⚠️ 中等 | 需 mock `HandleEvent` 和表达式引擎，状态机转换（ACTIVE→CEASED/SUPPRESSED）可做表驱动测试 |
| R4 表达式运算 | ✅ 可测 | 纯计算逻辑，无外部依赖，`ExpressionParser` + `Exp_Calculate` 可端到端测试 |
| R5 数据处理流 | ⚠️ 中等 | 需 mock 线程和表达式引擎，`Refresh_*` 函数逻辑可单独测试 |
| R6 跨模块访问 | ⚠️ 中等 | `Intern_Get` 需 mock SQLite（METERVALUE_REC 路径）和配置数据结构，参数分发逻辑可测 |
| R7 数据库安全 | ❌ 难 | 依赖 publib `Lock_SqlDB_FileLocker`（`flock`），需文件系统或 mock；`Reopen_AndLockSqliteDB` 已注释不可测 |

---

## Key Files

> 源文件在 `src/moduals/data_processor/`，头文件在 `src/moduals/data_processor/dpr/` 和 `src/include/data_model/`。

| 文件 | 对应 R | 说明 |
|------|:------:|------|
| dpr.h | R1-R7 | 主头文件：`CFG_ERRCODE` 枚举、`CFG_ROW_PARSER` 回调、12 个 `Cfg_Parse_*` 声明、`GetParam_ByPR`/`GetAlarm_ByPR`/`GetDevModel_ById`/`GetDevce_ById`/`GetSamplerById`、`Check_ParamValue`、`Dump_ParamValue`（转储）、`Debug_PrintCfg`（仅调试）、`HandleEvent`/`RestoreHisEvent`、`Register_EventHandler`、`Reopen_AndLockSqliteDB`/`UnLock_SqliteDB`（声明有效，实现已注释） |
| run_expression.h | R4 | 表达式引擎：`TypesUsedByExp` 枚举（12 运算符 + 元素类型）、`Exp_Calculate` Debug/Release 宏、`EXP_GET_CONST_*`/`EXP_IS_VAR` 宏、`ExpressionParser` |
| dataprocessor_model.h | R5, R6 | 数据模型：`ERRCDDE_DPR` 枚举（14 错误码）、`INTERGET_TYPE` 枚举（10 类型）、`INTERSET_TYPE` 枚举（13 类型）、`DATA_PROCESSOR` 结构体（函数指针表）、`INTER_QUEUE_CMD`/`INTERSET_DATA` |
| config_mangmt.c | R1 | 配置加载：`InitCfgMagt`、`GetParam_ByPR`（PARAM_TYPE 分发）、`GetAlarm_ByPR`、`GetDevModel_ById`、`Check_ParamValue`（大小+范围校验）、`Dump_ParamValue`/`Dump_ReadParamValue` |
| event_manager.c | R2, R7 | 事件管理：`Register_EventHandler`（256 槽位 + mutex）、`HandleEvent`（派发 + SQLite 持久化）、9 个索引计数器、`RestoreHisEvent`、`Reopen_AndLockSqliteDB`（实现已注释，见 event_manager.c:155-206） |
| data_processing.c | R3, R5 | 数据处理：`Alarm_Begin`/`Alarm_Cease`/`Alarm_Suppress`、`Refresh_StatusParam`/`Refresh_CtrlParam`/`Refresh_SettParam`/`Refresh_Alarm`/`Refresh_HisParam`、`_ThreadEntry_DataProcessing`（主循环 200ms） |
| dpr_loader.c | R6 | 模块桥接：`Intern_Get`（INTERGET_TYPE 分发）、`GetModuleName`（MODULE_INFO 多态）、`Open_Lock_SqliteDB`/`Close_UnLock_SqliteDB` 调用 |
| run_expression.c | R4 | 表达式运行时：`Exp_CalculateD`/`Exp_CalculateR` 实现、变量值查找、运算符栈式计算 |

## Known Tech Debts

1. **`Reopen_AndLockSqliteDB` 被注释** — `dpr.h` 有声明，`event_manager.c:155-206` 整段注释，未编译。当前使用 publib 的 `Open_Lock_SqliteDB`/`Close_UnLock_SqliteDB` 替代
2. **零测试代码** — 全模块无单元测试，Makefile `TEST_EXEC =`（空）
3. **`data_processing.c` 硬编码查询** — `_ThreadEntry_DataProcessing` 中 `GETTYPE_SP_VAR, 200, 83/81` 为硬编码参数 ID，未配置化
4. **`Alarm_Cease` 注释残留** — `data_processing.c:52-56` 保留被注释的 SUPPRESSED→CEASED 旧逻辑（已被 `!= ALARM_STATUS_CEASED` 替代）
5. **9 个全局索引计数器** — `event_manager.c:4-12` 的静态变量未封装，散落在事件/告警/历史等不同函数中维护
