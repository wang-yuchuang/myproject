## MODIFIED Requirements

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
