# Spec: comm-ports — 通信模块

**Subsystem**: comm-ports
**Version**: 0.4
**Created**: 2026-07-01
**Last Updated**: 2026-07-15

---

## Purpose

通信接口层，封装多种通信协议，通过统一的 HAL（硬件抽象层）接口提供传输无关的通信能力。

**核心设计**：所有 9 个协议模块编译为独立的 `.so` 动态库，导出统一的 6 个 HAL 函数符号（`HAL_CommOpen` / `HAL_CommAccept` / `HAL_CommRead` / `HAL_CommWrite` / `HAL_CommControl` / `HAL_CommClose`）。`comm_loader.c` 在运行时通过 `dlopen` + `dlsym` 按需加载，上层业务代码通过 `COMMPORT` 结构体中的函数指针调用，无需感知底层传输方式。

HAL 原型集中定义于 `src/include/comm_ports/comm_port.h`（346 行），各模块通过 `#define XXX_CommOpen HAL_CommOpen` 别名宏复用。每个驱动内部结构的第一个字段必须是 `nLastErrorCode`，以支持 `CommGetLastError` 宏的指针解引用机制。

---

## Requirements

### Requirement: R1 统一通信抽象层

系统 MUST 提供统一的 HAL（硬件抽象层）通信接口，屏蔽不同通信协议的差异。6 个 HAL 函数的原型由 `COMM_OPEN_PROC` / `COMM_ACCEPT_PROC` / `COMM_READ_PROC` / `COMM_WRITE_PROC` / `COMM_CONTROL_PROC` / `COMM_CLOSE_PROC` 六个 typedef 定义，各驱动通过 `#define` 别名复用 HAL 名称导出。

#### Scenario: HAL 函数原型一致性
- **WHEN** 调用任意通信模块的 Open/Accept/Read/Write/Control/Close
- **THEN** 接口原型一致，由 `comm_port.h:232-278` 的 6 个 typedef 约束：
  - `COMM_OPEN_PROC`: `HANDLE (*)(char* pPortDescriptor, char* pOpenParams, DWORD dwPortAttr, int nTimeout, int* pErrCode)`
  - `COMM_ACCEPT_PROC`: `HANDLE (*)(HANDLE hPort)`
  - `COMM_READ_PROC`: `int (*)(HANDLE hPort, char* pBuffer, int nBytesToRead)`
  - `COMM_WRITE_PROC`: `int (*)(HANDLE hPort, char* pBuffer, int nBytesToWrite)`
  - `COMM_CONTROL_PROC`: `int (*)(HANDLE hPort, int nCmd, void* pBuffer, int nDataLength)`
  - `COMM_CLOSE_PROC`: `int (*)(HANDLE hPort)`
- **AND** `comm_port.h:284-291` 声明同名 `HAL_CommOpen` / `HAL_CommAccept` / `HAL_CommRead` / `HAL_CommWrite` / `HAL_CommControl` / `HAL_CommClose` 供 `dlsym` 加载

#### Scenario: 模块别名宏机制
- **WHEN** 通信模块以 `_MAKE_SHARED_LIB` 宏编译为动态库时
- **THEN** 模块通过 `#define` 别名将自身函数映射到 HAL 名称（如 `comm_socketcan.h:27-32`：`#define CAN_CommOpen HAL_CommOpen`，`commsock.h:24-29`：`#define TCPIP_CommOpen HAL_CommOpen`）
- **AND** 若未定义 `_MAKE_SHARED_LIB`（静态测试模式），别名宏不生效，函数保持模块前缀名
- **AND** 6 个符号名由 `SYM_HAL_COMMOPEN`（=`"HAL_CommOpen"`）等宏定义于 `comm_port.h:297-302`

#### Scenario: COMMPORT 结构体承载函数指针
- **WHEN** `comm_loader.c` 加载某端口后
- **THEN** `COMMPORT` 结构体（`comm_port.h:313-327`）填充完毕：
  - `info`（`COMMPORT_INFO`）：端口配置信息
  - `Open` / `Close` / `Accept` / `Read` / `Write` / `Control`：6 个函数指针，指向 `dlsym` 获取的地址
  - `bInUsed`：端口占用标记
  - `hLib`：`dlopen` 返回的动态库句柄
- **AND** `COMMPORT_INFO`（`comm_port.h:304-311`）包含：`nCommPort`（端口 ID，0=虚拟端口）、`szPortName[32]`、`bIsExlusive`、`szPortAttri[64]`、`szPortDriver[64]`（驱动库文件名）

#### Scenario: 超时配置结构
- **WHEN** HAL 函数需要超时参数时
- **THEN** 使用 `COMM_TIMEOUTS` 结构体（`comm_port.h:22-31`）：`nReadTimeout`（-1=永久等待）、`nWriteTimeout`（-1=永久等待）、`nIntervalTimeout`（字符间隔超时）
- **AND** 最小字符间隔为 `TIMEOUT_INTERVAL_CHAR = 50ms`（最低速率 20bps）
- **AND** 初始化宏 `INIT_TIMEOUTS(to, rto, wto)` 将间隔设为默认 50ms，`INIT_TIMEOUTS_EX(to, rto, wto, ito)` 可自定义间隔

---

### Requirement: R2 CAN 总线通信

系统 MUST 支持通过 Linux SocketCAN 接口进行 CAN 总线通信。

**依赖**：Linux 内核 SocketCAN（`<linux/can.h>` / `<linux/can/raw.h>`）

#### Scenario: CAN 帧收发
- **WHEN** 打开 CAN 接口后发送标准 CAN 帧
- **THEN** 使用 `COMM_CANFRAME` 结构体（`comm_port.h:117-122`）：`ulCanID`（`UINT` CAN 标识符）、`nDataLen`（`BYTE` 数据长度）、`byData[8]`（`BYTE` 数据载荷）
- **AND** 该结构体与 `<linux/can.h>` 的 `can_frame` 定义编译兼容

#### Scenario: CAN 过滤器
- **WHEN** 需要过滤接收的 CAN 帧
- **THEN** 使用 `COMM_CAN_FILTER` 结构体（`comm_port.h:124-128`）：`ulCANId`（`UINT` 过滤 ID）、`ulCANMask`（`UINT` 过滤掩码）
- **AND** 通过 `COMM_SET_CAN_FILTER` 控制命令（`COMM_CONTROL_COMMAND_ENUM`）下发到驱动

#### Scenario: CAN 波特率配置
- **WHEN** 通过 `COMM_SET_CAN_BAUDRATE` 控制命令配置波特率
- **THEN** 波特率取自 `COMM_CAN_BAUDRATE` 枚举（`comm_port.h:105-114`）：`COMM_CAN_BAUDRATE_50K` / `_100K` / `_125K` / `_250K` / `_500K` / `_800K` / `_1M`
- **AND** `CAN_PORT_DRV` 结构体（`comm_socketcan.h:81-95`）的 `emBaudRateLevel` 字段存储当前波特率

#### Scenario: CAN 端口重置
- **WHEN** 发送 `COMM_RESET_CAN` 控制命令
- **THEN** 驱动重新初始化 CAN 总线（关闭并重新打开 SocketCAN socket）
- **AND** `CAN_PORT_DRV.szCanPort[8]` 指定 CAN 设备名（如 `"can0"` / `"can1"`）

---

### Requirement: R3 TCP/IP 网络通信

系统 MUST 支持 TCP Server/Client 模式的网络通信。

**依赖**：Linux socket（`<sys/socket.h>` / `<netinet/in.h>`）

#### Scenario: TCP Server 监听与接受连接
- **WHEN** 以 `COMM_SERVER_MODE`（0x00）打开 TCP 端口
- **THEN** 驱动进入 `COMM_LOCAL_SERVER`（=0）模式，绑定并监听指定端口
- **AND** `HAL_CommAccept` 接受客户端连接后，返回新句柄，其工作模式标记为 `COMM_INCOMING_CLIENT`（=2）

#### Scenario: TCP Client 连接
- **WHEN** 以 `COMM_CLIENT_MODE`（0x01）打开 TCP 端口
- **THEN** 驱动进入 `COMM_OUTGOING_CLIENT`（=1）模式，主动连接远程服务器
- **AND** 连接成功后可正常收发数据

#### Scenario: 客户端连接管理
- **WHEN** TCP Server 需要管理多个并发客户端
- **THEN** `COMM_SET_MAX_CLIENTS` 控制命令设置最大允许连接数，默认 `TCPIP_MAX_CLIENTS_DEFAULT = 5`
- **AND** `COMM_GET_CLIENT_LIMITATION` 返回容量上限 `TCPIP_MAX_CLIENT_LIMITAION = 64`
- **AND** `COMM_GET_CUR_CLIENTS` 返回当前已连接客户端数

---

### Requirement: R4 UDP 网络通信

系统 MUST 支持 UDP 数据报通信。

**依赖**：Linux socket

#### Scenario: UDP 收发
- **WHEN** 打开 UDP 端口后发送数据报
- **THEN** 通过 `HAL_CommWrite` 发送，对端通过 `HAL_CommRead` 接收
- **AND** `SUdpPortDriver`（`comm_port.h:41-63`，typedef 为 `UDP_PORT_DRV`）维护通信状态

#### Scenario: UDP 驱动结构体
- **WHEN** UDP 端口处于工作状态
- **THEN** `UDP_PORT_DRV` 结构体包含：`nLastErrorCode`（**必须是第一个字段**）、`nWorkMode`、`nSockPort`、`dwHostAddr`、`inPeerAddr`、`server_addr` / `client_addr` / `send_addr`（`struct sockaddr_in`）、`nSocket`（`SOCKET`）、`toTimeouts`、`nMaxClients`、`*pCurClients`
- **AND** `nLastErrorCode` 作为首字段是 `CommGetLastError` 宏的硬性约束

#### Scenario: UDP Server/Client 模式
- **WHEN** 以 `COMM_SERVER_MODE` 或 `COMM_CLIENT_MODE` 打开 UDP 端口
- **THEN** Server 模式绑定本地端口等待数据，Client 模式向指定 `send_addr` 发送数据
- **AND** 模式值复用 R3 的 `COMM_WORK_MODE_ENUM`：`COMM_LOCAL_SERVER`(0) / `COMM_OUTGOING_CLIENT`(1)

---

### Requirement: R5 WebSocket 通信

系统 MUST 支持 WebSocket 客户端/服务端通信。代码库提供**两个并行实现**，二者均符合统一 HAL 接口规范，但底层库不同。

> **重要**：`comm_websocket_ex/` 基于 **noPoll**，而非 libwebsockets。两个实现通过 `szPortDriver` 配置选择加载。

#### Scenario: WebSocket 连接
- **WHEN** 建立 WebSocket 连接（Client 或 Server 模式）
- **THEN** 可通过 `HAL_CommRead` / `HAL_CommWrite` 双向收发 WebSocket 数据帧
- **AND** 连接底层库（libwebsockets 或 noPoll）对上层透明

#### Scenario: WebSocket 协议模式
- **WHEN** 配置 WebSocket 通信的协议模式
- **THEN** 通过 `dwPortAttr` 参数指定（`comm_port.h:94-97`）：
  - `COMM_OCPP1_6_MODE`(0x00)：OCPP 1.6 JSON 协议
  - `COMM_INFYMBE_MODE`(0x01)：Ingetam MBE 协议
  - `COMM_OCPP2_0_1_MODE`(0x02)：OCPP 2.0.1 协议
  - `COMM_BINARY_MODE`(0x03)：二进制透传模式

#### Scenario: WebSocket 心跳保活
- **WHEN** 发送 `COMM_SEND_WS_PING` 控制命令
- **THEN** 驱动向 OCPP 服务端发送 WebSocket PING 帧
- **AND** `COMM_GET_LASTTIME_RECEIVE_PONG` 返回最近一次收到 PONG 响应的时间戳，用于判断连接活性

#### Scenario: TLS 证书配置
- **WHEN** 发送 `COMM_SET_WS_GLOBAL_TLS_ROOT_CAFILE` 控制命令
- **THEN** 设置 libwebsockets 全局 TLS 根证书文件路径
- **AND** 此命令仅适用于 `comm_websocket/`（libwebsockets）实现，对 `comm_websocket_ex/`（noPoll）无效果

---

### Requirement: R6 HTTP/FTP 通信

系统 MUST 支持 HTTP/HTTPS 请求和 FTP/FTPS 文件传输。

**依赖**：libcurl

#### Scenario: HTTP/FTP 模式选择
- **WHEN** 通过 `dwPortAttr` 指定传输协议模式
- **THEN** 支持以下模式（`comm_port.h:99-103`）：
  - `COMM_HTTP_MODE`(0x10)：HTTP 明文
  - `COMM_HTTPS_MODE`(0x20)：HTTPS（TLS 加密）
  - `COMM_FTP_MODE`(0x30)：FTP 明文
  - `COMM_FTPS_MODE`(0x40)：FTPS（TLS 加密）

#### Scenario: 异步传输进度查询
- **WHEN** HTTP/FTP 传输正在进行中
- **THEN** `HAL_CommRead` 返回 `ERR_COMM_HTTPFTP_PENDING` 表示操作未完成
- **AND** `COMM_GET_HTTPFTP_PROCESS` 控制命令查询当前上传/下载进度

---

### Requirement: R7 进程间通信（IPC）

系统 MUST 支持基于 nanomsg 库的进程间通信。

**依赖**：nanomsg 库

#### Scenario: IPC 模式选择
- **WHEN** 通过 `dwPortAttr` 指定 IPC 通信模式
- **THEN** 支持以下模式（`comm_port.h:139-140`）：
  - `IPC_MODE_REQ_REP`(0)：请求-应答模式（REQ/REP）
  - `IPC_MODE_PUSH_PULL`(1)：推-拉模式（PUSH/PULL）
- **AND** 通过 `COMM_SET_IPC_MODE` 控制命令可在运行时切换模式

#### Scenario: IPC 消息收发
- **WHEN** 两个进程通过 IPC 通道收发消息
- **THEN** `HAL_CommWrite` 发送消息，`HAL_CommRead` 接收消息
- **AND** 若使用错误的 IPC 模式（如 REQ 端调用 Read 而非 Write），返回 `ERR_COMM_WRONG_IPC_MODE`

---

### Requirement: R8 核间通信（RPMSG）

系统 MUST 支持通过 iccmsg 协议进行 AP（Cortex-A55）与 SEC（Cortex-R5）的核间通信。

**依赖**：iccmsg（SSDK）+ 内核 rpmsg/ipcc 驱动

#### Scenario: 核间消息收发
- **WHEN** AP 域通过 rpmsg 发送消息到 SEC 域
- **THEN** SEC 域正确接收消息
- **AND** SEC 域的回复能正确返回 AP 域

#### Scenario: 多帧重组
- **WHEN** 发送超过单帧容量的消息
- **THEN** 系统自动分包发送并在接收端重组
- **AND** 重组逻辑在驱动内部实现，对上层 HAL 接口透明

##### Test: 核间 IPC 延迟
- **Type**: performance
- **Method**: 通过 `rpmsg_bsp/src/echo_test.c` 的 `latency_test()` 发送消息并测量往返时间；延迟统计字段见 `iccmsg_queue.h`（`avg_latency_us`/`total_latency_us`）
- **Expected**: 记录并输出延迟统计；具体阈值待目标硬件标定（spec 此前写死的 100ms 阈值在代码中无依据，已移除）

---

### Requirement: R9 串口通信

系统 MUST 支持标准串口（RS232/RS485）通信。

**依赖**：POSIX termios

#### Scenario: 平台自适应串口设备名
- **WHEN** 编译时定义 `IMMU3` 宏（IMMU3 平台）
- **THEN** 串口设备名前缀为 `ttyO`（`comm_port.h:335-338`）：`TTYS_ONLY_NAME="ttyO"`、`TTYS_FULL_PREFIX="/dev/ttyO"`、`TTYS_RS485_1="/dev/ttyO3"`、`TTYS_IEC101="/dev/ttyO1"`
- **AND** `TTYS_ALIAS_NAME="COM"` 作为统一别名

#### Scenario: 非 IMMU3 平台串口设备名
- **WHEN** 编译时未定义 `IMMU3` 宏（如 D9360/BMU05/EMU01 平台）
- **THEN** 串口设备名前缀为 `ttyS`（`comm_port.h:340-343`）：`TTYS_ONLY_NAME="ttyS"`、`TTYS_FULL_PREFIX="/dev/ttyS"`、`TTYS_RS485_1="/dev/ttyS3"`、`TTYS_IEC101="/dev/ttyS1"`
- **AND** `TTYS_USB2="/dev/ttyUSB2"` 在所有平台上可用

#### Scenario: 串口参数配置
- **WHEN** 通过 `COMM_SET_SERIAL_ATTRIB` 控制命令设置串口属性
- **THEN** 配置波特率/数据位/校验位/停止位（如 `"9600,n,8,1"` 格式）
- **AND** 底层使用 termios API 实现配置

#### Scenario: RS485/RS232 模式切换
- **WHEN** 发送 `COMM_SET_SERIAL_485` 控制命令
- **THEN** 串口切换为 RS485 模式
- **AND** 发送 `COMM_SET_SERIAL_485_To232` 控制命令时，RS485 端口转换为 RS232 模式

---

### Requirement: R10 通信端口加载

系统 MUST 在初始化时通过 `comm_loader.c` 统一加载所有配置的通信端口。

#### Scenario: 端口遍历与加载
- **WHEN** 系统启动时调用 `Init_Load_AllCommPorts(CHARGER* pCharger)`
- **THEN** 遍历 `pCharger->commports[]` 数组（共 `nTotal` 个端口）
- **AND** 对每个端口调用 `Load_ACommPort(COMMPORT* pCommPort)` 加载
- **AND** 若 `pCommPort->info.nCommPort == 0`（虚拟/dummy 端口），跳过加载

#### Scenario: 动态库加载流程
- **WHEN** `Load_ACommPort` 加载单个端口
- **THEN** 构造库路径：`$ENV_VARIABLE_APP_PATH/comm_port/<szPortDriver>`
- **AND** 调用 `LoadDynamicLibrary` 加载 6 个符号（`SYM_HAL_COMMOPEN` / `SYM_HAL_COMMACCEPT` / `SYM_HAL_COMMREAD` / `SYM_HAL_COMMWRITE` / `SYM_HAL_COMMCONTROL` / `SYM_HAL_COMMCLOSE`），`TRUE` 参数表示所有符号必须找到
- **AND** 符号地址写入 `pCommPort->Open/Accept/Read/Write/Control/Close`，`pCommPort->hLib` 保存 `dlopen` 句柄

#### Scenario: 加载失败处理
- **WHEN** 动态库文件不存在、符号缺失或 `dlopen` 失败
- **THEN** `Load_ACommPort` 返回 `FALSE`，`pCommPort->hLib` 为 `NULL`
- **AND** `Init_Load_AllCommPorts` 记录错误日志 `"Can NOT load the comm port [%s]"` 并返回 `FALSE`，终止启动流程

---

### Requirement: R11 控制命令接口

系统 MUST 通过 `HAL_CommControl` 提供统一的运行时控制接口，使用 `COMM_CONTROL_COMMAND_ENUM` 枚举区分不同命令。

#### Scenario: 获取最近错误码
- **WHEN** 发送 `COMM_GET_LAST_ERROR`(=1) 命令（`comm_port.h:150`，`COMM_CONTROL_COMMAND_ENUM` 的首个标准命令）
- **THEN** 返回驱动结构体首字段 `nLastErrorCode` 的值，即最近一次操作的错误码
- **AND** 该命令与 `CommGetLastError(hPort)` 宏（`comm_port.h:144`）语义等价：二者均读取驱动结构体的 `nLastErrorCode` 字段，区别在于 `COMM_GET_LAST_ERROR` 经由 `HAL_CommControl` 函数调用入口查询，而 `CommGetLastError` 宏直接做 `(**(int**)(hPort))` 双重解引用
- **AND** 此命令要求 `nLastErrorCode` MUST 为驱动结构体首字段（见 R12），否则读取到的错误码无效

#### Scenario: 超时控制
- **WHEN** 发送 `COMM_SET_TIMEOUTS`(=3) 命令，`pBuffer` 指向 `COMM_TIMEOUTS` 结构体，`nDataLength = sizeof(COMM_TIMEOUTS)`
- **THEN** 驱动更新内部超时配置（读超时、写超时、字符间隔超时）
- **AND** `COMM_GET_TIMEOUTS`(=4) 命令读取当前超时配置到 `COMM_TIMEOUTS` 结构体

#### Scenario: 缓冲区清除
- **WHEN** 发送 `COMM_PURGE_TXCLEAR`(=5) 命令
- **THEN** 清除驱动的发送缓冲区
- **AND** 发送 `COMM_PURGE_RXCLEAR`(=6) 命令时，清除接收缓冲区

#### Scenario: 客户端连接管理
- **WHEN** 发送 `COMM_SET_MAX_CLIENTS`(=7) 命令
- **THEN** 设置 Server 模式端口允许的最大客户端连接数
- **AND** `COMM_GET_MAX_CLIENTS`(=8) 返回当前设置的最大客户端数
- **AND** `COMM_GET_CUR_CLIENTS`(=9) 返回当前已连接的客户端数
- **AND** `COMM_GET_CLIENT_LIMITATION`(=10) 返回驱动支持的最大客户端容量

#### Scenario: 对端信息查询
- **WHEN** 发送 `COMM_GET_PEER_INFO`(=2) 命令，`pBuffer` 指向 `COMM_PEER_INFO` 结构体，`nDataLength = sizeof(COMM_PEER_INFO)`
- **THEN** 填充对端信息：`nPeerType`、`nPeerPort`、`szAddr[32]`（IP 地址或电话号码）
- **AND** `MAX_LEN_PEER_ADDR = 32`

#### Scenario: 私有命令扩展
- **WHEN** 驱动需要自定义控制命令
- **THEN** 使用 `COMM_PRIVATE_COMMAND`(=255) 作为基准值，大于 255 的值由各 HAL 驱动自定义
- **AND** 标准命令范围 1-254 由 `COMM_CONTROL_COMMAND_ENUM` 统一管理

---

### Requirement: R12 错误处理与状态查询

系统 MUST 通过统一的 `COMM_ERROR` 枚举和 `CommGetLastError` 宏提供错误处理机制。

#### Scenario: 错误码体系
- **WHEN** HAL 函数执行失败
- **THEN** 返回值表示失败（`HANDLE` 返回 `NULL`/`INVALID_SOCKET`，`int` 返回 `SOCKET_ERROR`=-1）
- **AND** 详细的错误码通过驱动结构体的 `nLastErrorCode` 字段记录，取自 `COMM_ERROR` 枚举（`comm_port.h:65-88`）的 21 个错误码

#### Scenario: CommGetLastError 宏机制
- **WHEN** 调用 `CommGetLastError(hPort)` 宏（`comm_port.h:144`）
- **THEN** 展开为 `(**(int**)(hPort))`，即解引用 `hPort` 指针两次读取 `nLastErrorCode` 的值
- **AND** 此机制**硬性要求**所有驱动结构体的第一个字段必须是 `int nLastErrorCode`（如 `UDP_PORT_DRV`、`CAN_PORT_DRV`、`STcpipPortDriver` 均标注 `"NOTE: MUST BE THE FIRST FIELD!"`）

#### Scenario: 控制命令错误分类
- **WHEN** `HAL_CommControl` 执行失败
- **THEN** 错误细分为三类：
  - `ERR_COMM_CTRL_COMMAND`：不支持的控制命令（`nCmd` 值无效）
  - `ERR_COMM_CTRL_PARAMS`：控制命令参数无效（`pBuffer` 为 `NULL` 或 `nDataLength` 不匹配）
  - `ERR_COMM_CTRL_EXECUTION`：控制命令执行失败（运行时错误，如设置波特率失败）

#### Scenario: 常见运行时错误码
- **WHEN** 通信过程中发生各种异常
- **THEN** `COMM_ERROR` 枚举覆盖以下场景（`comm_port.h:66-87`）：
  - `ERR_COMM_OK`(0)：成功
  - `ERR_COMM_NO_MEMORY`(1)：内存分配失败
  - `ERR_COMM_TIMEOUT`(2)：读写超时
  - `ERR_COMM_OPENING_PORT`(3)：打开端口失败
  - `ERR_COMM_OPENING_PARAM`(4)：打开参数错误
  - `ERR_COMM_SUPPORT_ACCEPT`(5)：协议不支持连接接受（对 UDP/Serial 等无连接协议调用 `HAL_CommAccept`，详见「Accept 不支持检测」场景）
  - `ERR_COMM_MANY_CLIENTS`(6)：客户端连接数超限
  - `ERR_COMM_ACCEPT_FAILURE`(7)：TCP Server `accept()` 系统调用失败（详见「Accept 不支持检测」场景）
  - `ERR_COMM_READ_DATA`(8) / `ERR_COMM_WRITE_DATA`(9)：读写数据错误
  - `ERR_COMM_CONNECT_SERVER`(10)：Client 连接服务器失败
  - `ERR_COMM_CREATE_SERVER`(11)：Server 创建失败
  - `ERR_COMM_PORT_HANDLE`(12)：无效端口句柄
  - `ERR_COMM_CTRL_COMMAND`(13)：不支持的控制命令（`nCmd` 值无效，详见「控制命令错误分类」场景）
  - `ERR_COMM_CTRL_PARAMS`(14)：控制命令参数无效（`pBuffer` 为 `NULL` 或 `nDataLength` 不匹配，详见「控制命令错误分类」场景）
  - `ERR_COMM_CTRL_EXECUTION`(15)：控制命令执行失败（运行时错误，如设置波特率失败，详见「控制命令错误分类」场景）
  - `ERR_COMM_BUFFER_SIZE`(16)：缓冲区大小不足
  - `ERR_COMM_CONNECTION_BROKEN`(17)：连接断开
  - `ERR_COMM_STD_PORT`(18)：无效/不支持的端口标识 —— 表示 Open 时端口 ID 非法（`modbus_reporter` 在 `InitTcpCommPort` 的 Open 错误码分支中消费此码，trace 输出 `"Open Comm Port failed ID invalid"`，见 `modbus_main.c:496`）。**注意**：当前各 HAL 驱动源码（`src/comm_ports/`）均未赋值此码，为枚举中定义保留的错误码，仅在 reporter 侧做防御性匹配
  - `ERR_COMM_WRONG_IPC_MODE`(19)：IPC 模式不匹配
  - `ERR_COMM_HTTPFTP_PENDING`(20)：HTTP/FTP 异步操作未完成

#### Scenario: Accept 不支持检测
- **WHEN** 对不支持连接接受的协议（如 UDP、Serial）调用 `HAL_CommAccept`
- **THEN** 返回 `ERR_COMM_SUPPORT_ACCEPT` 错误码
- **AND** `ERR_COMM_ACCEPT_FAILURE` 表示 TCP Server `accept()` 系统调用失败

---

## Modules

| 模块 | 目录 | 对应 R | 底层依赖 |
|------|------|:------:|------|
| CAN | comm_can/ | R2 | Linux SocketCAN |
| TCP/IP | comm_net_tcpip/ | R3 | Linux socket |
| UDP | comm_net_udp/ | R4 | Linux socket |
| WebSocket | comm_websocket/ | R5 | libwebsockets |
| WebSocket EX | comm_websocket_ex/ | R5 | **noPoll** ⚠️ 非 libwebsockets |
| HTTP/FTP | comm_http_ftp/ | R6 | curl |
| IPC | comm_ipc/ | R7 | nanomsg |
| RPMSG | comm_rpmsg/ | R8 | iccmsg (SSDK) + 内核 rpmsg/ipcc 驱动 |
| Serial | comm_std_serial/ | R9 | termios |
| Loader | comm_loader.c | R10 | dlopen/dlsym 动态加载各模块 |
| Control | comm_port.h（COMM_CONTROL_COMMAND_ENUM） | R11 | 各驱动实现 |
| Error | comm_port.h（COMM_ERROR enum + CommGetLastError） | R12 | 各驱动 nLastErrorCode 首字段约定 |

---

## Test Definition

> **现状说明**：以下"白盒测试"列描述的是**目标态**。当前代码库中所有 comm_ports 模块的 `Makefile` `TEST_EXEC =`（空），全项目零测试文件。以下表格记录的是各 R 达到白盒可测的难点和策略，待测试基础设施建立后实施。

| Requirement | 白盒测试 | 方式 |
|------------|:--------:|------|
| R1 统一抽象层 | ✅ 可测 | 静态检查各模块头文件中 6 个 `#define` 别名宏 + HAL typedef 一致性 |
| R2 CAN | ⚠️ 中等 | mock SocketCAN 层（`<linux/can.h>`）；验证 `COMM_CANFRAME` / `COMM_CAN_FILTER` 序列化 |
| R3 TCP/IP | ⚠️ 中等 | mock socket 层；验证 Server/Client 模式切换和客户端计数 |
| R4 UDP | ⚠️ 中等 | mock socket 层；验证 `UDP_PORT_DRV` 首字段约束 |
| R5 WebSocket | ⚠️ 中等 | `comm_websocket/` mock libwebsockets；`comm_websocket_ex/` mock noPoll（两个独立实现需分别测试） |
| R6 HTTP/FTP | ⚠️ 中等 | mock curl；验证异步 `ERR_COMM_HTTPFTP_PENDING` 流转 |
| R7 IPC | ⚠️ 中等 | mock nanomsg；验证 REQ/REP 与 PUSH/PULL 模式 |
| R8 RPMSG | ⚠️ 中等 | 单元层可 mock `iccmsg_hal_t` / `iccmsg_ll_t`；端到端往返延迟测试（`echo_test latency_test`）需真实硬件 |
| R9 Serial | ⚠️ 中等 | mock termios；验证 `#ifdef IMMU3` 平台条件编译路径 |
| R10 端口加载 | ✅ 可测 | mock `dlopen` / `dlsym`；验证 dummy 端口跳过和符号缺失处理 |
| R11 控制命令 | ✅ 可测 | mock 各驱动 Control 函数；验证 `COMM_CONTROL_COMMAND_ENUM` 各命令的参数校验 |
| R12 错误处理 | ✅ 可测 | 验证 `CommGetLastError` 宏指针解引用；构造首字段非 `nLastErrorCode` 的结构体验证约束 |

---

## Key Files

> HAL 原型和公共定义在 `src/include/comm_ports/comm_port.h`（346 行）。加载器在 `src/comm_ports/comm_loader.c`。各协议模块在 `src/comm_ports/<module>/` 下独立编译为 `.so`。

| 文件 | 对应 R | 说明 |
|------|:------:|------|
| comm_port.h | R1,R11,R12 | **HAL 核心头文件**：6 个 typedef + 声明、SYM_HAL_* 符号名、COMMPORT/COMMPORT_INFO 结构体、COMM_ERROR 枚举(20码)、COMM_CONTROL_COMMAND_ENUM(20+命令)、COMM_TIMEOUTS、COMM_PEER_INFO、平台 TTYS_* 宏 |
| comm_loader.c | R10 | 动态加载器：遍历 `commports[]`，`LoadDynamicLibrary` + `dlsym` 加载 6 个符号 |
| comm_loader.h | R10 | 加载器接口声明 `Init_Load_AllCommPorts` |
| comm_socketcan.h | R1,R2 | CAN 驱动头文件：`CAN_Comm*` 别名宏、`CAN_PORT_DRV` 结构体 |
| commsock.h | R1,R3 | TCP/IP 驱动头文件：`TCPIP_Comm*` 别名宏、`STcpipPortDriver` 结构体、`TCPIP_MAX_CLIENTS_*` 常量 |
| comm_port.h:41-64 | R4 | `SUdpPortDriver`/`UDP_PORT_DRV` 结构体定义（首字段 nLastErrorCode 约束） |
| comm_port.h:93-97 | R5 | WebSocket 协议模式常量 `COMM_OCPP1_6_MODE` 等 |
| comm_port.h:99-103 | R6 | HTTP/FTP 模式常量 `COMM_HTTP_MODE` 等 |
| comm_port.h:139-140 | R7 | IPC 模式常量 `IPC_MODE_REQ_REP` / `IPC_MODE_PUSH_PULL` |
| comm_port.h:330-344 | R9 | 平台串口设备名宏（`#ifdef IMMU3` 条件编译） |

---

## Known Tech Debts

1. **双 WebSocket 实现** — `comm_websocket/`（libwebsockets）与 `comm_websocket_ex/`（noPoll）并存，维护成本高，需通过配置选择加载
2. **零测试代码** — 所有 comm_ports 模块 `Makefile` `TEST_EXEC =`（空），全模块无单元测试
3. **`CommGetLastError` 首字段约束无编译期保证** — `nLastErrorCode` 必须为驱动结构体首字段是运行时约定，若新驱动违反此约定将导致读取错误码时解引用到错误地址
4. **IMMU3 平台条件编译** — `#ifdef IMMU3` 分裂串口设备名（ttyO vs ttyS），未抽象为配置驱动，移植到新平台需修改头文件
5. **`COMM_PRIVATE_COMMAND`=255 预留** — 各驱动私有命令无统一注册机制，存在命令号冲突风险
