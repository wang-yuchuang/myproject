# 第三方依赖库清单

> **性质**：接口契约 / 参考
> **需求权威源**：暂无（第三方依赖清单，无对应 capability）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`65dab6f7`（`src/include/3rd_party/` 目录实测）
> **状态**：🟠 部分

本项目第三方依赖库统一放置在 `src/include/3rd_party/` 目录下，并通过 `src/include.mk` 链接进各模块。下表列出当前目录内实测存在的 16 个条目。

| 目录 | 用途（推断） | 备注 |
|---|---|---|
| curl | HTTP 请求 | 动态/静态未核实 |
| iec61850.bak | IEC 61850 旧版备份 | ⚠️ 名字含 .bak，是否仍被引用待核实 |
| libexpat | XML 解析 | `include.mk` 链接 `-lexpat` |
| libiec104 | IEC 104 协议 | 对应 `iec_104_reporter`（当前未编译） |
| libpahomqtt | MQTT 客户端 | 目录名即为 `libpahomqtt` |
| libqrcode | 二维码生成 | — |
| libsocketcan | CAN socket 封装 | 对应 `comm_can` |
| libwebsockets | WebSocket | 另有 `nopoll`（`comm_websocket_ex` 用 nopoll，见 comm-ports spec） |
| log4c | 日志 | `include.mk` 链接 `-llog4c` |
| nanomsg | 消息队列 | — |
| nopoll | 轻量 WebSocket | `comm_websocket_ex` 依赖 |
| openssl | 加密 | `include.mk` 链接 `libssl.so.1.1` / `libcrypto.so.1.1`（动态） |
| packages | 第三方库源码目录 | 内含各库源码 |
| sqlite | 本地数据库 | `include.mk` 链接 `-lsqlite3` |
| tslib | 触摸屏 | — |
| zlib | 压缩 | `include.mk` 链接 `libz.so.1.2.11` |

说明：本页从已移除的 `code-analysis/legacy_app.md`（2026-07-01）中抢救出仅有的有效内容；原文另含一个签名 token，已**有意不迁移**。
