# 术语表（用词基准）

> **性质**：参考表（索引类）
> **需求权威源**：`openspec/specs/`（各行出处见「出处」列）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`86f803b5`
> **状态**：✅ 已核对

本表是**用词基准**：写文档、写 spec 时以此为准；同一概念全仓只用一种写法。新术语先加表、再使用。出处指向 spec 的 `R<n>`、代码 `file:line` 或事实快照；无法在仓内核实全称的条目标 `⚠️ 待核实`。

## 业务域

| 术语 | 全称 / 展开 | 含义（一句话） | 出处 |
|---|---|---|---|
| 储能后备管理 | — | 本系统（Solaris）的业务定位：储能系统的后备电源管理 | `openspec/config.yaml` context；`AGENTS.md` 头部 |
| BMS | Battery Management System | 电池管理系统，经 BMS 协议采集储能电池数据 | samplers R7 |
| BMU | Battery Management Unit ⚠️ 待核实 | 电池侧从设备，升级顺序在 BSU 之后 | samplers R7（`BSU→BMU→IMEU2→IMEU3`）；全称仓内无明文 |
| BSU | Battery Stack Unit ⚠️ 待核实 | 电池侧升级链的首个设备，必须先于 BMU 升级 | samplers R7 |
| PCS | Power Conversion System | 功率变换系统，数据经 CCU 采样、web 后台可查 | samplers R8；reporters R5 |
| CCU | Central Control Unit | 中央控制单元，最大采样器 `ccu_sampler` 的对接对象 | samplers R8 |
| EMS | Energy Management System | 能量管理系统，含分时段参数与从站 EMS 上报 | samplers R8；reporters R11 |
| LCCfg / LC | Load Configuration（负载配置）⚠️ 待核实 | 从 DAI 读取的负载配置组：系统类型 / 应用场景 / ESC 数量 / 支路数 / 柜表等 | reporters `GetLCCfg_Reporter()`（spec.md:28） |
| 支路 / 柜 | — | LCCfg 中的层级计数维度（支路数、柜表），电池柜为物理单元 | reporters spec.md:28；samplers spec.md:315 |

## 运行实体

| 术语 | 全称 / 展开 | 含义（一句话） | 出处 |
|---|---|---|---|
| nccu_app | — | 主业务进程，6 步加载（DPR→配置→DAI→comm→samplers→reporters） | app-lifecycle R8 |
| syshand | system hand | OS 命令代理守护，System V IPC + 心跳；心跳丢失仅记录不重启 | system-hand R3；`handler.c:114-125` |
| app_entry | — | 应用入口 shell：写 PID、trap SIGTERM，转交 startrun | app-lifecycle R7 |
| startrun | — | 启动脚本：设环境变量（`NCCU_DATA_PATH` 等）、按升级介质分流 | app-lifecycle R7/R12；`startrun:15` |
| run_app | — | 运行时准备脚本：起 `syshand` 再起 `nccu_app` | app-lifecycle R7；`run_app:136/191` |
| stoprun | — | 停止脚本：`killall -9 app_update` 等清理动作 | app-lifecycle R9；`stoprun:12` |
| app_update | — | 升级控制台程序（源自 `usb_update/update_console`），SD/USB 升级模式被 startrun 调起 | `src/Makefile:11`；`startrun:98-105` |
| log_view | — | 日志查看工具（`log_viewer/` 编译产物），`log_cat` 包装调用 | `src/utilities/log_viewer/Makefile` EXEC；`log_cat:18` |
| app_sysmgr | — | AP↔R 跨核管理守护（BSP 交付），BOOT_READY 握手 / MARK_GOOD / 心跳 | app-lifecycle R10 |
| S91app_sysmgr | — | 拉起 app_sysmgr 的 init 脚本 | app-lifecycle R10；快照 `## app-sysmgr` |
| S92app_mount | — | SDK 侧 init 脚本：选槽 + 验 D9APPMD1 + dm-verity 挂载 `/app`（只挂载、不启动） | app-lifecycle R7；快照「启动责任真空」 |

## 数据面

| 术语 | 全称 / 展开 | 含义（一句话） | 出处 |
|---|---|---|---|
| DPR | Data Processor | 数据中心：配置 / 事件 / 告警 / 表达式四合一核心 | data-processor R1-R4 |
| sampler | 采样器 | 对接设备的采集模块（.so），4 个 DLLExport | samplers `sampler_base.h:68-77` |
| reporter | 上报器 | 对接云/后台的上报模块（.so），仅 1 个 `Initialize` 导出 | reporters `reporters_base.h:27` |
| Intern_Get / Intern_Set | — | DPR 对外的统一跨模块数据读写函数指针 | data-processor spec.md:186,221 |
| DAI | Data Access Interface | 数据访问接口：sampler/reporter 经它读写 DPR 数据 | app-lifecycle R8 步骤 3 |
| DAITYPE_* | — | DAI 操作类型枚举（设备/参数/告警/状态值等） | `src/include/data_model/dai_model.h:10-21` |
| System.cfg | — | SQLite 配置库（非 INI），路径 `/app/config/System.cfg` | data-processor R1 |
| nccu.data | — | 运行数据 SQLite 库，位于 `/data/` 顶层（由 `NCCU_DATA_PATH` 决定） | 快照「运行时可写路径」；`iDataBase.c:66` |
| syshand.run | — | syshand 就绪标志文件，nccu_app 启动时等待它 | app-lifecycle R7；`main.c:316` |

## 通信

| 术语 | 全称 / 展开 | 含义（一句话） | 出处 |
|---|---|---|---|
| HAL | Hardware Abstraction Layer | 通信抽象层：9 个模块统一 6 函数接口 | comm-ports R1；`comm_port.h:284-291` |
| HAL_CommOpen 等 6 函数 | — | Open/Accept/Read/Write/Control/Close，模块经 `#define` 别名复用 | comm-ports R1 |
| comm_ports | — | 通信 HAL 模块目录（9 个 .so，dlsym 动态加载） | comm-ports R10 |
| RPMSG | Remote Processor Messaging | AP↔R 核间通信（iccmsg BSP 库） | comm-ports R8 |
| noPoll | — | `comm_websocket_ex` 的 WebSocket 库（非 libwebsockets） | comm-ports R5；`app-lifecycle R14` |
| libwebsockets | — | `comm_websocket` 的 WebSocket 库，与 noPoll 并存按配置选用 | comm-ports R5 |
| paho MQTT | — | Eclipse Paho MQTT 客户端库（`es_mqtt_reporter` 内置源码） | reporters 依赖表 |
| Modbus TCP | — | Modbus TCP 上报协议（`modbus_reporter`） | reporters R8 附近模块表 |
| IEC 101/104/61850 | — | 电力规约上报器（存在源码但当前未编译） | reporters 未编译清单 |
| OCPP | Open Charge Point Protocol | EV 充电桩对接协议（`occp_reporter`，未编译） | reporters R9 |
| iOCPP | Infypower OCPP | Infypower 私有 OCPP 变体（`es_iocpp_reporter`，已编译） | reporters R12 |

## 平台 / 交付

| 术语 | 全称 / 展开 | 含义（一句话） | 出处 |
|---|---|---|---|
| RAUC | Robust Auto-Update Controller | A/B 槽位 OTA 框架，`rauc install` 安装 bundle | app-lifecycle R6 |
| A/B slot | — | app_a/app_b 双槽位，安装到非活动槽、失败可回滚 | app-lifecycle R6 |
| slot-status | — | RAUC 槽位状态文件，查版本用 `cat /var/lib/rauc/slot-status` | app-lifecycle R9 方法注记 |
| D9APPMD1 | D9 App MetaData 1 ⚠️ 待核实 | app.img 尾部 8 字节 magic，构建后校验、挂载前验证 | `build.sh:137`；`sign_metadata.sh:76` |
| dm-verity | device-mapper verity | 只读 ext4 的哈希树完整性校验（sha256 / 4096 块） | app-lifecycle R4 |
| /app | — | dm-verity 只读挂载的 app 分区挂载点 | config.yaml 约束 |
| /data | — | SQLite 数据库与持久数据目录（`NCCU_DATA_PATH`） | 快照「运行时可写路径」 |
| /opt/appdata | — | 可写目录（log/config/data），实为 `/data/rootfs/opt/appdata` 的 bind mount | 快照同节（2026-09-16 复核） |
| MU_TYPE | — | 平台变体选择变量（当前恒为 `mu3`），驱动 mk_define 与 make install | `build.sh:28,85-94` |
| -DIMMU3 | — | IMMU3 平台编译宏，叠加 `src/shells/mu3/` 脚本变体 | app-lifecycle R12；config.yaml |
| RAUC_COMPATIBLE | — | bundle 兼容性标识（当前 `bmu02_a00`），设备不符则拒装 | `build.sh:27,149` |
| .raucb | — | RAUC OTA bundle 产物文件扩展名 | app-lifecycle R6 |
| app-post-install | — | RAUC 安装钩子（BSP 侧），安装后启动 `app_entry`（OTA 路径唯一启动者） | 快照「OTA 路径」 |

## 已知写法陷阱（反面清单）

- `comm_websocket_ex` ≠ libwebsockets：写 WebSocket EX 时必须写 noPoll。
- DB 在 `/data/` 顶层，`/opt/appdata/data/nccu.data` 一类路径是错的。
- init 脚本名是 `S92app_mount`，不是 `S91app_mount`（S91 已被 app_sysmgr 占用）。
- `AppLog.data` 是 SQLite 库不是文本日志，别拿 grep 当事件计数。
