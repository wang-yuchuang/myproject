# 上报器协议与开关契约

> **性质**：接口契约
> **需求权威源**：`openspec/specs/reporters/spec.md`（R1-R12）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`97cac555`（最后核对时的代码版本）
> **状态**：🟠 部分

本页是协议与开关的对照契约：每个上报器讲什么协议、受哪个开关控制。加载机制叙述见 [数据上报](../20-features/data-reporting.md)，数据读取面见 [dai.md](dai.md)。

## 上报器 ↔ 协议 ↔ 开关

控制开关均为 DAI `DAITYPE_SETT_PARAMVALUE_RW` 参数，格式 devID/paramID，读 0 表示禁用。编译状态以 `src/Makefile:60-74` 为准。

| 上报器（.so） | 协议/去向 | 类型/角色 | 控制开关（DAI param） | 编译 | 依据 |
|---|---|---|---|---|---|
| es_iocpp_reporter（ID 2） | Infypower iOCPP | iOCPP 服务端对接 | 1/47（=0 且 nPlatformType≠0 时跳过） | ✅ | spec R12；`reporters_loader.c:178-191` |
| mbe-mqtt（ID 3，maintain_backend_reporter） | MQTT | 远程维护通道 | 无独立开关 | ✅ | spec R10；loader 无该判定 |
| web_api_reporter（ID 4，web_reporter 复合模块） | HTTP + FastCGI | 本地浏览器 + REST API | 无独立开关 | ✅ | spec R5；loader 无该判定 |
| es_mqtt_reporter（ID 6） | MQTT + TLS（内置 paho-mqtt） | 云平台上报 | 无独立开关（仅受平台互斥） | ✅ | spec R2；`reporters_loader.c:84-104` |
| ify_cloud_reporter（ID 10） | 英飞源私有协议 | 英飞源云 | 1/16 | ✅ | spec R3；`reporters_loader.c:166-175` |
| modbus_reporter（ID 11） | Modbus TCP（libmodbus） | 外部 Modbus 客户端 | 1/50 | ✅ | spec R4；`reporters_loader.c:193-201` |
| ess_maintenance_reporter（ID 12） | 自定义协议 | ESS 运维平台 | 1/51 | ✅ | spec R10；`reporters_loader.c:203-211` |
| slaveEMS_reporter（ID 13） | 自定义协议 | 从站 EMS | 1/42 | ✅ | spec R11；`reporters_loader.c:213-221` |
| iec_104_reporter（ID 7） | IEC 60870-5-104（libiec104） | 电力主站 | 参与平台互斥 | ✗ 未编译（`src/Makefile:58` 注释） | spec R6 |
| iec_61850_reporter（ID 8） | IEC 61850 | 变电站 | 参与平台互斥 | ✗ 未编译（`src/Makefile:57` 注释） | spec R8 |
| iec_101_reporter（ID 9） | IEC 60870-5-101（串口） | 电力主站 | 参与平台互斥 | ✗ 未编译（`src/Makefile:59` 注释） | spec R7 |
| modbustcp2_reporter（ID 14） | Modbus TCP | modbus_reporter 的早期替代（死代码） | 1/57（`ulValue` 判定） | ✗ 未编译（不在 DIRS） | spec R1 开关表；`reporters_loader.c:223-231` |
| occp_reporter | OCPP | EV 充电桩遗留 | 无 | ✗ 未编译（不在 DIRS） | spec R9 |
| inter_master_reporter | 自定义协议 | 主站间通信 | 无 | ✗ 未编译（`src/Makefile:56` 注释） | spec 无对应 R |
| Qt GUI reporter（ID 1） | 本地 GUI | 已去条件化，永久跳过 | 无 | ✗ 不加载 | spec R1；`reporters_loader.c:51-56` |

web_reporter 复合模块含 4 个子目录（`web_server` 可执行 / `back_api` FastCGI / `front` 静态资源 / `web_api_reporter` .so），上表按 reporter 接口角色只列 `web_api_reporter` 一行。

## 平台类型互斥

`nPlatformType` 读自 DAI 参数 1/10（`reporters_loader.c:81-83`），每个 reporter 行遍历时重复读取。它决定云平台协议族互斥，同一时间只启用一种，关系硬编码在 loader 的 if-else 链中（`reporters_loader.c:84-162`；spec R1）：

| nPlatformType | 含义 | 跳过的 Reporter ID |
|:---:|---|---|
| 0 | Std（Web 上报） | 6 / 7 / 8 / 9 |
| 1, 2 | MQTT | 7 / 8 / 9 |
| 3 | IEC104 | 6 / 8 / 9 |
| 4 | IEC61850 | 6 / 7 / 9 |
| 5 | IEC101 | 6 / 7 / 8 |
| 6 | 未安装 | 6 / 7 / 8 / 9 |

注意 1/47 开关与互斥的交互：`nPlatformType == 0`（Std）时 es_iocpp 即使开关为 0 也不跳过，只有非 Std 平台才尊重该开关（`reporters_loader.c:185-189`）。

## 各上报器开关

以下配对已逐行核对 loader 代码：

- **1/16 → ID 10**（ify_cloud_reporter），`emValue == 0` 跳过（`reporters_loader.c:166-175`）
- **1/47 → ID 2**（es_iocpp_reporter），`emValue == 0` 且 `nPlatformType != 0` 时跳过（`:178-191`）
- **1/50 → ID 11**（modbus_reporter），`emValue == 0` 跳过（`:193-201`）
- **1/51 → ID 12**（ess_maintenance_reporter），`emValue == 0` 跳过（`:203-211`）
- **1/42 → ID 13**（slaveEMS_reporter），`emValue == 0` 跳过（`:213-221`）
- **1/57 → ID 14**（modbustcp2_reporter），`ulValue == 0` 跳过（`:223-231`），loader 中唯一的非枚举判定；目标模块当前未编译，此开关暂无实际效果
- ID 3（mbe-mqtt）与 ID 4（web_api_reporter）在当前代码中没有独立开关，只受平台互斥约束

⚠️ 待核实：spec R1 独立开关表及 R5/R10/R12 行文把 1/47 记为 WebAPI(ID4)、1/51 记为 mbe(ID3)、1/50 记为 EssMaintenance(ID12)、1/57 记为 iOCPP(ID2)，与 loader 硬编码不一致；spec 待修正，本页以代码为准（详见 `docs/20-features/data-reporting.md` 互斥与开关一节）。

## 事件与读通道

- 事件订阅：reporter 在 `Initialize` 中经 `pDAI->Regsiter_EvHandler(...)` 订阅 `EVENT_TYPE_ALARM` / `EVENT_TYPE_CFG_CHANGED` / `EVENT_TYPE_FIRMWARE_UPDATE`，此后上报由事件回调入队、工作线程消费发送（spec reporters Purpose；es_mqtt 为典型实现，spec R1）。
- 告警持久化：`EVENT_TYPE_ALARM` 由 DPR 侧写入 SQLite `Data_Alarms` 表（spec data-processor R2）。
- 数据读取：reporter 不直接触达 DPR，统一经注入的 `pReporter->pDAI`（`reporters_loader.c:48`）：`Get/Set` 转发 DPR 函数指针，`Query` 直读运行数据库。接口细节见 [dai.md](dai.md)；跨模块访问语义见 spec data-processor R6。

## 代码位置

- 加载器入口：`src/moduals/reporters/reporters_loader.c:36`（`Init_Load_AllReporters`）
- pDAI 注入：`reporters_loader.c:48`；Qt GUI 永久跳过：`:51-56`
- 平台互斥链：`reporters_loader.c:80-162`；独立开关判定：`:166-231`
- 失败即中断：`reporters_loader.c:235-249`（任一 reporter 加载或初始化失败，立即返回 FALSE）
- 导出符号 `Initialize`：`src/moduals/reporters/reporters_base.h:27`；符号名常量 `REPORTER_INIT`：`src/include/data_model/reporter_model.h:7`
- 编译清单：`src/Makefile:56-74`（未编译项均以注释或缺席形式可见）
