## Context

`app_sysmgr` 是 bmu05 上 **boot 早期即运行**的 AP↔R 跨核系统管理守护（实测 PID 1003、4 线程、`fd 3-15` 全为 rpmsg、`wchan = hrtimer_nanosleep`）。它的存在此前完全不在本仓库的 spec 视野内 —— `openspec/specs/` 下 grep `app_sysmgr` 为 **0 处**。

它是 2026-09-16 真机调查的副产物发现，当天即三次影响判断（S91 槽位占用 → 脚本改名 → 3 条 Requirement 写错名；v1 曾启动 `app_entry` → 启动责任真空；独立的跨核心跳 → 与 syshand 心跳混淆）。它同时是本仓库 spec **唯一一处**描述「boot 顺序中 S91 是谁」的地方所缺失的对象。

同时必须明确它的**边界**：`app_sysmgr` 是本仓库**不拥有**的 BSP 组件，应用也**不直接与它通信**（应用 ↔ syshand ↔ …）。它与本仓库的相关性**仅在于应用启动链的 boot 契约**。

## Goals / Non-Goals

**Goals**

- 让 spec 能回答三个问题：boot 顺序里 S91 是谁 / 谁负责启动应用 / 跨核心跳与应用心跳有何区别
- 把「本守护 MUST NOT 启动 `app_entry`」写成**显式否定性契约**，防止未来 BSP 把该能力加回时无人察觉
- 提供一份**只读契约**，使 `keep-app-alive-on-bmu05` 的 boot 接线（`S92app_mount`）与 BSP 的能力边界有据可依

**Non-Goals**

- 不复制 BSP 实现（不描述 `app_sysmgr` 的内部线程模型、状态机、协议帧格式）
- 不改任何代码；不要求 BSP 做任何改动
- 不记录 `/data/app_sysmgr.log` 等历史残留文件的运维细节（已进 `.code-facts-snapshot.md`）
- 不为 `app_sysmgr` 单独建 capability（理由见 D1）

## Decisions

### D1：新增 `app-lifecycle` 的 R10，而非新建 `app-sysmgr` capability

| 方案 | 评价 |
|---|---|
| **A. `app-lifecycle` 新增 R10**（选用） | `app_sysmgr` 与本仓库的相关性**只有一条** —— 它在 boot 顺序中的位置及其与 app 启动链的边界。而 `app-lifecycle` 已经在记录同类的 BSP boot 契约（R4 运行时 verity 挂载、R6 RAUC 安装前置、R7 运行时启动链、R9 优雅停止），R10 与它们**同构**。放在一起，读者能在一个文件里读完「boot 时谁做什么」 |
| B. 新建 capability `app-sysmgr` | **否决** —— 会误导性地暗示本仓库拥有该组件。仓库内 daemon 的先例（`system-hand` 描述本仓库的 `syshand`）支持「daemon = 独立 capability」，但 `app_sysmgr` **是本仓不拥有的 BSP 代码**，与 `S92app_mount`（同样 BSP、同样被记入 `app-lifecycle`）一致，故应跟随后者 |
| C. 不记录 | **否决** —— 该空洞已在一天内三次造成实际误判，且无任何其它地方记录它 |

*成本考量*：`app-lifecycle` 已 369 行、R1-R9。追加 R10 使其更长，但换来的是「boot 契约单点可读」；拆成两个 capability 反而要求读者跨文件拼接。

### D2：只记录**可观测行为**，不描述实现

R10 的每条内容都源自**可观测证据**：命令行、`/proc/<pid>/{fd,status,wchan}`、`strings` 抽出的日志串。**不写**内部线程职责划分、rpmsg 消息格式、重试计数器的具体数值（除日志串已暴露的 `5s`）。理由：

- 实现归 BSP，写细了必然随 BSP 演进而漂移，变成负债
- 可观测行为才是**本仓库可验证**的部分（`openspec/config.yaml` 约定：Cite file:line evidence for every documented behavior）

*例外*：日志串**原样保留**（`MARK_GOOD ok (slot %c GPT retry reset)` 等）。它们是唯一可用来在真机上断言行为的抓手，删去会让 Requirement 不可验证。

### D3：证据优先级 —— 真机 > 源码目录

本变更的证据全部来自**真机**。此处记录一条方法教训，因为它直接影响本 spec 的可信度：

⚠️ 早期曾仅凭 BSP 源码目录 `def_buildroot_rootfs/etc/init.d/` 判定「只有 `S92app_mount`」，但该目录是**部分 overlay** —— 缺 `S10udev` / `S25modules` / `S50sshd` 等真机上存在的脚本，**也没有 `S91app_sysmgr`**。若当时据此下结论会得到相反答案。

→ 因此 R10 的所有事实均以真机为准；**源码目录不得单独作为结论依据**。此教训同样适用于 `S91app_mount` 改名判定（已记录于 `fix-app-lifecycle-script-refs` 的 D4）。

### D4：把否定性契约写进 Requirement，而非只写进文档

`R10` 显式包含：

> - 本守护 **MUST NOT** 承担启动 `/app/app_entry` 的职责
> - 应用启动 MUST 由 `S92app_mount` 的 boot 路径或 RAUC `app-post-install` 的 OTA 路径负责
> - 跨核心跳与应用心跳是**两套独立机制**，MUST NOT 混用

理由：`keep-app-alive-on-bmu05` D8 已证实 **v1 曾经就是这样做的**，且该能力是被「顺便删掉」而非被明确取消的。若只把边界写在散文里，BSP 某次恢复 v1 行为就会静默产生**两个启动者**（`app_entry` 的 `pidof nccu_app` 幂等守卫是唯一防线）。写进 Requirement 后，该边界成为**可评审、可回归**的契约。

## Risks / Trade-offs

| 风险 | 等级 | 缓解 |
|---|---|---|
| `app_sysmgr` 是 BSP 组件，本仓 spec 记录它可能「越界」 | 低 | 已在 Requirement 首段显式声明**只读契约**（实现归 BSP 仓），并限定记录范围为「可观测行为 + 与应用启动链的边界」 |
| 日志串随 BSP 版本变化 → Requirement 的 Scenario 失效 | 中 | 这是**设计意图**：日志串变化应触发 spec 更新。已加入 `.code-facts-snapshot.md` 的漂移检查清单（「`/etc/init.d/` 下 app 相关脚本名」「`app_sysmgr` 是否重新具备启动能力」） |
| R10 记录的 v1→v2 历史事实可能被误读为「要求回退 v1」 | 低 | 已标注「历史事实，供追溯」，且同一 Requirement 的否定性契约明确方向相反 |
| 与 `keep-app-alive-on-bmu05` 的 R7 内容重叠（都涉及「谁启动 app」） | 低 | 分工明确：R7 描述**启动链本身**，R10 描述**本守护不参与该链**。两者可独立归档，无 MODIFY 冲突 |
