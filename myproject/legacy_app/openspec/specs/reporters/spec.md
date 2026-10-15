# Spec: reporters — 上报模块

**Subsystem**: reporters
**Version**: 0.3
**Created**: 2026-07-01
**Last Updated**: 2026-07-14

---

## Purpose

业务数据上报模块，将从 DPR 处理后的数据通过多种协议上报到云端、运维平台或上位机。通过统一加载器 `reporters_loader.c` 初始化和管理。

架构位置：samplers → DPR → **reporters** → 外部系统

**统一接口规范**（`reporters_base.h:27`）：每个上报器 MUST 导出唯一的 `DLLExport BOOL Initialize(HANDLE pReporterThis)` 函数。与 samplers（4 个导出函数）不同，reporter 只有 `Initialize` 一个导出点——后续的数据上报由各 reporter 内部线程/回调驱动，通过 DPR 的事件机制（`Event_Handler` 回调）或 DAI 接口获取数据。

**事件驱动模型**：所有 reporter 在 `Initialize` 中完成以下初始化步骤（以 `es_mqtt_reporter` 为典型）：
1. 分配私有数据堆 `pReporter->pDataHeap = NEW(...)`（零初始化）
2. 创建互斥锁（`Mutex_Create`）和消息队列（`Queue_Create`）
3. 创建工作线程（`RunThread_Create("name", _ThreadEntry_*, pReporter, &bExit)`）
4. 向 DPR 注册事件处理器（`pDAI->Regsiter_EvHandler("name", this, Event_Handler, evTag, msgTag, ...)`），订阅 `EVENT_TYPE_ALARM` / `EVENT_TYPE_CFG_CHANGED` / `EVENT_TYPE_FIRMWARE_UPDATE` 等事件

**数据序列化**：`reporters_base.h` 引入 `cJSON`（`../cJSON.h`）作为统一的 JSON 构建工具，`CreateJSON_C155` / `CreateJSON_C157` / `CreateJSON_C202` / `CreateJSON_C204` / `CreateJSON_C206` / `CreateJSON_SEMS_C103` / `CreateJSON_SEMS_C110` / `CreateJSON_SEMS_C202` 等辅助函数服务于不同云平台的协议格式。

**公共辅助函数**（`reporters_pubfun.c`）：
- `Make_TimeString()` — ISO 8601 / UTC 时间字符串格式化（源码使用 `gmtime_r()`，输出纯 UTC 格式 `YYYY-MM-DDTHH:MM:SSZ`）
- `GetLCCfg_Reporter()` — 从 DAI 读取负载配置（LCCfg：系统类型 / 应用场景 / 客户端 LC 索引 / ESC 数量 / 支路数 / 柜表等）
- `Reporter_GetCommPort_ById()` — 按 ID 获取通信端口实例

---
## Requirements
### Requirement: R1 统一上报器加载机制

系统 MUST 在启动时通过 `Init_Load_AllReporters()` 统一初始化所有配置的上报器，由 `main.c` 第 6 步（`Init_Load_AllReporters`）调用。

**加载逻辑**（`reporters_loader.c:36`）：
- 遍历 `pCharger->reporters.p[]` 数组（`nTotal` 个），每个 reporter 通过 `LoadDynamicLibrary(MakeFullFileName(..., "reporter", szReporterDriver), ...)` 加载 `.so`
- dlsym 解析 **1 个符号**：`"Initialize"`（赋值到 `pReporter->Init`），符号名定义于 `reporter_model.h:7` (`REPORTER_INIT`)
- 每个 reporter 加载后注入 `pReporter->pDAI = &pCharger->dai`

**永久跳过规则**：
- `nReporterID == 1`：Qt GUI reporter，无条件跳过（注释 `// Don't Load Qt GUI reporter_gui`，原 `#ifdef IMMU3` 现已去条件化）

**平台类型条件加载**（`nPlatformType` = DAI `DAITYPE_SETT_PARAMVALUE_RW`, param 1/10）：

| nPlatformType | 含义 | 跳过的 Reporter ID |
|:---:|------|------|
| 0 | Std（标准 Web 上报） | 6(MQTT) / 7(IEC104) / 8(IEC61850) / 9(IEC101) |
| 1, 2 | MQTT | 7 / 8 / 9 |
| 3 | IEC104 | 6 / 8 / 9 |
| 4 | IEC61850 | 6 / 7 / 9 |
| 5 | IEC101 | 6 / 7 / 8 |
| 6 | 未安装 | 6 / 7 / 8 / 9 |

> 同一时间只启用一种云平台协议族，互斥关系硬编码在 loader 中。

**独立开关条件加载**（各 reporter 单独的 DAI enable/disable 参数）：

| Reporter ID | 控制参数 (devID/paramID) | 为 0 时跳过 |
|:---:|------|:---:|
| 4 (Web API) | 1/47 | 仅当 `nPlatformType != 0` 时跳过 |
| 10 (ify cloud) | 1/16 | ✓ |
| 12 (ess_maintenance) | 1/50 | ✓ |
| 3 (maintain_backend/mbe-mqtt) | 1/51 | ✓ |
| 13 (slaveEMS) | 1/42 | ✓ |
| 2 (es_iocpp) | 1/57 | ⚠️ 使用 `valGet.val.ulValue`（已知不一致，见下方说明） |
> **已知不一致**：上表中所有控制开关均通过 `valGet.val.emValue`（枚举值）判断，唯独 es_iocpp 的 param 1/57 使用 `valGet.val.ulValue`（无符号长整型）判断（`reporters_loader.c:225`）。原因未确认，疑似历史遗留。

#### Scenario: 上报器加载与初始化
- **WHEN** 系统启动执行 `Init_Load_AllReporters()`
- **THEN** 每个配置的上报器 `.so` 被加载（`LoadReporterLib`），符号 `Initialize` 通过 dlsym 解析
- **AND** `pReporter->Init((HANDLE)pReporter)` 被调用完成各自初始化
- **AND** 与当前 `nPlatformType` 互斥的上报器被跳过（TRACE 记录 Reporter Type 和 Name）

#### Scenario: 加载失败处理
- **WHEN** 某个上报器的 `.so` 加载失败（`LoadReporterLib` 返回 FALSE）或 `Initialize` 返回 FALSE
- **THEN** `Init_Load_AllReporters` **立即返回 FALSE**，中断后续所有上报器的加载
- **AND** 错误通过 `LogOut(..., LOG_TYPE_ERROR, ...)` 记录到日志
- **AND** main 进程根据返回值决定是否继续启动

#### Scenario: 条件跳过日志
- **WHEN** 上报器因 `nPlatformType` 互斥或独立开关被跳过
- **THEN** 输出 `TRACE` 日志记录跳过原因（含 Reporter ID 和名称）
- **AND** 该上报器因类型互斥被跳过，不加载、不初始化，`continue` 到下一个

#### Scenario: Qt GUI reporter 永久跳过
- **WHEN** 遍历到 `nReporterID == 1` 的上报器
- **THEN** 无条件 `continue`（不加载 Qt GUI reporter）
- **AND** 输出 `TRACEI("reporters_loader:Init_Load_AllReporters continue not load nReporterID:1 ...")` 日志

### Requirement: R2 MQTT 上报

系统 MUST 支持通过 MQTT 协议上报数据到云平台（Reporter ID = 6）。

**实现**：`es_mqtt_reporter/` 内置 **paho-mqtt** C 客户端源码（`paho_mqtt_src/` + `3rd_party/libpahomqtt/`），核心文件：
- `es_mqtt_main.c`（~1959 行）— Initialize + Event_Handler + 工作线程
- `es_mqtt_msghandler.c` — 消息构建与发送（`stMsgInfoList[C_xxx].pfnMsgHandler`）
- `es_mqtt_msgparser.c` — 下行消息解析
- `GH_DigitalEnergy_Pub.c` — 国鸿数字能源云对接（`GH_MQTT_TYPE` 枚举：ARR/CLU/EMU/PCS/FIRE/LIQCOOL/DRIER/METER/CELL 等设备类型 + 状态类型 + 命令类型）

**TLS 双向认证**：证书路径 `/app/config/cacert.pem` / `client.pem` / `client.key`

**消息队列**：`pMqtt->hqMsg2Sent`（`Queue_Create(MAX_NORMAL_MSG_QUEUE_SIZE, sizeof(MQTT_CALL_MSG), 0)`），工作线程从队列消费并发布

**唯一 ID 生成**：`Make_UniqID_Mqtt()` 基于递增计数器（起始 `MQTT_UNIQID_START = 25689566`），溢出回绕

#### Scenario: MQTT 连接与发布
- **WHEN** `Initialize` 创建工作线程 `_ThreadEntry_MqttComm`
- **THEN** 线程连接 MQTT Broker（支持 TLS 双向认证）
- **AND** 事件触发时由 `pfnMsgHandler` 构建 JSON 消息，入队 `hqMsg2Sent`
- **AND** 工作线程出队并调用 paho-mqtt API 发布

#### Scenario: 事件订阅
- **WHEN** `Initialize` 注册以下事件处理器
- **THEN** `EVENT_TYPE_ALARM`（BEGIN/END/RESTORE）触发告警上报
- **AND** `EVENT_TYPE_FIRMWARE_UPDATE`（BEGIN/END/STATUS_CHANGED）触发固件升级状态上报
- **AND** `EVENT_TYPE_DIAGNOSTIC_UPLOAD` 触发诊断数据上报
- **AND** `EVENT_TYPE_CFG_CHANGED` 触发配置变更上报
- **AND** `EVENT_TYPE_SYSTEM_EXIT`（END）触发退出清理
- **AND** `EVENT_TYPE_ABILITY_CHANGE`（BEGIN/END，handler no-op — 源码注释 `//do not treat this, use status change event instead`）

#### Scenario: 国鸿数字能源云对接
- **WHEN** 目标云平台为国鸿数字能源
- **THEN** `GH_DigitalEnergy_Pub.c` 按 `GH_MQTT_TYPE` 枚举分类上报（设备状态 / 告警 / 功率命令 / 调度功率 / 功率限制）
- **AND** 支持 `GH_SUBTOPIC_NUM = 3` 个订阅主题
- **AND** 时间戳含北京时间偏移（`BEIJING_TIMEZONE_OFFSET_HOURS = 8*60*60`）

#### Scenario: 消息时间格式
- **WHEN** 构建消息时间戳
- **THEN** `emApplyArea == 0`（Local）使用 `"YYYY-MM-DD HH:MM:SS"` 格式
- **AND** 否则使用 ISO 8601 `"YYYY-MM-DDTHH:MM:SSZ"` 格式（`Make_MqttTimeString`）

### Requirement: R3 英飞源云上报

系统 MUST 支持通过英飞源（Infypower）自定义协议上报数据到英飞源云平台（Reporter ID = 10）。

**实现**：`ify_cloud_reporter/`，核心文件：
- `es_ify_main.c` — Initialize + Event_Handler + 工作线程
- `es_ify_msghandler.c` — 消息构建
- `es_ify_msgparser.c` — 下行命令解析

**控制开关**：DAI param 1/16（为 0 时跳过加载）

#### Scenario: 云平台连接与数据上报
- **WHEN** `Initialize` 创建工作线程建立到英飞源云的连接
- **THEN** 按配置周期上报运行数据（JSON 格式，符合英飞源协议规范）
- **AND** 工作线程处理连接保活和断线重连

#### Scenario: 下行命令处理
- **WHEN** 云平台下发控制命令
- **THEN** `es_ify_msgparser.c` 解析命令帧
- **AND** 通过 `es_ify_msghandler.c` 分发处理并执行

### Requirement: R4 Modbus 上报

系统 MUST 支持通过 Modbus TCP 协议对外提供数据（作为 Modbus TCP Server 响应外部查询）。

**实现**：`modbus_reporter/`，核心文件：
- `modbus_main.c`（~630 行）— Initialize + Event_Handler + 工作线程
- `handle_frame.c` — Modbus 请求帧处理
- `modbus_tcp_datamap.c` — 寄存器地址映射（业务数据 → Modbus 寄存器）
- 依赖 `modbus.h`（libmodbus 封装）

**输出**：`reporter_modbustcp.so`

#### Scenario: Modbus TCP Server 响应
- **WHEN** 外部 Modbus TCP 客户端发送读寄存器请求
- **THEN** `handle_frame.c` 解析请求帧
- **AND** `modbus_tcp_datamap.c` 从 DPR 运行数据中查找对应寄存器值
- **AND** 正确返回 Modbus 响应帧

#### Scenario: 事件驱动数据刷新
- **WHEN** DPR 推送 `EVENT_TYPE_ALARM` / `EVENT_TYPE_CFG_CHANGED` / `EVENT_TYPE_SYSTEM_EXIT` 等事件
- **THEN** Event_Handler 更新内部数据缓存
- **AND** 后续 Modbus 查询返回最新数据

#### Scenario: 通信状态跟踪
- **WHEN** Modbus 通信发生错误
- **THEN** `pReporter->nCommErrors` 递增
- **AND** `pReporter->bCommStatus` 反映当前通信状态

### Requirement: R5 Web 服务

系统 MUST 提供内置的 Web 服务和 REST API，供本地浏览器查看和管理设备（Reporter ID = 4）。

**实现**：`web_reporter/` 是个复合模块，包含 **4 个子目录**，各自独立编译为 `.so` 或可执行文件：

| 子目录 | 职责 | 输出 |
|--------|------|------|
| `web_server/` | HTTP 服务器（独立进程），含 `server_main.conf` / `server.pem`（TLS）/ `start_web_server` 启动脚本 | 可执行文件 |
| `back_api/` | 后端 FastCGI API（`fcgi_defs.h` / `fcgi_lib/`），含 `api_per_page/*.c` 页面级 API | FastCGI 程序 |
| `front/` | 前端静态资源（HTML/JS/CSS） | 静态文件 |
| `web_api_reporter/` | Web API 上报器（reporter 接口，`reporter_webapi_main.c`） | `reporter_webapi.so` |

**FastCGI 后端 API**（`back_api/api_per_page/`）覆盖：PCS 运行信息/参数、BMS 参数设置、告警查询（实时/历史）、固件管理、日志查询、充电记录、手动控制、数据备份恢复、用户管理、快速设置等（~20+ 个 API 页面文件）。

**Web API Reporter**（`web_api_reporter/reporter_webapi_main.c`）：
- 通过 `COM_WEB_IPC` 通信端口与 back_api 交互（`pUsedCommPort->Open("COM_WEB_IPC", WEB_IPC_KEY, COMM_SERVER_MODE, ...)`）
- 定义 `WEB_REQTYPE_*` 请求类型枚举，通过 `DEF_WEBREQPARSER` 宏注册解析器
- 后端直接读取 `/app/config/System.cfg`（SQLite）和 `/data/nccu.data` 获取配置与运行数据

**控制开关**：DAI param 1/47（为 0 且 `nPlatformType != 0` 时跳过加载）

#### Scenario: 浏览器请求处理
- **WHEN** 浏览器请求 Web 页面或 REST API
- **THEN** `web_server` HTTP 服务器接收请求并转发到 FastCGI 后端
- **AND** `back_api/fcgi_main.c` 处理请求，路由到对应 `api_per_page/*.c` 处理函数
- **AND** 返回 HTML 页面或 JSON 数据

#### Scenario: Web API Reporter IPC 通信
- **WHEN** back_api 需要与主进程交换数据
- **THEN** 通过 `COM_WEB_IPC` 通信端口（`WEB_IPC_KEY`）与 `web_api_reporter` 交互
- **AND** `web_api_reporter` 从 DPR 获取运行数据并返回
- **AND** 支持版本信息 / 设备信息 / 参数信息 / 告警信息 / 参数值 / 回读值 / 参数控制设置等请求类型

#### Scenario: 会话与用户管理
- **WHEN** 用户通过浏览器登录 Web 界面
- **THEN** `session_mgmt.c` 管理会话（`MAX_ALLOW_WEB_USER = 5` 并发用户）
- **AND** 登录频率限制（`ADDSESS_LOGIN_TOO_FREQUENT`）
- **AND** `api_login.c` / `api_user_mgmt.c` 处理认证和用户 CRUD

### Requirement: R6 IEC 60870-5-104 协议上报

系统 MUST 支持通过 IEC 60870-5-104 协议（TCP/IP）上报电力远动数据（Reporter ID = 7）。

> **编译状态**：源码存在于 `iec_104_reporter/`，但当前在 `src/Makefile` 中被注释（`# DIRS += moduals/reporters/iec_104_reporter`），**未纳入常规编译**。

**实现**：`iec_104_reporter/`，依赖 `3rd_party/libiec104/`（`cs104_slave.h` / `cs104_connection.h` / `buffer_frame.h` / `hal_thread.h` / `hal_time.h`），核心文件：
- `iec_104_main.c`（~188 行）— Initialize + Event_Handler
- `iec_104_handler.c` — 协议处理
- `iec_104_sigmap.c` — 信号映射（`IEC104_CONFIG_SIGMAP`：信号索引 → IOA / 品质描述符 / 数据类型 / 缩放 / 值指针）
- `iec_104_simple_client.c` — 简单客户端工具

**数据类型**（`OBJVALUE_DATATYPE`）：INT / BOOL / FLOAT_DOT1（1 位小数）/ String32

**EMS 扩展**：支持 EMS 分时段参数（`EMS_SECTOR_PARAM`，最多 `MAX_EMS_SECTOR = 12` 个时段：起止时间 / 停止 SOC / 功率限制等）

#### Scenario: IEC 104 连接与数据传输
- **WHEN** 主站通过 TCP 连接（启用时）
- **THEN** `cs104_slave` 建立连接，连接状态经过 `STAT_Connect_Opened` → `STAT_Activated` 流转
- **AND** 按 IOA（Information Object Address）映射上报遥测/遥信数据

#### Scenario: 信号映射
- **WHEN** DPR 数据需要通过 IEC 104 上报
- **THEN** `iec_104_sigmap.c` 将内部信号索引映射到 IOA + 数据类型 + 缩放因子
- **AND** `QualityDescriptorP` 标注数据品质（有效/无效/被替代等）

### Requirement: R7 IEC 60870-5-101 协议上报

系统 MUST 支持通过 IEC 60870-5-101 协议（串口 RTU）上报电力远动数据（Reporter ID = 9）。

> **编译状态**：源码存在于 `iec_101_reporter/`，但当前在 `src/Makefile` 中被注释（`# DIRS += moduals/reporters/iec_101_reporter`），**未纳入常规编译**。

**实现**：`iec_101_reporter/`，核心文件：
- `iec_101_main.c` — Initialize + Event_Handler
- `iec_101_handler.c` — 协议帧处理（串口通信）

与 IEC 104 共享相同的事件处理模式（`EVENT_TYPE_ALARM` / `EVENT_TYPE_FIRMWARE_UPDATE` / `EVENT_TYPE_DIAGNOSTIC_UPLOAD` / `EVENT_TYPE_SYSTEM_EXIT`）

#### Scenario: IEC 101 串口通信
- **WHEN** 主站通过串口轮询（启用时）
- **THEN** 按 IEC 101 帧格式响应遥测/遥信/电度量数据
- **AND** 支持事件触发上报（SOE 等）

### Requirement: R8 IEC 61850 协议上报

系统 MUST 支持通过 IEC 61850 协议上报变电站自动化数据（Reporter ID = 8）。

> **编译状态**：源码存在于 `iec_61850_reporter/`（`iec_61850_main.c` + `iec_61850.h`），但当前在 `src/Makefile` 中被注释（`# DIRS += moduals/reporters/iec_61850_reporter`），**未纳入常规编译**。

**实现**：`iec_61850_reporter/iec_61850_main.c` — Initialize + 事件处理

#### Scenario: IEC 61850 数据模型服务
- **WHEN** IEC 61850 客户端请求（启用时）
- **THEN** 提供 LD/ LN/ 数据对象模型
- **AND** 支持报告控制块（RCB）主动上报数据变化

### Requirement: R9 OCPP 协议上报

系统 MUST 支持通过 OCPP（Open Charge Point Protocol）协议对接 EV 充电桩后台管理系统。

> **编译状态**：源码存在于 `occp_reporter/`，但**不在 `src/Makefile` 的 DIRS 列表中**（从未纳入构建），无 `.o` / `.so` 产物。这是早期 EV 充电桩遗留代码。

**实现**：`occp_reporter/`，核心文件：
- `ocpp.c` — Initialize + Event_Handler
- `ocpp_msghandler.c` — OCPP 消息构建（`stMsgInfoList[Authorize].pfnMsgHandler` 等）
- `ocpp_msgparser.c` — 下行消息解析
- `ocpp.h` — OCPP 连接状态（`Ready` 等）/ 数据结构

**事件类型**：`EVENT_TYPE_USER_AUTHORIZE` / `EVENT_TYPE_CHARGING`（充电桩专用事件，区别于储能场景）

#### Scenario: OCPP 授权流程（源码存在，未编译）
- **WHEN** 收到 `EVENT_TYPE_USER_AUTHORIZE` 事件
- **THEN** `Event_Handler` 提取 `szIdTocken`，构建 Authorize 请求发送到中央系统
- **AND** `bAuthorizeInProcess` 标志防止并发授权

#### Scenario: 充电事务务上报（源码存在，未编译）
- **WHEN** 收到 `EVENT_TYPE_CHARGING`（BEGIN/END）
- **THEN** 按 `CHG_TYPE_GUN_INCHARGE` 处理充电枪状态
- **AND** `emHandlingConnector` 跟踪当前处理的连接器编号

### Requirement: R10 运维后台上报

系统 MUST 支持上报数据到储能运维管理平台，包含两个独立子模块。

**子模块 A — ESS 运维上报**（`ess_maintenance_reporter/`）：
- `ess_maintenance_main.c` — Initialize + Event_Handler
- `ess_maintenance_handler.c` — 消息处理
- 依赖 `web_reporter/back_api/web_data_model.h`（复用 Web 数据模型）
- 输出：`reporter_ess_maintenance.so`

**子模块 B — 维护后端 MQTT 通道**（`maintain_backend_reporter/mbe-mqtt/`，Reporter ID = 3）：
- 通过 MQTT 协议对接远程维护通道
- 控制开关：DAI param 1/51（为 0 时跳过加载）
- 另有 `mbe-websocket-old/` 已废弃（注释 `#DIRS += ...mbe-websocket-old`，标注 `phased-out`）

#### Scenario: ESS 运维数据上报
- **WHEN** 按配置周期上报运维数据
- **THEN** `ess_maintenance_main.c` 从 DPR 获取运行数据
- **AND** 复用 `web_data_model.h` 定义的数据格式
- **AND** 数据正确送达运维平台

#### Scenario: 维护后端 MQTT 通道
- **WHEN** DAI param 1/51 为 1（启用）
- **THEN** `mbe-mqtt` 建立 MQTT 连接到维护后端
- **AND** 支持远程诊断、日志提取、固件升级等维护指令

### Requirement: R11 从站 EMS 上报

系统 MUST 支持作为从站上报数据到主站能量管理系统（Reporter ID = 13）。

**实现**：`slaveEMS_reporter/`，控制开关：DAI param 1/42（为 0 时跳过加载）

#### Scenario: 从站数据同步
- **WHEN** 主站 EMS 请求数据或按周期同步
- **THEN** slaveEMS reporter 返回当前运行状态和参数
- **AND** 支持 EMS 分时段参数上报（充放电策略 / SOC 限制 / 功率限制）

#### Scenario: 主从切换
- **WHEN** 系统配置为从站模式
- **THEN** 仅响应主站命令，不主动调度
- **AND** DAI param 1/42 控制是否启用此 reporter

### Requirement: R12 ES iOCPP 上报

系统 MUST 支持通过 Infypower 私有 iOCPP 协议上报数据（Reporter ID = 2）。

**实现**：`es_iocpp_reporter/`，核心文件：
- `es_iocpp_main.c` — Initialize + Event_Handler + 工作线程
- `es_iocpp_msghandler.c` — 消息构建
- `es_iocpp_msgparser.c` — 下行消息解析
- `es_iocpp.h` — 协议定义
- 输出：`reporter_es_iocpp.so`

**控制开关**：DAI param 1/57（为 0 时跳过加载）

> **注意**：与 `occp_reporter/`（R9，标准 OCPP，未编译）不同，`es_iocpp_reporter` 是 Infypower 定制的 iOCPP 协议实现，已纳入常规编译。

#### Scenario: iOCPP 连接与上报
- **WHEN** `Initialize` 创建工作线程建立到 iOCPP 服务端的连接
- **THEN** 按私有协议格式上报运行数据
- **AND** `es_iocpp_msghandler.c` 构建各类消息（数据/状态/告警）

#### Scenario: 下行命令处理
- **WHEN** iOCPP 服务端下发控制命令
- **THEN** `es_iocpp_msgparser.c` 解析命令帧
- **AND** 分发执行并回复执行结果

---

## Modules

| 模块 | 路径 | Reporter ID | 协议/依赖 | 编译 | 目标系统 |
|------|------|:---:|----------|:---:|---------|
| MQTT | `es_mqtt_reporter/` | 6 | MQTT（内置 paho-mqtt 源码 + TLS） | ✅ | 通用 MQTT 云平台（含国鸿数字能源） |
| 英飞源云 | `ify_cloud_reporter/` | 10 | 英飞源私有协议 | ✅ | 英飞源云 |
| Modbus | `modbus_reporter/` | — | Modbus TCP（libmodbus） | ✅ | 外部 Modbus 客户端 |
| Web 服务 | `web_reporter/` | 4 | HTTP + FastCGI（4 子目录） | ✅ | 本地浏览器 + REST API |
| ESS 运维 | `ess_maintenance_reporter/` | 12 | 自定义协议（依赖 web_data_model.h） | ✅ | 运维管理平台 |
| 维护后端 | `maintain_backend_reporter/mbe-mqtt/` | 3 | MQTT | ✅ | 远程维护通道 |
| Slave EMS | `slaveEMS_reporter/` | 13 | 自定义协议 | ✅ | 从站 EMS |
| ES iOCPP | `es_iocpp_reporter/` | 2 | Infypower iOCPP | ✅ | iOCPP 服务端 |
| IEC 104 | `iec_104_reporter/` | 7 | IEC 60870-5-104（libiec104） | ❌ 注释 | 电力主站 |
| IEC 101 | `iec_101_reporter/` | 9 | IEC 60870-5-101（串口） | ❌ 注释 | 电力主站 |
| IEC 61850 | `iec_61850_reporter/` | 8 | IEC 61850 | ❌ 注释 | 变电站 |
| OCPP | `occp_reporter/` | — | OCPP（EV 充电桩） | ❌ 未纳入 | 充电桩后台 |
| 主站间通信 | `inter_master_reporter/` | — | 自定义 | ❌ 注释 | 主站间通信 |

> ⚠️ **Reporter_ID 真值来源**：`src/config/System.sql:4320-4332`（`Charger_Reporter` 表）。**2026-09-17 修正了本表与 R1 独立开关表中的 11 处错映射** —— 原值系按“Requirement 出现顺序”推定，非查配置所得。
>
> 正确映射：`1=QtGUI`（永久跳过）/ `2=ES-iOCPP` / `3=mbe_new` / `4=WebAPI` / `6=MQTT` / `7=IEC104` / `8=IEC61850` / `9=IEC101` / `10=ES-Ify` / `11=ModbusTCP` / `12=EssMaintenance` / `13=slaveEMS` / `14=ModbusTCP2`。注意 **无 ID 5**。

> **未编译模块说明**：
> - `iec_104_reporter/` / `iec_101_reporter/` / `iec_61850_reporter/`：在 `src/Makefile` 中被注释（`# DIRS += ...`），源码完整但未纳入常规构建
> - `occp_reporter/`：EV 充电桩遗留代码，不在 Makefile DIRS 列表中，无编译产物
> - `inter_master_reporter/`：在 `src/Makefile` 中被注释，主站间通信模块
> - `modbustcp2_reporter/`：**死代码**，不在 Makefile 中，`TEST_EXEC` 为空，是 modbus_reporter 的早期替代实现
> - `maintain_backend_reporter/mbe-websocket-old/`：已废弃（`phased-out`），在 Makefile 中被注释

---

## Test Definition

> **现状说明**：所有 reporter 的 Makefile 中 `TEST_EXEC =`（空），全模块零测试代码。以下表格记录的是各 R 达到白盒可测的难点和策略。

| Requirement | 白盒测试 | 方式 |
|------------|:--------:|------|
| R1 统一加载 | ⚠️ 中等 | mock 各上报器 Initialize 符号，验证 nPlatformType 跳过逻辑 + 独立开关跳过逻辑 |
| R2 MQTT | ⚠️ 中等 | mock paho-mqtt 库 + COMMPORT，验证消息构建和事件处理 |
| R3 英飞源云 | ⚠️ 中等 | mock 通信端口，验证消息格式和重连逻辑 |
| R4 Modbus | ⚠️ 中等 | mock libmodbus，验证寄存器映射和帧处理 |
| R5 Web 服务 | ❌ 难 | FastCGI 后端需完整 Web 服务器环境，IPC 通道需进程对偶测试 |
| R6 IEC 104 | ❌ 难 | 需 libiec104 + 电力主站模拟器（且当前未编译） |
| R7 IEC 101 | ❌ 难 | 需串口环境 + 电力主站模拟器（且当前未编译） |
| R8 IEC 61850 | ❌ 难 | 需 IEC 61850 客户端模拟器（且当前未编译） |
| R9 OCPP | ❌ 难 | 需 OCPP 中央系统模拟器（且当前未编译） |
| R10 运维后台 | ⚠️ 中等 | mock MQTT + 通信端口，验证数据格式 |
| R11 Slave EMS | ⚠️ 中等 | mock 主站请求，验证数据同步 |
| R12 ES iOCPP | ⚠️ 中等 | mock iOCPP 服务端，验证消息编解码 |

---

## Key Files

> 源文件在 `src/moduals/reporters/`，公共接口在 `reporters_base.h` / `reporters_pubfun.c`，数据模型在 `src/include/data_model/reporter_model.h`。

| 文件 | 对应 R | 说明 |
|------|:------:|------|
| reporters_loader.c | R1 | 统一加载器（`Init_Load_AllReporters`，nPlatformType 条件 + 独立开关条件加载） |
| reporters_loader.h | R1 | 加载器头文件（`Init_Load_AllReporters` 声明） |
| reporters_base.h | R1-R12 | 公共接口（`DLLExport Initialize` + cJSON 辅助函数声明 + 常量定义） |
| reporters_pubfun.c | R1-R12 | 公共辅助函数（`Make_TimeString` / `GetLCCfg_Reporter` / `Reporter_GetCommPort_ById`） |
| reporter_model.h | R1 | 数据模型（`REPORTER_INFO` / `REPORTER` 结构体 + `REPORTER_INIT` 符号名） |
| es_mqtt_reporter/es_mqtt_main.c | R2 | MQTT 上报 Initialize + Event_Handler + 工作线程（~1959 行） |
| es_mqtt_reporter/es_mqtt_msghandler.c | R2 | MQTT 消息构建（`stMsgInfoList[].pfnMsgHandler`） |
| es_mqtt_reporter/GH_DigitalEnergy_Pub.c | R2 | 国鸿数字能源云对接 |
| es_mqtt_reporter/paho_mqtt_src/ | R2 | 内置 paho-mqtt C 客户端源码 |
| ify_cloud_reporter/es_ify_main.c | R3 | 英飞源云上报主逻辑 |
| modbus_reporter/modbus_main.c | R4 | Modbus TCP Server 主逻辑（~630 行） |
| modbus_reporter/modbus_tcp_datamap.c | R4 | 寄存器地址映射 |
| web_reporter/web_api_reporter/reporter_webapi_main.c | R5 | Web API reporter 接口（IPC 通信） |
| web_reporter/back_api/fcgi_main.c | R5 | FastCGI 后端入口 |
| web_reporter/back_api/fcgi_defs.h | R5 | FastCGI 定义（会话管理 / 用户限制 / 国际化） |
| web_reporter/back_api/api_per_page/ | R5 | 页面级 API（~20+ 文件：PCS/BMS/告警/固件/日志/用户管理） |
| web_reporter/web_server/ | R5 | HTTP 服务器（独立进程 + TLS + 启动脚本） |
| iec_104_reporter/iec_104_main.c | R6 | IEC 104 上报（未编译） |
| iec_104_reporter/iec_104_sigmap.c | R6 | 信号→IOA 映射 |
| iec_101_reporter/iec_101_main.c | R7 | IEC 101 上报（未编译） |
| iec_61850_reporter/iec_61850_main.c | R8 | IEC 61850 上报（未编译） |
| occp_reporter/ocpp.c | R9 | OCPP 充电桩上报（未编译，遗留） |
| ess_maintenance_reporter/ess_maintenance_main.c | R10 | ESS 运维上报 |
| maintain_backend_reporter/mbe-mqtt/ | R10 | 维护后端 MQTT 通道 |
| slaveEMS_reporter/ | R11 | 从站 EMS 上报 |
| es_iocpp_reporter/es_iocpp_main.c | R12 | ES iOCPP 上报 |
| modbustcp2_reporter/ | (死代码) | modbus_reporter 早期替代，未纳入构建 |
| inter_master_reporter/ | (未编译) | 主站间通信，Makefile 中注释 |

---

## Known Tech Debts

1. **加载失败中断全部** — `Init_Load_AllReporters` 中任一 reporter 加载或初始化失败时 `return FALSE`，中断后续所有 reporter 的加载，缺乏容错隔离（与 samplers loader 不同）
2. **`modbustcp2_reporter/` 死代码** — 3 个源文件（`modbustcp2_main.c` / `handle_frame.c` / `modbustcp2_datamap.c`）+ 1 个头文件（`modbus.h`），不在任何 Makefile DIRS 中，是 `modbus_reporter/` 的早期实现
3. **`occp_reporter/` 遗留代码** — EV 充电桩 OCPP 实现，不在 Makefile 中，与储能场景无关，应确认是否清理
4. **IEC 协议族未编译** — `iec_104` / `iec_101` / `iec_61850` 三个 reporter 源码完整但在 `src/Makefile` 中被注释，需确认是否为待激活功能或应清理
5. **平台互斥逻辑硬编码** — `nPlatformType` 与 Reporter ID 的互斥关系以 if-else 链硬编码在 loader 中（6 个 else-if 分支），缺乏配置化，新增协议需改代码
6. **`mbe-websocket-old` 废弃未清理** — `maintain_backend_reporter/mbe-websocket-old/` 已标注 `phased-out`，仍存在于代码库
7. **零测试代码** — 所有 reporter 的 Makefile `TEST_EXEC =`（空），全模块无单元测试
8. **REPORTER 结构体命名混淆** — `hSamplerLib` 字段名沿用了 sampler 的命名（reporter 复用了 sampler 的加载框架），实际加载的是 reporter `.so`
