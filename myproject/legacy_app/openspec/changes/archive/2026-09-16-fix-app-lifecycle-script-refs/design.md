## Context

`openspec/specs/app-lifecycle/spec.md` 的 canonical 文本形成于 2026-07-14（v0.2→v0.3 本轮验证时定型）。此后 BSP 侧发生了一次**脚本改名**，但本仓 spec 未同步，共留下 **9 处** `S91app_mount` 引用。

改名时点可由 build 产物佐证（见 `proposal.md` 的 E4/E5）：`ap1_bmu02_a00` / `ap1_bmu03_a00` / `ap1_bmu05_a00`(ospi2_emmc) 三个旧变体内是 `S91app_mount`；`ap1_bmu05_a00`(emmc_only / ospi2_xip_emmc) 两个新变体内是 `S92app_mount`。

**为何改名**：新脚本 `S91app_sysmgr`（`/usr/sbin/app_sysmgr` —— AP↔R 跨核管理守护，负责 BOOT_READY 握手 / SSDK 版本与槽位同步 / 跨核心跳 / RAUC `MARK_GOOD`）需要占用 S91 槽位并抢在 app 挂载前启动，原 `S91app_mount` 因此让位到 S92。

## Goals / Non-Goals

**Goals**

- 消除 canonical spec 中指向**不存在文件**的 7 处 Requirement 引用（R4 / R6 / R9）
- 保持"最小改动"：仅替换标识符，任何行为语义、Scenario、证据行数都不变
- 使 app-lifecycle spec 内部自洽（不再是"R7 说 S92、R6/R8/R9 说 S91"）

**Non-Goals**

- 不修 BSP 侧的改名残留（`S92app_mount` 自身头注释、`app-post-install` 的 6 处注释、`app-trust/README.md`）—— 那些是 BSP 仓的文件，归主仓 openspec
- 不改任何 C 代码 / shell 脚本
- 不新增 Requirement、不重编号
- 不触碰 R7（理由见 D2）

## Decisions

### D1：只替换标识符，不做任何"顺手改进"

三条 Requirement 的正文、Scenario、表格、脚注**逐字保留**，唯一差异是 `S91app_mount` → `S92app_mount`。理由：本变更的正当性完全来自"文档指向了不存在的文件"，一旦夹带语义修改，评审价值就被稀释，且无法用"逐字对比只差一个标识符"来快速验证。

*替代方案*：顺手把 R6 里写死的 `compatible=bmu02_a00` 也一并改成可变说明。**否决** —— 那是另一类事实（见 R1 脚注已列 `RAUC_COMPATIBLE="bmu02_a00"` 为写死值），且 bmu05 实际用 `RAUC_COMPATIBLE=bmu05_a00` 覆盖，属于独立议题。

### D2：不触碰 R7，避免与 `keep-app-alive-on-bmu05` 双重 MODIFY

R7「运行时启动链」内的 2 处 `S91app_mount`（canonical `:229`、`:246`）**已经**由 `keep-app-alive-on-bmu05` 的 delta 处理：

```
$ grep -c S91app_mount openspec/changes/keep-app-alive-on-bmu05/specs/app-lifecycle/spec.md
0
$ grep -n 'S92app_mount' openspec/changes/keep-app-alive-on-bmu05/specs/app-lifecycle/spec.md
9:- boot：BSP `/etc/init.d/S92app_mount` 的 `start)` 分支，…
```

两个 change 同时 MODIFY 同一 Requirement 不会报错（`specs-apply.js` 按顺序应用到同一个 canonical），但会：
1. 造成**归档顺序耦合** —— 后归档者必须基于前者的结果书写 delta，否则会覆盖掉对方的内容
2. 让 R7 的 delta 出现两份近乎重复的文本

因此本变更**只处理 R4 / R6 / R9**，R7 交给 `keep-app-alive-on-bmu05`。二者归档顺序**无关紧要**（作用于不同 Requirement）。

### D3：Purpose 段的 2 处引用无法经 delta 修正（登记为延后项）

`openspec` 的 delta 机制仅支持四种 Requirement 级操作：

```
dist/core/specs-apply.js:164
  // Extract requirements section and build name->block map
  const parts = extractRequirementsSection(targetContent);
dist/core/specs-apply.js:170
  // Apply operations in order: RENAMED → REMOVED → MODIFIED → ADDED
```

作用域限定在 `## Requirements` 区块内的 `### Requirement:` 块；`## Purpose` 属于 preamble，只被原样保留、**没有任何 delta 操作能写入它**。

而项目约定（`openspec/config.yaml`）写明：

> Canonical specs are updated ONLY via change proposals + "openspec archive" — **never hand-edited**.

因此 canonical 第 18、20 行的 `S91app_mount` **本变更无法修正**：

| 位置 | 原文 |
|---|---|
| `:18` | **阶段三：运行时**（R7-R9）——设备上电后 S91app_mount 挂载分区、app_entry 注册信号、… |
| `:20` | > **范围说明**：dm-verity 的**运行时挂载**（`veritysetup open` / `dmsetup`）由 SDK 侧的 `S91app_mount` 脚本负责，不在本仓库。本 spec R4 仅描述构建期的 hashtree 生成。 |

**三个可选方案**：

| 方案 | 做法 | 评价 |
|---|---|---|
| A | 手工编辑 canonical 的 Purpose 段 | 直接违反 config.yaml 约定；且破坏"canonical 只由 archive 生成"的可追溯性 |
| B | 等下一次 canonical 整体重写时一并处理 | **选用** —— 这两处是纯叙述性文字，不影响任何 Requirement 的可验证性；已登记为 `tasks.md` 的 `<DEFERRED>` 项 |
| C | 用 `## REMOVED Requirements` + `## ADDED Requirements` 重建整份 spec | 无效 —— Purpose 仍不在作用域内；且会让 R1-R9 的 git 历史变成全量重写，损失可追溯性 |

**影响评估**：方案 B 的代价是 canonical 的 Purpose 段会在一段时间内仍称 `S91app_mount`。由于 Purpose 段是**导读性文字**、所有**规范性内容**（R1-R9 的 MUST/Scenario）都已正确，实际误导风险很低；且 `proposal.md` 已明确记录该限制，读者可自行对照。

### D4：证据优先级 —— 真机 > BSP 源码 > 子仓文档

判定"哪个脚本名是真的"用了三类证据。优先级按可信度排序：

1. **真机实测**（`ls /etc/init.d/`）—— 最高，直接回答"设备上有什么"
2. **BSP 源码**（`def_buildroot_rootfs/etc/init.d/`）—— 次之，但注意该目录是**部分 overlay**（缺 `S10udev` / `S25modules` / `S50sshd` 等，也无 `S91app_sysmgr`），**不能单独据此下结论**；本次之所以能定论，是因为它与真机一致
3. **子仓文档**（`source/test_app/AGENTS.md:28`）—— 仅作旁证

*曾走过的弯路*：早期仅凭 BSP 源码目录得出"只有 S92"，该结论**运气好而方法不严谨**（源码目录不完整）。最终定论依赖真机 `ls`。

## Risks / Trade-offs

| 风险 | 等级 | 缓解 |
|---|---|---|
| 归档顺序与 `keep-app-alive-on-bmu05` 耦合 | 低 | 两者作用于不同 Requirement（R7 vs R4/R6/R9），顺序无关；已用 `grep -c S91app_mount` 证明 R7 delta 为零残留 |
| Purpose 段遗留 2 处错误引用 | 低 | 登记为 `<DEFERRED>`，`proposal.md` 已显式披露 |
| BSP 侧改名残留未修 | 中 | 属 BSP 仓，`proposal.md` 已列出 4 处（E2/E3/E6/E7），需在主仓 openspec 另立变更 |
