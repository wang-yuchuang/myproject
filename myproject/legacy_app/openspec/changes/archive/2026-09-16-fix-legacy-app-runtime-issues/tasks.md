## 1. mu3 启动脚本同步

- [x] 1.1 将 src/shells/mu3/run_app 的 LD_LIBRARY_PATH 添加 `:$NCCU_RUN_PATH/extensions`
- [x] 1.2 移除 src/shells/mu3/run_app 中所有 `cp -f /app/extensions/lib*.so /usr/lib/` 命令
- [x] 1.3 将 src/shells/mu3/run_app 的 syshand 检查从 `if ! [ -f $SYSHAND_PID_FILE ]` 改为 `if ps | grep -q '[s]yshand'`
- [x] 1.4 将 src/shells/mu3/run_app 的 log4crc 拷贝添加容错 `2>/dev/null || true`
- [x] 1.5 src/shells/mu3/run_app 中 nccu_app 启动去掉 strace 包裹，改为 `2>>$TRACE_LOG &`
- [x] 1.6 同步修复 src/shells/mu3/run_app.debug、src/shells/run_app.debug

## 2. shmget 容错

- [x] 2.1 src/utilities/system_hand/handler.c: `shmget` 参数从 `IPC_CREAT | IPC_EXCL` 改为 `IPC_CREAT`，成功后 memset 清零
- [x] 2.2 src/utilities/start_fbshow/standard/main.c: 同步修改 shmget 参数

## 3. RAUC compatible 可配置

- [x] 3.1 build.sh: `RAUC_COMPATIBLE` 改为 `${RAUC_COMPATIBLE:-bmu02_a00}`

## 4. strace 残留清理

- [x] 4.1 src/shells/run_app: `strace -o $TRACE_LOG -e openat,stat ./bin/nccu_app` 改为 `./bin/nccu_app 2>>$TRACE_LOG &`
- [x] 4.2 src/shells/mu3/run_app: 同步修改（如尚未通过 1.5 完成）

## 5. 验证

- [x] 5.1 构建通过，app.img + RAUC bundle 正常生成 ✅
- [x] 5.2 OTA 升级后 syshand 启动不报 shmget ERRRRR ✅
- [x] 5.3 nccu_app 启动并持续运行（70+秒，主循环700+次）✅
- [x] 5.4 无 /usr/lib cp 错误日志 ✅
- [x] 5.5 RAUC compatible 环境变量覆盖生效 ✅

## 6. extensions/install.sh 库拷贝修复

- [x] 6.1 将 src/extensions/install.sh 中 L274-L279 的 `cp -f` 到 `/usr/lib/` 改为通过 LD_LIBRARY_PATH 加载（已注释）
- [x] 6.2 `/etc/log4crc` 的 `cp -f` 容错（install.sh 中不存在 log4crc cp；run_app 中已修复）

## 7. 调试过程中发现的新问题修复

- [x] 7.1 USB/SD 检测排除内部 eMMC（startrun + mu3/startrun）✅
- [x] 7.2 log4crc 路径改为 /opt/appdata/config（4 个 run_app + iLog4c.c）✅
- [x] 7.3 startrun else 分支移除 app_update 闪屏调用 ✅
- [x] 7.4 syshand.run 文件自动补齐（run_app + mu3/run_app）✅
- [x] 7.5 libnopoll.so.0 SONAME 链接（extensions/Makefile）✅
- [x] 7.6 sampler_ccu.so 脏编译修复（ccu_com_232.o 清理重建）✅
- [x] 7.7 nccu_app stdout 也重定向到 trace.log ✅
- [x] 7.8 启动链全程添加 [BOOT] 追踪标记 ✅

## 8. 验证（硬件验证结果）

- [x] 8.1 构建通过，app.img + RAUC bundle 正常生成 ✅
- [x] 8.2 syshand 启动无 shmget ERRRRR ✅
- [x] 8.3 nccu_app 启动并持续运行（PID 1451，70+秒稳定）✅
- [x] 8.4 无 /usr/lib cp 错误日志 ✅
- [x] 8.5 无 /etc/log4crc cp 错误日志 ✅
- [x] 8.6 RAUC compatible 环境变量覆盖生效 ✅
- [ ] 8.7 恢复看门狗/心跳重启机制（调试禁用临时措施）

## 9. 归档前修订（2026-09-16）：delta 重写

- [x] 9.1 删除原 delta `specs/legacy-app-runtime-fix/spec.md`（`## MODIFIED` 指向不存在的 capability —— 照原样归档会报错）
- [x] 9.2 新建 delta `specs/app-lifecycle/spec.md`：`## ADDED Requirements` + **R12 / R13 / R14**（8 条 → 3 条，详见 `proposal.md` 的归档前修订段）
- [x] 9.3 删除已被覆盖的 5 条（shmget 容错 / RAUC compatible 可配 / SD 卡检测排除 eMMC / log4crc 可写路径 / syshand.run 标记）
- [x] 9.4 顺带消除原 delta 的自身缺陷：`RAUC compatible configurable` 条目内有一段 Scenario 被整块重复两遍（该条目已删除）
- [x] 9.5 补 `R<n>` 前缀，宿主改为 `app-lifecycle`

## 10. 归档验证（**必须在 `decouple-from-sdk-and-ro-rootfs-adapt` 之后**）

- [ ] 10.1 `openspec validate fix-legacy-app-runtime-issues` 通过
- [ ] 10.2 归档**前**基线：`grep -c '^### Requirement' openspec/specs/app-lifecycle/spec.md` → 预期 **11**（R1-R11，已含 decouple 的 R11）
- [ ] 10.3 `openspec archive fix-legacy-app-runtime-issues -y` → 预期 `app-lifecycle: update`、`+ 3`（三条 ADDED）
- [ ] 10.4 归档**后**复验：同一 grep → **14**（R1-R14）；`grep -n 'Requirement: R1[234]'` 三条均命中
- [ ] 10.5 **转录完整性检查**：`git diff openspec/specs/app-lifecycle/spec.md` 应**只显示 R12-R14 三块的纯追加**（在 R11 之后），且 R1-R11 无任何行内改动。允许 `openspec` 自身的空行规范化；若出现预期外的**行内**改动，必须 `git checkout --` 回退后重做
- [ ] 10.6 复核 `#### Scenario` 总数增加 = 5（R12 两条 + R13 一条 + R14 一条）

## 11. 待办

- [ ] 11.1 `<DEFERRED>` 任务 8.7（恢复看门狗/心跳重启机制）**永不执行** —— 已由 `keep-app-alive-on-bmu05` D6 判定 superseded（D9360 无 `/dev/wd`，且方向与「应用故障不重启整机」冲突）。保留为历史状态，**不得**因「未完成」而重开