# 配置与数据

> **性质**：接口契约 / 叙述性文档
> **需求权威源**：`openspec/specs/data-processor/spec.md`（R1 配置管理）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：—（目录索引，版本无关）
> **状态**：✅ 已建立（3 页）

## 目的

说明设备行为如何由 SQLite 配置驱动。

## 文件清单

- ✅ `system-cfg.md` — `System.cfg` 表与加载流程
- ✅ `device-model-and-params.md` — DeviceModel / 参数 ID 模型
- ✅ `i18n.md` — 多语言文案

## 权威源与依赖

- `openspec/specs/data-processor/spec.md`

## 待办

- [x] 记录 System.cfg 主要表与规模（DeviceModel 4157 / 状态量 1958 / 告警 1119 / i18n 5205，以代码为准）
- [x] 说明 Cfg_Parse_* 流程
