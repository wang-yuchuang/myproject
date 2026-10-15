# Spec 索引

> **性质**：索引
> **需求权威源**：`openspec/specs/<capability>/spec.md`
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：—
> **状态**：✅ 已核对（2026-09-17，需求数经 grep 逐个验证）

> ⚠️ **spec 头部的 Version/Last Updated 属 delta 不可达区，可能滞后；内容以 Requirements 与代码为准。**

## 总览

canonical spec 共 **7 个 capability**，Requirement 总数 **74**。需求数为 2026-09-17 以 `grep -c '^### Requirement:'` 对各 spec 实测的结果。

| capability | 版本* | 需求数 | 最后更新* | 核心机制一句话 | 链接 |
|---|---|---|---|---|---|
| samplers | 0.4 | 14 | 2026-07-14 | 10 个采样器各编译为独立 .so，运行时 dlopen + dlsym 动态加载，统一 4 个 DLLExport 接口 | [spec.md](../../openspec/specs/samplers/spec.md) |
| app-lifecycle | 0.3 | 14 | 2026-07-14 | 覆盖构建、部署、运行时三阶段：build.sh 流水线、dm-verity 只读挂载、RAUC OTA 与 boot 启动链 | [spec.md](../../openspec/specs/app-lifecycle/spec.md) |
| comm-ports | 0.4 | 12 | 2026-07-15 | 9 个通信 .so 模块经 6 个 HAL_Comm* 函数提供统一传输抽象，dlsym 动态加载 | [spec.md](../../openspec/specs/comm-ports/spec.md) |
| reporters | 0.3 | 12 | 2026-07-14 | 8 个上报器由统一加载器初始化，按 nPlatformType 互斥跳过，多协议上报云端与运维平台 | [spec.md](../../openspec/specs/reporters/spec.md) |
| publib | 0.3 | 10 | 2026-07-02 | 公共基础库：线程 / 互斥锁 / 队列 / 内存池 / 日志 / SQLite 封装，含 syshand 客户端 SDK | [spec.md](../../openspec/specs/publib/spec.md) |
| data-processor | 0.3 | 7 | 2026-07-14 | DPR 数据中心：SQLite 配置加载、表达式运算、规则判断、事件与告警，系统核心枢纽 | [spec.md](../../openspec/specs/data-processor/spec.md) |
| system-hand | 0.4 | 5 | 2026-07-14 | syshand 守护进程，System V IPC 命令代理，OS 调用抽象层；心跳丢失仅告警不重启 | [spec.md](../../openspec/specs/system-hand/spec.md) |
| **合计** | | **74** | | | |

\* 版本与最后更新两列摘自各 spec 头部，仅作参考，解释权归 Requirements 与代码，理由见顶部警告。

## 数据流视角

```
设备 ──→ samplers ──→ data-processor (DPR) ──→ reporters ──→ 云端 / 运维平台
                        ↑ 配置(System.cfg)          ↑
                        └──── comm-ports 提供传输 ───┘
publib 为以上全部提供基础库；system-hand 与 app-lifecycle 支撑运行环境与部署链
```

## 需求数核对方法

```bash
for f in openspec/specs/*/spec.md; do echo "$(dirname $f | xargs basename): $(grep -c '^### Requirement:' $f)"; done
```

总和应等于 74。若不等，说明某个 spec 增删了 Requirement，本表需同步。

## 与其它文档面的关系

- 需求原文与验收 Scenario 在各 spec.md，本文件不复制断言，只给入口。
- 代码层面的模块数、行号、路径以 `openspec/.code-facts-snapshot.md` 为基线；spec 头部元数据与快照冲突时，以快照为准。
- spec 的演进历史（哪条 Requirement 由哪个变更引入或修正）→ [change-history.md](change-history.md)。
- 各模块编译清单与 Makefile 出处 → AGENTS.md「子系统关键事实」表。

## 更新触发

代码发生模块增删（Makefile `DIRS`）、Requirement 增删或变更归档时，同步本表的需求数与链接。核对日期写入头部状态行。
