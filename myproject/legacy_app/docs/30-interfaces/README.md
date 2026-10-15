# 接口契约

> **性质**：接口契约
> **需求权威源**：`openspec/specs/comm-ports/spec.md`、`openspec/specs/publib/spec.md`、`openspec/specs/data-processor/spec.md`
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：—（目录索引，版本无关）
> **状态**：✅ 已建立（6 页）

## 目的

记录模块间与对外接口契约。

## 文件清单

- ✅ `hal-comm.md` — HAL 6 函数（comm_port.h）
- ✅ `dai.md` — 数据访问接口（★ 当前无 spec，最大真空）
- ✅ `syshand-ipc.md` — System V IPC 协议与客户端 SDK
- ✅ `sqlite-schema.md` — `System.cfg` 与 `nccu.data` 表结构
- ✅ `reporter-protocols.md` — 各上报协议与 DAI 开关参数
- ✅ `third-party-libs.md` — 第三方依赖库清单（`src/include/3rd_party/`）

## 权威源与依赖

- `openspec/specs/comm-ports/spec.md`
- `openspec/specs/publib/spec.md`
- `openspec/specs/data-processor/spec.md`

## 待办

- [ ] 补 DAI：`src/data_interfaces/dai_loader.c` 无对应 spec，最高优先
- [ ] 固化 HAL 6 函数原型与语义
- [x] 整理 SQLite schema
