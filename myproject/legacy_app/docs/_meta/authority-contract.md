# 权威契约头（每篇 docs 文件必带）

> **性质**：文档维护规范。
> **状态**：🟡 骨架，内容待填充

## 文件头模板

每个 `docs/**/*.md`（`_meta/` 自身除外）**必须包含**如下头部（可位于文件首行，或紧随 H1 标题之后）：

````markdown
> **性质**：〈叙述性文档 / 操作手册 / 接口契约 / 索引〉
> **需求权威源**：`openspec/specs/<capability>/spec.md`（无对应 capability 时写「暂无（架构/操作类，无需求源）」）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`<git short sha>`（最后核对时的代码版本；骨架期为 `—`）
> **状态**：🟡 骨架 / 🟠 部分 / ✅ 已核对
````

> 权威头既可紧贴文件首行出现，也可位于 H1 标题之后；两种位置均符合规范，当前仓库各文件的混用属合规。

## 硬规则

1. **不复制需求断言**：docs 中不得出现 "系统 MUST/SHALL/应…" 形式的验收级断言。需要时链接到 spec 的 `R<n>`。
2. **数字必须可回溯**：模块数 / ID / 行号 / 路径 / 常量，必须在文中给出代码位置（`file:line`）或链接到 `.code-facts-snapshot.md`；并更新头部「对应提交」。
3. **spec 头元数据不可信**：`openspec/specs/*/spec.md` 的 `Version` / `Last Updated` 属结构性不可达区（可能过期）；引用前以代码与快照为准。
4. **冲突以权威源表为准**：见 `docs/README.md` 的「权威源声明」。
5. **不重复 AGENTS.md**：面向 agent 的速查内容留在 `AGENTS.md`，docs 只链接。

## 更新触发条件

| 代码/工程变更 | 需要同步的 docs |
|---|---|
| 启动链 / 脚本 / systemd-init 脚本名 | `10-architecture/boot-chain.md`、`60-operations/lifecycle.md` |
| Makefile `DIRS` / 模块增删 | `20-features/*`、`90-reference/spec-index.md` |
| HAL 函数签名 / 数量 | `30-interfaces/hal-comm.md` |
| SQLite schema / 参数 ID | `30-interfaces/sqlite-schema.md`、`40-configuration/*` |
| 构建 / 签名 / verity / RAUC 流程 | `50-build-deploy/*` |
| 平台变体宏（`-DIMMU3`）或 `shells/mu3` | `70-platforms/*` |
