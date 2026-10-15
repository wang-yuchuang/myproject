## Context

本次修正源于一次**我自己的误判再纠正**，过程本身值得记录。

最初我用 `readlink -f /opt/appdata`（返回自身）与 `df`（两者同为 `/dev/mmcblk0p45`）判定「`/opt/appdata` 与 `/data` 是同一分区上的两个无关目录」。**这个判断方法有缺陷**：`readlink -f` 检测不出 bind mount，而 `df` 输出「同设备、同 180M 占用」其实已经提示同源，我误读为巧合。

后经 `/proc/mounts` + `S00data_mount` 源码确认真实拓扑：

```
/data         ← mount /dev/mmcblk0p45                  (USERDATA_POINT="/data")
/opt/appdata  ← mount --bind /data/rootfs/opt/appdata  (BIND_MOUNTS: opt/appdata|/opt/appdata)
```

**但**即使拓扑澄清了，"路径写错"的结论依然成立 —— 因为 bind mount 只说明**分区归属**，不改变**目录路径**。`/opt/appdata/data/nccu.data` 与 `/data/nccu.data` 是两个不同目录下的不同文件，`stat` 已证前者不存在。

## Goals / Non-Goals

**Goals**

- 消除「读者按 spec 去 `/opt/appdata/data/` 找 `nccu.data` 却找不到」这类**照着文档做必失败**的描述
- 把「DB 文件」与「其锁/索引」两类文件的位置**显式分开**，因为二者确实不同目录、极易混淆
- 让 `app-lifecycle` R8 与 `publib` R2 对 DB 路径的表述**互相一致**

**Non-Goals**

- 不改任何代码（代码行为正确，是文档错了）
- 不重构 R2/R8 的其它内容；不新增 Requirement；不重编号
- 不把「挂载拓扑」本身写进 spec —— 那属事实基线，归 `openspec/.code-facts-snapshot.md`（已记录）
- 不修 `app-lifecycle` Purpose 段的 2 处 `S91app_mount`（delta 机制无法覆盖，已在 `fix-app-lifecycle-script-refs` 登记为 `<DEFERRED>`）

## Decisions

### D1：路径表述用「`$NCCU_DATA_PATH` + 运行时值」，而不是简单写 `/data`

三种写法的比较：

| 写法 | 评价 |
|---|---|
| `/opt/appdata/data/nccu.data`（原文） | **错** —— 该文件不存在于该目录 |
| 直接写 `/data/nccu.data` | 对当前所有变体都对，但**丢失了"由环境变量驱动"这一机制**，未来若引入变体覆盖会再次漂移 |
| **`$NCCU_DATA_PATH/nccu.data`（运行时 `/data/…`）**（选用） | 既表达机制（E1/E2），又给出可核验的运行时值（E3/E4）；与 `app-lifecycle` R7 已有的 `NCCU_DATA_PATH=/data` 表述呼应 |

### D2：delta 用「逐行提取 + 精确替换」生成，而非人工重抄

MODIFIED 是**整块替换** —— delta 里 R8 的 60 行、R2 的 69 行会被逐字写回 canonical。任何抄写偏差都会**静默污染** canonical（丢一行、多一行、全角/半角、空白差异）。

因此本变更的 delta **不由人工重抄**，而是：
1. 从 canonical 精确提取目标行区间（`sed -n '280,339p'` / `sed -n '58,126p'`）
2. 对指定的 3 行做**精确字符串替换**
3. 用 `diff <提取块> <最终delta>` 自检 —— **必须只显示这 3 处变更**

验收再加一道独立闸门：归档后 `git diff openspec/specs/*/spec.md` **必须只显示这 3 行的行内变更**（允许 `openspec` 自身的空行规范化，判据见已归档变更 `fix-app-lifecycle-script-refs` 的 tasks.md 3.7）。

### D3：把方法教训写进事实基线，而不是只写结论

「`readlink -f` 检测不出 bind mount」这条已写入 `openspec/.code-facts-snapshot.md` 的 `### 运行时可写路径与 DB/锁文件`。理由：这类错误**能通过审查**（结论看起来合理），只有记录方法才能防止复发。同类先例：`fix-app-lifecycle-script-refs` D4 记录的「BSP 源码目录是部分 overlay，不能单独作为结论依据」。

### D4：本变更应在 `keep-app-alive-on-bmu05` 归档之前完成

第 3 处（就绪等待值）的漂移来源是今天的代码提交 `c8906429`。若 `keep-app-alive-on-bmu05` 先归档，其 R7 delta 会把 canonical 的启动链描述更新为新代码，而 **R8 步骤 12 仍写着旧值** → 同一份 spec 内部再次自相矛盾。两者作用于**不同 Requirement**（R7 vs R8），无 MODIFY 冲突，但**顺序有实质影响**。

## Risks / Trade-offs

| 风险 | 等级 | 缓解 |
|---|---|---|
| MODIFIED 整块替换时抄写偏差静默污染 canonical | **中** | D2 的「提取+替换+自检」+ 归档后 `git diff` 只应含 3 行变更；不符则 `git checkout` 回退重做 |
| 只改文本不改代码 → 有人误以为要改代码 | 低 | `proposal.md` 与本节均明确「代码行为正确，是文档错了」 |
| `NCCU_DATA_PATH` 未来被改成其它值 → 新表述又"过时" | 低 | 新表述本来就把变量作为主语，变量改名属机制变更，理应触发 spec 更新（这正是期望行为） |
| 与 `keep-app-alive-on-bmu05` 的归档顺序 | 低 | D4 已说明：无冲突但顺序有影响，建议本变更先行 |
