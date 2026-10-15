# 架构

> **性质**：叙述性文档（图为主）
> **需求权威源**：`openspec/specs/app-lifecycle/spec.md`、`openspec/specs/publib/spec.md`
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：—（目录索引，版本无关）
> **状态**：✅ 已建立（6 页）

## 目的

用图说明进程/线程模型、启动链、数据流与运行时布局。

## 文件清单

- ✅ `process-model.md` — 双进程模型（syshand + nccu_app）
- ✅ `boot-chain.md` — 上电→挂载→启动→就绪 的时序
- ✅ `data-flow.md` — samplers → DPR → reporters；comm_ports 位置
- ✅ `runtime-layout.md` — `/app`、`/opt/appdata`、`/data`、`/var/volatile` 与 PID/锁/日志
- ✅ `ota-and-ab.md` — A/B slot、dm-verity、RAUC 生命周期
- ✅ `module-topology.md` — 模块拓扑：构建/链接依赖与加载器（.so 动态库）

## 权威源与依赖

- `openspec/specs/app-lifecycle/spec.md`
- `openspec/specs/publib/spec.md`

## 待办

- [x] 画启动链时序图（标注 BSP app_sysmgr 与 S92app_mount 的边界）
- [x] 画数据流图
- [x] 记录运行时目录与 DB/锁文件布局（对齐快照 2026-09-16 取证）
