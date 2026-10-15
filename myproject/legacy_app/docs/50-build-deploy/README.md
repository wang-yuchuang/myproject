# 构建与交付

> **性质**：操作手册
> **需求权威源**：`openspec/specs/app-lifecycle/spec.md`（R6/R7/R11）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：—（目录索引，版本无关）
> **状态**：✅ 已建立（5 页）

## 目的

从源码到可部署产物的完整流程。

## 文件清单

- ✅ `build.md` — `./build.sh` 三档
- ✅ `verity-and-signing.md` — ext4 / dm-verity（纯 Python）/ 签名
- ✅ `rauc-ota.md` — RAUC bundle 生成与安装
- ✅ `factory-flashing.md` — 工厂烧录 app.img
- ✅ `rauc-compatible.md` — `RAUC_COMPATIBLE` 与设备端一致性问题

## 权威源与依赖

- `openspec/specs/app-lifecycle/spec.md`

## 待办

- [x] 迁移根 README 的有效内容（RAUC compatible）并废弃陈旧部分
- [x] 记录 256MB 分区与 D9APPMD1 magic 校验
