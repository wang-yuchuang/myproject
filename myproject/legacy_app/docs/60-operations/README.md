# 运维手册

> **性质**：操作手册
> **需求权威源**：`openspec/specs/app-lifecycle/spec.md`、`openspec/specs/system-hand/spec.md`
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：—（目录索引，版本无关）
> **状态**：✅ 已建立（4 页）

## 目的

现场运维：启停、日志、升级、排障与恢复。

## 文件清单

- ✅ `lifecycle.md` — 启停（app_entry / startrun / stoprun）
- ✅ `logs-and-db.md` — 日志与运行时 DB
- ✅ `troubleshooting.md` — 常见故障
- ✅ `recovery.md` — 掉电 / slot 恢复

## 权威源与依赖

- `openspec/specs/app-lifecycle/spec.md`
- `openspec/specs/system-hand/spec.md`

## 待办

- [x] 记录停止链当前不可达的事实（app-lifecycle R9）与现场替代做法
- [x] 记录 syshand 心跳仅记录不重启的语义
- [x] 升级/回滚 runbook
