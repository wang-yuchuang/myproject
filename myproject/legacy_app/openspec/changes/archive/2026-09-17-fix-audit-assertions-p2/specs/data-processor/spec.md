## MODIFIED Requirements

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
