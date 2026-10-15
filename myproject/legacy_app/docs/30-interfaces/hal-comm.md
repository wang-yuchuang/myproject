# 通信 HAL 统一接口（6 函数）

> **性质**：接口契约
> **需求权威源**：`openspec/specs/comm-ports/spec.md`（R1 统一 HAL / R10 动态加载）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`97cac555`
> **状态**：🟠 部分

## 调用方 / 被调方

- 调用方：samplers、reporters 及各服务器业务模块。它们不直接调用 `HAL_Comm*` 符号，而是经 `COMMPORT` 结构体的 6 个函数指针调用（`src/include/comm_ports/comm_port.h:317-322`）。
- 被调方：9 个协议模块动态库（`libcomm_serial.so` / `libcomm_can.so` / `libcomm_tcpip.so` / `libcomm_httpftp.so` / `libcomm_ipc.so` / `libcomm_ws.so` / `libcomm_ws_ex.so` / `libcomm_udp.so` / `libcomm_rpmsg.so`）。模块能力与底层依赖见功能页 `20-features/communications.md`，本页不重复。
- 加载器：`src/comm_ports/comm_loader.c` 编译进 nccu_app 主程序（`src/main/Makefile:13`），在启动第 4/6 步由 `Init_Load_AllCommPorts` 调用（`src/main/main.c:174`），运行时 `dlopen` + `dlsym` 填充 `COMMPORT` 函数指针。

## 原型

6 个 typedef 约束各驱动实现（`comm_port.h:232-278`），6 个 `HAL_Comm*` 同名声明供 `dlsym` 加载（`comm_port.h:284-291`）：

```c
typedef HANDLE (*COMM_OPEN_PROC)(IN char *pPortDescriptor, IN char *pOpenParams,
    IN DWORD dwPortAttr, IN int nTimeout, OUT int *pErrCode);          /* :232-239 */
typedef HANDLE (*COMM_ACCEPT_PROC)(IN HANDLE hPort);                   /* :242-245 */
typedef int (*COMM_READ_PROC)(IN HANDLE hPort, OUT char *pBuffer,
    IN int nBytesToRead);                                              /* :248-253 */
typedef int (*COMM_WRITE_PROC)(IN HANDLE hPort, IN char *pBuffer,
    IN int nBytesToWrite);                                             /* :256-261 */
typedef int (*COMM_CONTROL_PROC)(IN HANDLE hPort, IN int nCmd,
    IN OUT void *pBuffer, IN int nDataLength);                         /* :264-270 */
typedef int (*COMM_CLOSE_PROC)(IN HANDLE hPort);                       /* :275-278 */

HANDLE HAL_CommOpen( IN char *pPortDescriptor, IN char *pOpenParams,
	IN DWORD dwPortAttr, IN int nTimeout, OUT int *pErrCode );         /* :284-285 */
HANDLE HAL_CommAccept( IN HANDLE hPort );                              /* :286  */
int HAL_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead);    /* :287 */
int HAL_CommWrite(IN HANDLE hPort, IN char *pBuffer, IN int nBytesToWrite);   /* :288 */
int HAL_CommControl( IN HANDLE hPort, IN int nCmd,
	IN OUT void *pBuffer, IN int nDataLength);                         /* :289-290 */
int HAL_CommClose( IN HANDLE hPort );                                  /* :291  */
```

导出符号名由 `SYM_HAL_COMMOPEN` 等 6 个宏给定（`comm_port.h:297-302`）。各模块编译为 `.so` 时经别名宏把自身实现映射到 HAL 名，如 `#define CAN_CommOpen HAL_CommOpen`（`comm_socketcan.h:27-32`）、`#define TCPIP_CommOpen HAL_CommOpen`（`commsock.h:24-29`）；静态编译（未定义 `_MAKE_SHARED_LIB`）时别名不生效，函数保持模块前缀名（spec R1）。

## 语义与约束

- **句柄**：`HANDLE` 即 `void*`（`src/include/base_defines/basetypes.h:33`）。`HAL_CommOpen` 成功返回驱动内部结构体指针（如 `commsock.c:470`），失败返回 `NULL`（`commsock.c:412/432/459`），具体错误经 `pErrCode` 带出。
- **`HAL_CommAccept` 仅 TCP 有实际语义**：`comm_net_tcpip` 只在端口以 `COMM_LOCAL_SERVER` 模式打开时执行 `accept()` 并返回客户端新句柄，否则置 `ERR_COMM_SUPPORT_ACCEPT` 返回 `NULL`（`commsock.c:473-511`）。其余 8 个模块均为占位实现：serial / udp 置 `ERR_COMM_SUPPORT_ACCEPT`（`commserial.c:470`、`commsock_udp.c:523`）；http_ftp / websocket_ex 返回 `NULL`（`comm_httpftp.c:113`、`comm_websocket_np.c:367`）；can / ipc / rpmsg / websocket(libwebsockets) 直接返回原句柄（`comm_socketcan.c:151`、`comm_ipc.c:142`、`commrpmsg.c:58`、`comm_websocket.c:260`）。
- **`HAL_CommControl` 是统一运行时控制入口**，控制码定义于 `COMM_CONTROL_COMMAND_ENUM`（`comm_port.h:148-188`）。全仓上层（samplers/reporters/publib/main）实际用到的控制码：`COMM_SET_SERIAL_ATTRIB`、`COMM_SET_SERIAL_485`、`COMM_SET_SERIAL_485_To232`、`COMM_SET_TIMEOUTS`、`COMM_PURGE_RXCLEAR`、`COMM_PURGE_TXCLEAR`、`COMM_SET_CAN_FILTER`、`COMM_SET_MAX_CLIENTS`、`COMM_SET_IPC_MODE`、`COMM_GET_PEER_INFO`、`COMM_GET_LAST_ERROR`、`COMM_GET_HTTPFTP_PROCESS`、`COMM_SEND_WS_PING`、`COMM_GET_LASTTIME_RECEIVE_PONG`、`COMM_SET_WS_GLOBAL_TLS_ROOT_CAFILE`。枚举中其余控制码（如 `COMM_RESET_CAN`、`COMM_GET_TIMEOUTS`）当前无上层调用。
- **驱动结构体首字段必须是 `int nLastErrorCode`**：`CommGetLastError(hPort)` 宏展开为 `(**(int **)(hPort))`，把句柄双重解引用直读首字段（`comm_port.h:144`），字段顺序变动即读到垃圾值。各驱动遵守此约定（如 `UDP_PORT_DRV`，`comm_port.h:43`）。依据：spec R1。
- **Read/Write 返回值**：成功返回累计字节数（如 `commsock.c:666`），失败置 `ERR_COMM_TIMEOUT` / `ERR_COMM_READ_DATA` 等错误码并返回 `-1`（`commsock.c:566-578`）；读超时行为由 `COMM_TIMEOUTS` 控制（`comm_port.h:22-31`，`-1` 为永久等待）。

## 错误处理

- 错误码取值来自 `COMM_ERROR` 枚举（`comm_port.h:65-88`），由各驱动在失败路径写入自身结构体的 `nLastErrorCode`。
- 上层查询有两条路径：直接调 `CommGetLastError(hPort)` 宏读首字段；或经 `HAL_CommControl` 发 `COMM_GET_LAST_ERROR`（=1，`comm_port.h:150`）取同一错误码。
- Read 返回 `-1` 时应先查错误码再决定重试或关闭；`ERR_COMM_CONNECTION_BROKEN` 表示对端断开（如 `comm_websocket_np.c:384-388`）。

## 版本与兼容性

- 6 函数集合的任何增减或签名改动，同时影响：9 个动态库的导出符号、`COMMPORT` 函数指针布局（`comm_port.h:317-322`）、`comm_loader.c` 的符号表（`comm_loader.c:10-18`）以及 spec R1（`openspec/specs/comm-ports/spec.md`）。
- `dlsym` 以 `TRUE` 参数要求 6 个符号全部解析成功，缺任一符号即加载失败、启动终止（`comm_loader.c:32-36`、`:54-59`）；时序叙述见功能页「加载时序与失败行为」。

## 代码位置

| 内容 | 位置 |
|------|------|
| 6 个 typedef（约束驱动实现） | `src/include/comm_ports/comm_port.h:232-278` |
| 6 个 `HAL_Comm*` 声明 | `src/include/comm_ports/comm_port.h:284-291` |
| 导出符号名宏 `SYM_HAL_*` | `src/include/comm_ports/comm_port.h:297-302` |
| `CommGetLastError` 宏 | `src/include/comm_ports/comm_port.h:144` |
| 控制码枚举 | `src/include/comm_ports/comm_port.h:148-188` |
| `COMMPORT` / `COMMPORT_INFO` 结构体 | `src/include/comm_ports/comm_port.h:313-327` / `304-311` |
| 别名宏示例 | `src/comm_ports/comm_net_tcpip/commsock.h:24-29`、`src/comm_ports/comm_can/comm_socketcan.h:27-32` |
| 加载器实现（编译进主程序） | `src/comm_ports/comm_loader.c:8-39`（单端口加载）、`:41-64`（启动遍历） |
| 主程序调用点（第 4/6 步） | `src/main/main.c:173-174` |
