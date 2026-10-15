# docs 写作模板

> **性质**：文档维护规范。
> **状态**：🟡 骨架，内容待填充

所有模板均以 `_meta/authority-contract.md` 的「权威契约头」开场。

## A. 叙述性文档（如 architecture / features）

````markdown
# 〈标题〉

## 概述
〈2–5 句，读者能得到什么〉

## 结构 / 流程
〈ASCII 图优先〉

## 关键设计点
- 〈点〉 —— 依据：`<file:line>` 或 spec `R<n>`

## 与其他部分的关系
- 〈子系统〉：〈关系〉 —— 见 `openspec/specs/<cap>/spec.md#R<n>`

## 已知限制
- 〈限制〉
````

## B. 操作手册（runbook，如 operations / build-deploy）

````markdown
# 〈操作名〉

## 前置条件
## 步骤
1. 〈命令〉
   - 预期输出：〈…〉
   - 失败时：〈回退/排查链接〉
## 验证
## 回滚
## 风险与注意
````

## C. 接口契约（interfaces）

````markdown
# 〈接口名〉

## 调用方 / 被调方
## 原型 / 数据格式
## 语义与约束
## 错误处理
## 版本与兼容性
## 代码位置
- 定义：`<file:line>`
- 实现：`<file:line>`
````

## D. 索引类（reference）

````markdown
# 〈索引名〉

| 项 | 说明 | 权威源 |
|---|---|---|
| 〈…〉 | 〈…〉 | [链接](../../openspec/...) |
````
