# Spec: samplers — 采样模块

**Subsystem**: samplers
**Version**: 0.4
**Created**: 2026-07-01
**Last Updated**: 2026-07-14

---

## Purpose

数据采集模块，从各类设备和接口采集实时数据。通过统一加载器 `sampler_loader.c` 初始化和调度，**每个采样器编译为独立 `.so`**（**共 10 个**，真值见 `src/Makefile:36-47` 与下方编译清单），在运行时通过 `dlopen` + `dlsym` 动态加载。

架构位置：设备 → **samplers** → DPR → reporters

采样器不直接调用底层通信库，而是通过 `COMMPORT*` 抽象（由 comm-ports 模块提供，见 [comm-ports spec](../comm-ports/spec.md) R1）收发数据，实现传输无关。

**源码位置**：
- 加载器与线程：`src/moduals/samplers/sampler_loader.c` / `sampler_loader.h`
- 接口定义：`src/moduals/samplers/sampler_base.h:68-77`
- 数据模型：`src/include/data_model/sampler_model.h`
- 辅助函数：`src/moduals/samplers/sampler_pubfunc.c`
- **17** 个采样器子目录：`src/moduals/samplers/<name>_sampler/`（注：`thermal_control/` 不带 `_sampler` 后缀）

**编译产物**（**10 个编译 ✅ / 7 个未编译 ❌**；真值来源 `src/Makefile:36-47`）：

| .so 文件 | 采样器目录 | 功能 | 编译 |
|---------|-----------|------|:--:|
| `sampler_whm.so` | `kwhmeter_sampler/` | 电表（RS485 / UART Server） | ✅ |
| `sampler_485_2.so` | `dev_485_2_sampler/` | RS485-2 总线设备 | ✅ |
| `sampler_system.so` | `system_sampler/` | 系统状态 + OTA + LTE Modem | ✅ |
| `sampler_ccu.so` | `ccu_sampler/` | CCU 中央控制单元 | ✅ |
| `sampler_stbatt.so` | `st_batt_sampler/` | 储能电池 BMS | ✅ |
| `sampler_inter_udp.so` | `udp_inter_sampler/` | UDP 交互通信 | ✅ |
| `sampler_uart_server.so` | `uart_server_sampler/` | 串口服务器（DLT645 / Modbus RTU） | ✅ |
| `sampler_aircon.so` | `aircon_sampler/` | 空调 RS485 控制（R12） | ✅ |
| `sampler_auxdev.so` | `auxdev_sampler/` | 辅助设备监控（R13） | ✅ |
| `sampler_thermal.so` | `thermal_control/` | 热管理策略（R14；目录无 `_sampler` 后缀） | ✅ |
| `sampler_charger.so` | `charger_sampler/` | EV 充电桩（Modbus TCP） | ❌ Makefile 注释 |
| `sampler_aems.so` | `aems_sampler/` | AEMS 储能管理 | ❌ Makefile 注释 |
| `sampler_pv_inverter.so` | `pv_inverter_sampler/` | PV 光伏逆变器 | ❌ Makefile 注释 |
| `sampler_imeu.so` | `imeu_sampler/` | IMEU 模块 | ❌ Makefile 注释 |
| `sampler_inter_slave.so` | `inter_slave_sampler/` | 从机通信 | ❌ Makefile 注释 |
| `sampler_cr.so` | `cardreader_sampler/` | RFID 卡阅读器 | ❌ Makefile 注释 |
| `sampler_can2_metershare.so` | `can2_metershare_sampler/` | CAN 总线电表共享 | ❌ 未纳入 DIRS |

---
## Requirements
### Requirement: R1 统一采样器加载

系统 MUST 在启动时通过 `Init_Load_AllSamplers(CHARGER* pCharger)` 统一初始化所有配置的采样器，由 `main.c` 第 5 步（`5/6 Init_Load_AllSamplers`）调用。

**加载流程**（`sampler_loader.c:180-365`）：

1. **curl 初始化**：`curl_global_init(CURL_GLOBAL_ALL)` 在所有采样器加载前调用一次（供 HTTP/FTP 类采样器使用，如 `system_sampler` 的 OTA 下载）
2. **遍历配置数组**：迭代 `pCharger->samplers.p[]`（共 `nTotal` 个条目），每个条目是一个 `SAMPLER` 结构
3. **运行时字段初始化**：设置 `bJustControlled=FALSE`、`bLastSampleInterrupted=FALSE`、`bIsInitlized=FALSE`、`hDPR`、`bExit=FALSE`
4. **条件跳过**（详见 Scenario 2-3）
5. **资源创建**：`Queue_Create(MAX_CTRL_CMD_NUM=32, ...)` + `Mutex_Create(TRUE)`
6. **动态库加载**：`LoadSamplerLib()` → `dlopen` + `dlsym` 解析 4 个符号
7. **线程创建**：`RunThread_Create()` 启动 `__Thread_ASampler` 采样线程
8. **初始化回调**：调用 `pSampler->Init()`，失败则 `RunThread_Stop` 后返回 `FALSE`

**依赖**：`dlfcn.h`（dlopen/dlsym）、`3rd_party/curl`（curl_global_init）、publib 线程/队列/互斥锁

#### Scenario: 采样器全量加载成功

- **WHEN** 系统启动执行 `Init_Load_AllSamplers()`，所有配置的采样器满足加载条件
- **THEN** 每个采样器依次完成：队列/互斥锁创建 → `.so` 加载 → 线程启动 → `Initialize()` 回调
- **AND** `Initialize()` 返回 `TRUE` 后设置 `bIsInitlized = TRUE`，采样线程进入主循环
- **AND** 函数遍历完所有 `nTotal` 个条目后返回 `TRUE`

#### Scenario: 平台条件跳过（IMMU3）

- **WHEN** 编译时定义了 `#define IMMU3` 宏（`sampler_loader.c:204` 的 `#ifdef IMMU3`）
- **AND** 遍历到 `nSamplerID == 4` 的采样器
- **THEN** 该采样器被跳过（`continue`），不创建队列/互斥锁/线程
- **AND** 输出 `TRACEW` 日志记录跳过原因：`"Init_Load_AllSamplers continue IMMU3 not load nSamplerID:4"`

#### Scenario: 业务类型条件跳过（SEMS / AEMS）

- **WHEN** 首次遍历时从 DPR 读取系统类型（`pDPR->Get(GETTYPE_ST_VAR, 1, 100, ...)` 判断 SEMS，`1, 73` 判断 AEMS）
- **AND** 系统为 SEMS 类型（`bSEMS == TRUE`）且采样器 `nSamplerID` 在 4~6 范围内
- **THEN** 该采样器被跳过（`continue`），输出 `TRACEW` 日志
- **AND** 当系统非 AEMS 类型（`bAEMS == FALSE`）且 `nSamplerID == 9` 时，同样跳过

#### Scenario: 设备安装状态条件跳过

- **WHEN** 从 DPR 读取设备安装状态参数为 0（未安装）
- **THEN** 对应采样器被跳过：
  - `nSamplerID == 2`：INFY Charger 未安装（**无条件跳过**，旧代码 `1/56` 检查已注释）
  - `nSamplerID == 6`：UDP 设备未安装（param `1/8 == 0`）
  - `nSamplerID == 7`：UART Server 未安装（param `901/910 == 0`）
  - `nSamplerID == 8`：PV 逆变器未安装（param `1050/458 == 0`）
  - `nSamplerID == 10`：IMEU3 未安装（param `621/409 == 0`）
- **AND** 跳过时输出 `TRACEW` 或 `TRACED` 日志记录

#### Scenario: 动态库加载失败

- **WHEN** `LoadSamplerLib()` 中 `dlopen` 或 `dlsym` 失败（`hSamplerLib == NULL`）
- **THEN** 输出 `LOG_TYPE_ERROR` 日志：`"Fails on loading sampler lib --> [%s]"`
- **AND** `Init_Load_AllSamplers()` 返回 `FALSE`，整个加载流程中止
- **AND** 此前已成功加载的采样器线程继续运行

---

### Requirement: R2 采样线程调度与命令队列

系统 MUST 为每个已加载的采样器创建独立采样线程（`__Thread_ASampler`），按配置周期调度采样并处理 DPR 下发的控制命令。

**线程主循环**（`sampler_loader.c:23-143`）：

```
while (!bExit):
    RUN_THREAD_HEARTBEAT()              ← syshand 心跳
    if (!bIsInitlized): Sleep(100); continue

    # 1. 优先处理命令队列（每轮最多 6 条）
    for (nMaxExeCmd = 6; nMaxExeCmd >= 0; nMaxExeCmd--):
        cmd = Queue_Get(hqControlCommand, WAIT_CMD_QUEUE=100ms)
        if (empty): break
        Control(... cmd ...)             ← 执行控制命令
        if (OK): DPR->Set(SETTYPE_RAW_BACK_*)  ← 回写结果
        bJustControlled = TRUE

    # 2. 判断是否到采样时间
    if (IsTimeToSample()):
        err = Sample(hSampler)
        if (TIMEOUT or NEED_RESAMP): bLastSampleInterrupted = TRUE
        else: tmLastSampled = tmNow
```

**依赖**：publib（Queue / Mutex / Thread / RUN_THREAD_HEARTBEAT）、DPR Set/Get API

#### Scenario: 定周期采样触发

- **WHEN** 当前时间 `tmNow` 与上次采样时间 `tmLastSampled` 的差值 ≥ `nSamplePeriod_mSec / 1000.0` 秒
- **THEN** `IsTimeToSample()` 返回 `TRUE`，调用 `Sample()` 执行一次采样
- **AND** 采样成功（返回 `ERR_SAMPLE_OK`）后更新 `tmLastSampled = tmNow`
- **AND** 采样超时（`ERR_SAMPLE_TIMEOUT`）或需重采（`ERR_SAMPLE_NEED_RESAMP`）时设置 `bLastSampleInterrupted = TRUE`，下一轮立即重试

#### Scenario: 控制命令优先处理

- **WHEN** DPR 向采样器命令队列 `hqControlCommand` 写入 `INTER_QUEUE_CMD`
- **THEN** 线程主循环优先从队列取命令（每轮最多处理 6 条），调用 `Control()` 执行
- **AND** 命令类型 `SETTYPE_INTERN_CTRL` 映射为 `PARAM_TYPE_CONTROL`，其他映射为 `PARAM_TYPE_SETTING`
- **AND** `Control()` 返回 `ERR_CONTROL_OK` 后，通过 `DPR->Set(SETTYPE_RAW_BACK_*)` 将新值回写 DPR
- **AND** 设置 `bJustControlled = TRUE`，触发下一轮立即采样（不等周期）

#### Scenario: syshand 心跳保活

- **WHEN** 采样线程运行中
- **THEN** 每次循环迭代调用 `RUN_THREAD_HEARTBEAT()` 向 syshand 守护进程发送心跳
- **AND** 心跳间隔由采样周期决定（若周期过长，线程在 `Queue_Get` 的 `WAIT_CMD_QUEUE=100ms` 超时等待中循环，保证心跳频率）

#### Scenario: 线程退出与系统重启

- **WHEN** 采样线程检测到 `bExit == TRUE`（由外部设置）
- **THEN** 退出 `while` 循环
- **AND** 返回 `THREAD_CANCEL_ALL`（表示需要系统级重启，而非仅停止当前线程）

---

### Requirement: R3 统一接口规范与错误码

每个采样器 `.so` MUST 导出 4 个 `DLLExport` 函数，由加载器通过 `dlsym` 解析。函数原型定义在 `sampler_base.h:68-77`，类型定义在 `sampler_model.h`。

**4 个 DLLExport 函数**：

| 函数 | 原型 | 返回值 | 用途 |
|------|------|--------|------|
| `Initialize` | `BOOL Initialize(HANDLE hSamplerSelf)` | `TRUE`=成功 | 初始化：分配 `pDataHeap`、打开通信端口、注册事件处理器 |
| `Sample` | `int Sample(HANDLE hSamplerThis)` | `ERR_SAMPLE_*` | 执行一次采样周期，采集数据并推送到 DPR |
| `Control` | `int Control(HANDLE, PARAM_TYPE, int nDevId, int nParamId, VAR_VALUE*, int nValSize, int nTimeOut)` | `ERR_CONTROL_*` | 接收 DPR 下发的控制/设置命令 |
| `JigTest` | `int JigTest(HANDLE hSamplerThis, void* pJigData)` | int | 生产测试模式（工装检测），可选实现 |

**错误码**（`sampler_model.h:7-19`）：

| 错误码 | 值 | 含义 |
|--------|---|------|
| `ERR_SAMPLE_OK` | 0 | 采样成功 |
| `ERR_SAMPLE_TIMEOUT` | 1 | 采样超时（设备无响应） |
| `ERR_SAMPLE_NEED_RESAMP` | 2 | 需要重新采样（数据不完整） |
| `ERR_SAMPLE_FAILED` | 3 | 采样失败 |
| `ERR_CONTROL_OK` | 0 | 控制成功 |
| `ERR_CONTROL_FAILED` | 1 | 控制失败 |
| `ERR_CONTROL_TIMEOUT` | 2 | 控制超时 |
| `ERR_CONTROL_COMFAIL` | 3 | 通信失败 |
| `ERR_CONTROL_NOTSUPPORTED` | 4 | 不支持的操作 |

**SAMPLER 结构关键字段**（`sampler_model.h:74-117`）：
- `SAMPLER_INFO info`：`emModuleType` / `nSamplerID` / `szSamplerName[32]` / `szSamplerDriver[32]` / `szSamplerSetting[64]` / `nSamplePeriod_mSec`
- `COMMPORT* pUsedCommPort` + `HANDLE hCommPort`：通信端口抽象
- `HANDLE hSamplerLib`：dlopen 句柄
- `HANDLE hSampleThread`：采样线程
- `HANDLE hSamplerLock`：互斥锁
- `HANDLE hDPR`：DPR 句柄（用于 Get/Set 数据）
- `void* pDataHeap`：采样器私有数据堆
- 函数指针：`Init` / `Sample` / `Control` / `JigTest` / `Notify`
- `HANDLE hqControlCommand`：命令队列（容量 `MAX_CTRL_CMD_NUM=32`）
- `BYTE emWorkStatus`：0=Working / 1=Disconnected / 2=Non-working
- `double tmLastSampled`：上次采样时间戳
- `double fElapsedTime`：距上次采样的流逝时间
- `BOOL bJustControlled`：刚执行控制命令，触发下轮立即采样（不等周期）
- `BOOL bLastSampleInterrupted`：上次采样被中断，下轮立即重试
- `BOOL bIsInitlized`：初始化完成标志
- `BOOL bExit`：线程退出标志

#### Scenario: 采样器初始化标准流程

- **WHEN** 加载器调用 `Initialize(hSamplerSelf)`
- **THEN** 采样器通过 `NEW(XXX_ROUGH_DATA, 1)` 分配私有数据堆并赋值给 `pSampler->pDataHeap`
- **AND** 初始化私有数据中的 `hSampler`、`nSamplerId`、`pDPR`、`hSamplerLock` 字段
- **AND** 可选地打开通信端口、注册 DPR 事件处理器（`Register_EvHandler`）
- **AND** 返回 `TRUE` 表示初始化成功

#### Scenario: dlsym 符号解析

- **WHEN** `LoadSamplerLib()` 调用 `LoadDynamicLibrary()` 加载 `.so`
- **THEN** 解析 4 个符号名：`"Initialize"` / `"Sample"` / `"Control"` / `"JigTest"`（定义于 `SAMPLER_*_PROC_NAME` 宏）
- **AND** 解析结果存入 `pSampler->Init` / `Sample` / `Control` / `JigTest` 函数指针
- **AND** `.so` 路径由 `$APP_PATH/sampler/<szSamplerDriver>` 拼接

#### Scenario: 工装测试模式

- **WHEN** 系统进入生产测试模式（`pJigData->bEnterTestMode == TRUE`）
- **THEN** 加载器线程循环中调用 `JigTest(&info, pJigData)` 而非正常 `Sample()`
- **AND** `JigTest` 执行工装专用测试逻辑（如电池短时放电测试、两步测试等，见 `st_batt_testmode*.c`）
- **AND** 返回测试结果供工装判定

---

### Requirement: R4 DPR 数据交互辅助函数

系统 MUST 提供一组公共辅助函数（`sampler_pubfunc.c`），供所有采样器向 DPR 推送采集数据和发送事件通知。函数原型声明在 `sampler_base.h:80-105`。

**核心辅助函数**：

| 函数 | 用途 |
|------|------|
| `SetCommStatus(hSampler, nDevId, bCommOK)` | 设置通信状态，触发 `EVENT_TYPE_COMM_FAIL` 事件 |
| `Push_VarValue(setType, hSampler, nDevId, nParamId, varV)` | 推送 4 字节标量值到 DPR（`SETTYPE_RAW_ST` / `SETTYPE_INTERN_CTRL`） |
| `Push_VarValue_RereadPCS(setType, hSampler, nDevId, nParamId, RereadvarV, nWhichSingleModule)` | 推送 PCS 重读标量值到 DPR（`sampler_base.h:82`） |
| `Push_BlobValue(setType, hSampler, nDevId, nParamId, pBlob, nValSize, tp)` | 推送 BLOB 数据（如网络参数、管理计划） |
| `Push_BlobValue_Ex(setType, hSampler, nDevId, nParamId, pBlob, nValSize, tp, nTimeOut_MSec)` | 推送 BLOB 数据（带超时，`sampler_pubfunc.c:177`） |
| `Push_TextValue(setType, hSampler, nDevId, nParamId, pString, nValSize)` | 推送文本数据 |
| `SendMsg(hSampler, evTp, msgTp, pMsgData, nSize)` | 发送非阻塞事件通知到 DPR |
| `SendMsg_Urgent(...)` | 发送阻塞模式紧急事件 |
| `GetESSysType(hSampler)` | 获取储能系统类型 |
| `Samp_Avg_Filter(emType, fRawVal, nPointNum)` | 采样值均值滤波 |

**依赖**：DPR Set/Get API（`DATA_PROCESSOR->Set` / `Get`）

#### Scenario: 采集数据推送

- **WHEN** 采样器在 `Sample()` 中采集到设备数据
- **THEN** 通过 `Push_VarValue(SETTYPE_RAW_ST, hSampler, nDevId, nParamId, varV)` 推送到 DPR
- **AND** DPR 将数据存储为原始值（raw value），供 reporters 上报和表达式计算使用
- **AND** 当值为 `SAMP_INVALID_VALUE` 时自动设置 `bRawInvalid = TRUE`

#### Scenario: 通信故障事件通知

- **WHEN** 采样器检测到设备通信失败（如 RS485 无响应、TCP 断开）
- **THEN** 调用 `SetCommStatus(hSampler, nDevId, FALSE)`
- **AND** 触发 `EVENT_TYPE_COMM_FAIL` + `EVENT_MSG_BEGIN` 事件
- **AND** 事件参数包含 `OBJ_PTR {nId=nSamplerID, nSubId=nDevId}`（`nDevId=-1` 表示该采样器所有设备通信故障）
- **AND** 通信恢复后调用 `SetCommStatus(..., TRUE)` 触发 `EVENT_MSG_END`

#### Scenario: 紧急事件发送

- **WHEN** 采样器需要立即通知 DPR 重要事件（如电池严重告警）
- **THEN** 调用 `SendMsg_Urgent(hSampler, evTp, msgTp, pMsgData, nSize)`
- **AND** 事件以阻塞模式（`bBlockMode = TRUE`）发送，确保 DPR 及时处理

---

### Requirement: R5 电表采样（kwhmeter_sampler）

系统 MUST 支持通过 RS485 总线或 UART Server（TCP）读取电表数据（电压、电流、功率、电能等），编译为 `sampler_whm.so`。

**源文件**：`kwhmeter_sampler/rs485_whim_sampler_main.c` + `kwhmeter.c`
**采样周期**：`RS485_SAMPLER_PERIOD = 100`（100ms）
**支持类型**（`sampler_base.h:31-38` `METER_TYPE`）：`dc`（直流）/ `ac`（交流）/ `DataFromMaster`（主控转发）/ `UartServer`（串口服务器）

**依赖**：comm-ports RS485 模块、comm-ports UART Server 模块、DPR

#### Scenario: RS485 直连电表读取

- **WHEN** 电表通过 RS485 总线直连，`METER_TYPE` 为 `dc` 或 `ac`
- **THEN** 采样器通过 `s_pRS485_2_Comm`（RS485 通信端口）发送 Modbus/DLT645 查询帧
- **AND** 解析响应帧获取电压、电流、有功功率、无功功率、正向电能、反向电能等参数
- **AND** 通过 `Push_VarValue(SETTYPE_RAW_ST, ...)` 推送到 DPR

#### Scenario: UART Server 模式电表读取

- **WHEN** 电表类型为 `METER_TYPR_UartServer`，电表通过远程串口服务器（TCP）接入
- **THEN** 采样器通过 `s_pUartServer_Comm`（TCP 通信端口）连接串口服务器
- **AND** 透传 Modbus/DLT645 协议帧到远程电表

#### Scenario: 配置变更热响应

- **WHEN** DPR 触发 `EVENT_TYPE_CFG_CHANGED` 事件（如防逆流 `1/40`、防过载 `1/41` 配置变更）
- **THEN** `Event_Handler` 从 DPR 重新读取变更的配置
- **AND** 更新 `pWhim_Rough->needAntiReflux_Phase` / `needAntiOverLoad_Phase` 标志
- **AND** 返回 `EVENT_HANDED_OK` 或失败时返回 `EVENT_HANDED_FAILED`

---

### Requirement: R6 RS485-2 总线设备采样（dev_485_2_sampler）

系统 MUST 支持通过第二路 RS485 总线（`dev_485_2`）采集多种第三方设备数据，编译为 `sampler_485_2.so`。

**源文件**：`dev_485_2_sampler/dev_485_2_sampler_main.c` + 子模块
**支持的设备类型**（同目录下的独立 `.c` 文件）：
- `battcab_safety_management.c` — 电池柜安全管理
- `imeu2_v20_batt_cab.c` — IMEU2 v2.0 电池柜
- `lmlu2.c` — LMLU2 设备
- `mppt_hopesun_main.c` — MPPT 光伏控制器
- `load_ac_kwhmeter.c` — 负载侧交流电表

**依赖**：comm-ports RS485 模块、DPR

#### Scenario: 多设备轮询

- **WHEN** RS485-2 总线上挂载多个设备
- **THEN** 采样器按配置列表依次轮询每个设备
- **AND** 每个设备使用独立的 Modbus 地址和协议帧格式
- **AND** 设备无响应时通过 `SetCommStatus()` 上报通信故障

#### Scenario: 协议帧校验

- **WHEN** 采样器构造 RS485 请求帧（HexToAscii / AsciiToHex 转换）
- **THEN** 使用 `CalcSendCheckSum()` 计算校验和（反码求和）
- **AND** 使用 `Make_YDN_Lengh()` 编码长度字段（含长度校验字节）
- **AND** 接收帧时校验长度和校验和，不匹配则丢弃

---

### Requirement: R7 系统状态采样与 OTA/LTE 管理（system_sampler）

系统 MUST 定期采集系统自身运行状态，并提供 OTA 固件升级与 LTE 4G modem 管理能力。编译为 `sampler_system.so`。此采样器功能最复杂（4552 行）。

**源文件**：
- `sampler_system.c`（4552 行）— 主文件：系统状态采集、网络信息、告警、云平台数据
- `system_upgrade.c`（841 行）— OTA 固件升级流程（BSU/BMU/IMEU/IMSU 等多设备）
- `int_modem_mgmt.c` — LTE 4G 模块管理（AT 命令状态机）
- `ext_router_mgmt.c` — 外置路由器管理

**依赖**：`3rd_party/curl`（HTTP/FTP 下载）、`3rd_party/cJSON`（JSON 解析）、`ioctl`（网络信息）、AT 命令串口

#### Scenario: 系统状态周期采集

- **WHEN** `Sample()` 被线程调度调用
- **THEN** 采集并刷新以下系统状态：
  - **网络信息**（每 5 秒）：通过 `ioctl(SIOCGIFADDR/SIOCGIFNETMASK/SIOCGIFHWADDR)` 获取 IP/掩码/MAC，读取 `/proc/net/route` 获取网关
  - **系统时钟**：`Push_VarValue(1, 1, _NOW_)` 推送当前时间戳
  - **运行时长**：计算 `fRunSincePowerOn_Min`（上电后运行分钟数）
  - **可用状态**：`RefreshSysAvailableStatus()` — WLAN/云平台连接状态
  - **告警状态**：`RefreshSysAlarmStatus()` — 系统级告警
- **AND** 每次调用 `Manage_LTEModem()` 管理 LTE modem 状态机

#### Scenario: OTA 固件升级流程

- **WHEN** `emDownloadStatus != UPDOWN_IDLE`（收到固件升级触发）
- **THEN** 调用 `HandleFirmwareUpgrading()` 执行升级流程
- **AND** 升级文件来源：`/media/sda1/upgradefile`（SD 卡一键升级）或 `/opt/appdata/data/upgradefile`（Web 上传）
- **AND** **电池侧设备**（BSU→BMU→IMEU2→IMEU3→AIRIMEU3）按顺序升级，BSU 必须先于 BMU
- **AND** **模块侧设备**（N2101U1/U2→N2102U1/U2→2302U1/U2→IMSU）按顺序升级，IMSU 必须最后
- **AND** 升级进度和结果通过 `SendMsg(EVENT_TYPE_UI_NOTICE)` 通知 UI

#### Scenario: LTE 4G Modem 管理

- **WHEN** `Manage_LTEModem()` 被 `Sample()` 调用
- **THEN** LTE modem 状态机按以下状态流转：
  - `NOT_INIT` → `IDLE` → `DIALING` → `READY`（拨号成功）/ `DIAL_FAIL`（拨号失败）→ `WAIT_RESET`（复位等待）
- **AND** 通过 AT 命令（`/dev/ttyUSB*`）执行：查询 SIM 状态 / CSQ 信号 / IMSI / ICCID / PDP 激活 / ECM 拨号
- **AND** 返回 IMSI / ICCID / APN / 网络类型 / 信号强度等信息到 DPR

#### Scenario: 诊断日志上传

- **WHEN** `emUploadStatus != UPDOWN_IDLE`（收到诊断上传触发）
- **THEN** 调用 `HandleDiagnoseUploadiing()` 执行日志打包上传
- **AND** 使用 curl 将诊断数据上传到云平台

---

### Requirement: R8 CCU 中央控制单元采样（ccu_sampler）

系统 MUST 支持与 CCU（中央控制单元）通信并采集 PCS（功率变换系统）和 EMS（能量管理系统）数据。编译为 `sampler_ccu.so`。此采样器是子系统最多的采样器（约 47 个文件）。

**源文件**：`ccu_sampler/ccu_sampler_main.c` + 多变体实现
**支持的系统变体**（通过条件编译/运行时分支选择）：
- `ccu_sampler_main.c` — 标准 CCU
- `ess_sampler_main.c` — ESS 储能系统
- `sems_sampler_main.c` — SEMS 站级储能
- `inpcs_sampler_main.c` — 内置 PCS

**通信传输方式**（3 种，由配置决定）：
- `ccu_com_232.c` — RS232 串口（`CCU_BUFF_LEN_232 = 8192`）
- `ccu_com_eth.c` — 以太网 TCP
- `ccu_com_rpmsg.c` — RPMsg（核间通信，D9360 AP↔R 核）

**依赖**：comm-ports RS232/TCP/RPMsg 模块、DPR、EMS 管理计划

#### Scenario: CCU 协议通信

- **WHEN** 采样器与 CCU 建立通信（通过 `CCU_Check_And_ReOpen_CommPort()` 检查并打开端口）
- **THEN** 按 CCU 协议帧格式发送查询/控制指令
- **AND** 等待 CCU 响应（`CCU_Wait_ACCU_Reply_RS232()`，带超时）
- **AND** 通过 `ccu_protocol_parsers.c` / `ccu_rpmsg_parsers.c` 解析响应帧
- **AND** 解析 PCS 状态、功率、电压、电流等数据推送到 DPR

#### Scenario: EMS 管理计划初始化

- **WHEN** CCU 采样器初始化时（`Init_blobEmsCfgPlanParam()`）
- **THEN** 从 DPR 读取 EMS 配置计划（param `200/201`，`A_BLOB_EMS_CONFIG_PLAN`）
- **AND** 若计划段数 `< 1`（未配置），创建默认 9 段计划（含峰/平/谷时段、充放电模式、功率限制）
- **AND** 通过 `Save_EmsMgmtPlan()` 保存计划供后续调度使用

#### Scenario: CCU 固件升级

- **WHEN** 收到 CCU 固件升级指令
- **THEN** 从 `/app/ccu_bin/`（`CCU_FIRMWARE_UPGRADE_PATH`）读取升级文件
- **AND** 通过 `ccu_bin_upgrade.c` 的升级流程传输固件到 CCU

#### Scenario: 多变体运行时适配

- **WHEN** 系统配置为 SEMS / ESS / InPCS 不同变体
- **THEN** 采样器加载对应的状态机（`sems_statemachine.c` / `inpcs_statemachine.c` / `inpcs235kw_statemachine.c`）
- **AND** 使用对应的 EMS 管理逻辑（`sems_ems_management.c` / `ess_ems_management.c` / `ems_management.c`）

---

### Requirement: R9 储能电池 BMS 采样（st_batt_sampler）

系统 MUST 支持通过 BMS 协议采集储能电池数据，支持多厂商和多总线。编译为 `sampler_stbatt.so`。

**源文件**：`st_batt_sampler/st_batt_sampler.c` + 多厂商 BMS 实现
**支持的电池厂商/协议**（独立 `.c` 文件）：
- `can_infy_batt.c` — 英飞源 BMS（CAN 总线）
- `can_nd_batt.c` / `can_nd_v33_batt.c` — ND / ND v3.3 BMS（CAN 总线）
- `can_pl_batt.c` — PL BMS（CAN 总线）
- `modbus_fengch_batt.c` — 丰晨 BMS（Modbus RS485）
- `rpmsg_infy_batt.c` — 英飞源 BMS（RPMsg 核间通信）
- `imeu2_can.c` / `imeu3_can.c` — IMEU2/IMEU3 转发的 CAN 数据

**SOC 校准表**（`st_batt_sampler.c:9-56`）：内置 285Ah CATL 电芯的电压-SOC 映射表（充电/放电各有独立表），用于开路电压法 SOC 校准。

**依赖**：comm-ports CAN/RS485/RPMsg 模块、DPR

#### Scenario: 电池数据采集

- **WHEN** 采样器轮询 BMS
- **THEN** 采集以下数据并推送到 DPR：单体电压、总电压、总电流、SOC、SOH、温度、告警/故障状态、电池组信息
- **AND** 通过 `Push_VarValue(SETTYPE_RAW_ST, ...)` 推送标量数据
- **AND** 通过 `Push_BlobValue(...)` 推送 BLOB 数据（如电池管理计划 `A_BLOB_BATT_MGMT_PLAN`）

#### Scenario: SOC 电压校准

- **WHEN** BMS 上报的 SOC 需要校准（基于开路电压）
- **THEN** 使用 `ChargeSOCMap[]` / `DisChargeSOCMap[]` 查找表，根据当前单体电压插值计算 SOC
- **AND** 充电过程和放电过程使用不同的映射表

#### Scenario: 配置变更响应（待机模式）

- **WHEN** DPR 触发 `EVENT_TYPE_CFG_CHANGED`（如 `1/85` 待机设置 `bStandby_Set` 变更）
- **THEN** `Event_Handler` 从 DPR 重新读取配置
- **AND** 更新 `pSTBatt_Rough->bStandby_Set` 标志，加快自动充放响应

#### Scenario: 工装测试模式

- **WHEN** 进入工装测试模式
- **THEN** 执行 `st_batt_testmode.c`（标准测试）/ `st_batt_testmode_ShortTime.c`（短时测试）/ `st_batt_testmode_tow.c`（双步测试）
- **AND** 测试结果通过 `st_batt_datasave.c` / `st_batt_health.c` 保存和健康度计算

---

### Requirement: R10 UDP 交互采样（udp_inter_sampler）

系统 MUST 支持通过 UDP 协议与其他设备/系统进行交互通信（客户端 + 服务端模式），编译为 `sampler_inter_udp.so`。仅当 DPR param `1/8 != 0`（UDP 设备已安装）时加载。

**源文件**：
- `udp_public_main.c` — 公共初始化与工具函数
- `sub_udpclient_main.c` / `sub_udpclient_cmd_handler.c` — UDP 客户端
- `sub_udpserver_main.c` / `sub_udpserver_cmd_handler.c` — UDP 服务端

**依赖**：comm-ports UDP 模块、DPR、EMS 管理计划同步

#### Scenario: UDP 通信初始化

- **WHEN** `UdpPublic_Initialize()` 被调用
- **THEN** 初始化 `STU_UDP_ROUGHDATA` 私有数据
- **AND** 为每个储能系统类型（`ESSTYPE_MAX`）初始化 `tmLastSuccessExchange` / `fKWh_Pos` / `bRegisted`
- **AND** 从 DPR 读取 EMS 管理计划（param `200/201`）

#### Scenario: UDP 客户端/服务端交互

- **WHEN** UDP 采样器运行
- **THEN** 客户端模式主动发送数据帧到对端设备
- **AND** 服务端模式监听 UDP 端口接收对端数据
- **AND** 通过 `Make_UniqID_UDP()` 生成唯一消息 ID（基于时间戳 + 递增序号）
- **AND** 帧中包含时间字符串（`Make_UdpTimeString()`）和 IP 地址（`Make_IPV42String()`）

#### Scenario: EMS 管理计划同步

- **WHEN** UDP 设备间需要同步 EMS 管理计划
- **THEN** 通过 `tm_LastSyncEmsMgmt` / `tm_LastRecvEmsMgmt` 跟踪同步时间
- **AND** 使用 `isIPAddrMatchSysTp()` 验证 IP 地址与系统类型匹配

---

### Requirement: R11 串口服务采样（uart_server_sampler）

系统 MUST 支持串口服务器模式，作为 TCP Server 接收远程串口设备的数据（DLT645 电表 / Modbus RTU 设备）。编译为 `sampler_uart_server.so`。仅当 DPR param `901/910 != 0`（UART Server 已安装）时加载。

**源文件**：
- `uart_server_main.c` — 主逻辑（TCP Server + 多客户端管理）
- `dlt645_kWmeter.c` — DLT645 协议电表解析
- `modbus_rtu.c` — Modbus RTU 协议解析

**依赖**：comm-ports TCP Server 模块（`COMM_SERVER_MODE`）、DPR

#### Scenario: TCP Server 模式监听

- **WHEN** UART Server 采样器初始化
- **THEN** 通过 `UART_SERVER_TCP_ReopenServerPort()` 以 `COMM_SERVER_MODE` 打开 TCP 端口（默认 `192.168.1.100:8899`）
- **AND** 通过 `Control(COMM_SET_MAX_CLIENTS)` 设置最大客户端连接数（`TCP_CLIENT_NUM_MAX`）
- **AND** 接受客户端连接后透传串口数据

#### Scenario: DLT645 电表数据解析

- **WHEN** 远程 DLT645 电表通过 TCP 发送数据帧
- **THEN** `dlt645_kWmeter.c` 解析 DLT645-2007 协议帧
- **AND** 提取电压、电流、功率、电能等数据推送到 DPR

#### Scenario: 电能数据持久化恢复

- **WHEN** 收到 `EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW` + `EVENT_MSG_RESTORE` 事件
- **THEN** 从 `EVENT_METER_SAVE_DATA` 恢复历史电能数据
- **AND** 仅对 `ndevid == 401` 的电表恢复（保存当前 SPFW kwh）
- **AND** 更新 `meterData[0]` 的年/月/日保存时间戳

---


> **采样器清单与文档覆盖状态**（2026-09-17 核对，推翻本节点原有说法）：
>
> - **已详细记录（R5–R14）** —— 10 个编译中的全部：`kwhmeter_sampler`(R5) / `dev_485_2_sampler`(R6) / `system_sampler`(R7) / `ccu_sampler`(R8) / `st_batt_sampler`(R9) / `udp_inter_sampler`(R10) / `uart_server_sampler`(R11) / `aircon_sampler`(R12) / `auxdev_sampler`(R13) / `thermal_control`(R14)
> - **未编译、故无 Requirement**：`charger_sampler` / `aems_sampler` / `pv_inverter_sampler` / `imeu_sampler` / `inter_slave_sampler` / `cardreader_sampler` / `can2_metershare_sampler`
>
> ⚠️ 本节点原写为「未详细记录的采样器（**R12–R16**）」并声称「以下 **9 个**采样器**均正常编译**」—— 两处均错：R12–R14 实际已在本 spec 中详细记录，而上列 7 个**并未编译**（真值见 `src/Makefile:36-47`）。

### Requirement: R12 空调采样（aircon_sampler）

系统 MUST 通过 `sampler_aircon.so`（Sampler_ID=11）经 RS485 Modbus RTU 采集空调/水冷机组（Device 951 / DeviceModel 950）的运行数据与故障位图，并承接 `thermal_control` 下发的控制设定，形成「策略下发 → 执行 → 回读」闭环。

**绑定与周期**（`src/config/System.sql:4346`、`aircon_sampler_main.h:14,30-35`）：
- Sampler_ID = 11，CommPort_ID = 13（`RS485_AIRCON` / `ttyS9`），串口参数 `9600,N,8,1`，采样周期 1000 ms
- 驱动来源：`aircon_sampler_main.c`，复用 `pv_inverter_sampler/pv_modbus_rtu.c` 的 Modbus RTU 层（`aircon_sampler/Makefile:17`）

**参数空间**：
- 同飞（TF）液冷机组：`TF_PARAM_*` 1770-1810 —— 启停/工作模式/控制对象、冷热设定温度与温差、液温/回液温/蒸汽进出口温、环境温、冷凝温、EEV 开度、泵速、压缩机转速、风机转速、水压，以及系统高低压、液位、泵过载/堵转/缺相/欠压/超压、热管过载、风机过载、加热带、泵空转保护等故障位（`aircon_sampler_main.h:41-87`）
- 旧机型兼容：`EU3_PARAM_LEGACY_*` 1583-1597，从 `sensor0` / `syssta0` / `comp0` / `compon` 四个位图按位解析压缩机与系统故障（`aircon_sampler_main.c:632-647`）

**通信失效门控**：连续失败达阈值后置通信中断状态（`aircon_sampler_main.h:26`）；风冷/液冷安装类型切换时清通信告警并门控采样。

#### Scenario: 周期采集与故障位图解析

- **WHEN** 采样周期到达（1000 ms）且 RS485 通信正常
- **THEN** 通过 Modbus RTU 读取 TF 参数块，按类型推送：枚举原样、浮点按 `raw*0.1` 缩放（如液温、压力），经 `pushParamEnum` / 浮点推送进入 DPR
- **AND** 对旧机型额外解析 `EU3_PARAM_LEGACY_*` 位图，缺失位在 V3.00 上固定推 0

#### Scenario: 通信连续失败

- **WHEN** 读取连续失败次数达到阈值
- **THEN** 标记为通信中断状态并触发相应告警
- **AND** 按安装类型（风冷/液冷）决定是否门控采样

#### Scenario: 承接热管理策略下发

- **WHEN** `thermal_control` 通过 `Push_VarValue(SETTYPE_INTERN_SETT, …, devId=951, param=2203, …)` 下发模式或设定值
- **THEN** `Control()` 回调将设定写入机组 Modbus 寄存器
- **AND** 回读结果以 `SETTYPE_RAW_BACK_CTRL` 回写 DPR

### Requirement: R13 辅助设备采样（auxdev_sampler）

系统 MUST 通过 `sampler_auxdev.so`（Sampler_ID=12）在共享 RS485 总线上同时驱动 **U3Z 水冷机**与 **IMLU2 灯带**两类辅助设备，并根据电池簇/系统状态自动判决灯带颜色。

**绑定与周期**（`src/config/System.sql:4347`、`auxdev_sampler_main.h:12,40-43`）：
- Sampler_ID = 12，CommPort_ID = 14（`RS485_AUXDEV` / `ttyS5` = UART5），`9600,N,8,1`，采样周期 500 ms
- 设备：水冷机 `CHILLER_DEV_ID = 961`（DeviceModel 960）；灯带 `LED_DEV_ID = 966`（DeviceModel 970）
- 驱动来源：`auxdev_sampler_main.c` + `u3z_protocol.c` + `imlu2_protocol.c`（`auxdev_sampler/Makefile:17`）
- 角色区分：`AUXDEV_ROLE_LED` 标识灯带分支（`auxdev_sampler_main.h:83`）

**灯带自动判决数据源**（`auxdev_sampler_main.h:67-77`）——只读取，不产生这些量：
- 电池簇：`LED_DPR_DEV_BATTGRP` = 501 / `LED_DPR_DEV_BATTGRP2` = 502，读取 `LED_DPR_PARAM_RUNSTATUS` = 550（4=充电 5=放电）与 `LED_DPR_PARAM_WORKSTATUS` = 552（3=故障），由 `st_batt_sampler` 经 RPMsg 推送
- 系统级：`LED_DPR_DEV_CCU` = 200，读取 `LED_DPR_PARAM_SYSSTATUS` = 103（2=Faulted），由 `ccu_sampler` 推送
- 集群数量：`LED_DPR_BATT_GROUP_NUM` = 2（两簇共用水冷机/灯带）
- 灯带通信状态：`LED_PARAM_COMMSTAT` = 1939

#### Scenario: 水冷机采集

- **WHEN** 采样周期到达（500 ms）
- **THEN** 通过 U3Z 协议在 UART5 上读取水冷机状态并推入 DPR（devId 961）
- **AND** 失败时按通信失效策略处理

#### Scenario: 灯带颜色自动判决

- **WHEN** 任一电池簇运行状态（550）或告警状态（552）变化，或系统状态（103）变为 Faulted
- **THEN** 组合 2 个电池簇与系统级状态，计算目标颜色
- **AND** 与 `lastSentColor` 比较，仅在变化时经 IMLU2 协议下发（`pendingColor` 缓存未确认的目标值）

### Requirement: R14 热管理策略采样（thermal_control）

系统 MUST 以**纯计算型采样器** `sampler_thermal.so`（Sampler_ID=13，无 CommPort）实现公式化热管理策略：读取电池与核间聚合数据，经 `T_decision` 公式 + 滞回状态机 + 时间保护计算出目标动作，再下发给空调采样器。

**绑定与周期**（`src/config/System.sql:4348`、`thermal_control_main.h:15,24-28`）：
- Sampler_ID = 13，`Used_CommPort = 0`（无物理端口，纯计算），采样周期 1000 ms
- 自身设备：`THM_DEV_ID = 992`（DeviceModel 950 空调域）
- 驱动来源：`thermal_control_main.c`（`thermal_control/Makefile:17`）
- 设计定位：替代 R 核原有的 13 级 if-else 判定（`thermal_control_main.h:12-14`）

**数据输入**：
- 核间聚合：`THM_RPMSG_DEV_ID = 500`（`thermal_control_main.h:159`）
- 电池簇基准：`THM_BATT_DEV_BASE = 501`（`:332`）
- 关键量：`THM_SRC_T_CELL_MAX` / `THM_SRC_T_CELL_MIN` / `THM_SRC_HV_STATUS`（`thermal_control_main.c:164-245`）

**状态机**（`thermal_control_main.h:33-40`）：`STANDBY` / `COOLING` / `DEEPCOOL` / `HEATING` / `EMERGENCY_COOL` / `SELFCIRC` 六态，带滞回与时间保护。

**输出**：`Push_VarValue` → devId `THM_DST_DEV_ID = 951`（空调采样器），使其经 Modbus RTU 落到机组寄存器。

**参数空间**：
- 公式系数与策略数据：设置量 2408-2431、2443-2451、2452-2493（属 DeviceModel 950）
- 模式开关：`THM_PARAM_AIRCON_CTRL_MODE = 2203`（devId 951，ENUM 0=Auto / 1=Manual）

#### Scenario: 策略计算与下发

- **WHEN** 采样周期到达（1000 ms）
- **THEN** 从 DPR 读取电池温度极值、高压状态与核间聚合数据
- **AND** 代入 `T_decision` 公式，经滞回状态机与时间保护得到目标状态
- **AND** 以 `Push_VarValue(SETTYPE_INTERN_SETT, …, devId=951, …)` 下发设定值

#### Scenario: Web 修改系数即时生效

- **WHEN** 通过 Web 修改 2408-2493 区间的公式系数或策略参数
- **THEN** 下一次 `Sample()` 周期重新读取设置量并采用新值
- **AND** 无需重启进程

#### Scenario: 手动模式屏蔽自动策略

- **WHEN** devId 951 的 2203 被置为 1（Manual）
- **THEN** 自动策略不覆盖人工设定值

