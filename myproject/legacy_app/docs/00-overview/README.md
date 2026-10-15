# 系统总览

> **性质**：叙述性文档
> **需求权威源**：暂无（叙述类）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：—（目录索引，版本无关）
> **状态**：✅ 已建立（3 页）

## 目的

让不熟悉仓库的人理解系统定位、业务背景与术语。

## 文件清单

- ✅ `what-is-this.md` — 系统定位：基于 SemiDrive D9360 的储能后备管理 App 层
- ✅ `glossary.md` — 术语表：BMU / PCS / CCU / DPR / sampler / reporter / HAL / DAI / RAUC / dm-verity
- ✅ `scope-and-boundaries.md` — App 层与 BSP 的职责边界

## 权威源与依赖

- `openspec/config.yaml`（项目 context）
- `AGENTS.md`

## 待办

- [x] 写 what-is-this（业务背景与系统定位）
- [x] 建立术语表并全仓统一用词
- [x] 明确 App/BSP 边界（参考 app-lifecycle R10、keep-app-alive-on-bmu05 D8）
