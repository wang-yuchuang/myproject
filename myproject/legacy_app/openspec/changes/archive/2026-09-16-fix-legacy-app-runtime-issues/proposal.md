## Why

legacy_app 通过 RAUC OTA 升级到设备后，`syshand` 能启动但 `nccu_app` 启动后立即退出，核心业务无法运行。排查发现以下多个问题需要修复：

1. **mu3/run_app 未同步修复**：`MU_TYPE=mu3`，构建时实际使用的是 `src/shells/mu3/run_app` 而非 `src/shells/run_app`，但 mu3 版本的 LD_LIBRARY_PATH 未添加 `/app/extensions`、仍包含 `cp -f` 到 `/usr/lib` 的命令、且仍用 PID 文件检查判断 syshand 是否运行（而非进程检查）
2. **shmget IPC_EXCL 冲突**：syshand 的 `shmget(key, size, IPC_CREAT | IPC_EXCL)` 在非首次启动时会失败（SHM 段已存在），导致 `nccu_app` 判断 syshand 未就绪而退出
3. **RAUC compatible 版本号错误**：`build.sh` 写死 `RAUC_COMPATIBLE="bmu02_a00"`，但 D9 SDK 当前项目是 `bmu05_a00`，RAUC 安装时 compatible 不匹配可能导致拒绝安装
4. **strace 调试残留**：`run_app` 源码中 `nccu_app` 通过 `strace -o $TRACE_LOG ...` 启动，这是调试残留，生产环境应直接启动
5. **log4crc 拷贝容错**：`cp -f /app/extensions/log4crc /etc/log4crc` 在 bind mount 未就绪时需要静默容错

## What Changes

### mu3 启动脚本同步
- 将 `src/shells/run_app` 的所有修复同步到 `src/shells/mu3/run_app`：
  - LD_LIBRARY_PATH 添加 `/app/extensions`
  - 移除 `cp -f` 到 `/usr/lib/` 的命令
  - PID 文件检查改为 `ps | grep '[s]yshand'` 进程检查
  - 添加 TRACE_LOG 变量和 /opt/appdata 目录创建
  - log4crc 拷贝添加容错 `2>/dev/null || true`
  - nccu_app 启动去掉 strace 包裹

### shmget 容错
- `handler.c` 的 `shmget` 从 `IPC_CREAT | IPC_EXCL` 改为 `IPC_CREAT`（允许复用已有的 SHM 段）
- `startup_fbshow/main.c` 同步修改

### RAUC compatible 版本号
- `build.sh` 的 `RAUC_COMPATIBLE` 从写死的 `bmu02_a00` 改为从环境变量或 `.project.conf` 读取

## Capabilities

### Modified Capabilities

- `legacy-app-boot`: mu3 启动脚本同步修复，shmget 容错，strace 残留清理
- `app-build-entry`: RAUC compatible 版本号可配置

## Impact

- **src/shells/mu3/run_app**: 全面同步 src/shells/run_app 的修复
- **src/utilities/system_hand/handler.c**: shmget 参数调整（IPC_EXCL → 无 IPC_EXCL）
- **src/utilities/start_fbshow/standard/main.c**: 同步 shmget 调整
- **build.sh**: RAUC_COMPATIBLE 可配置
- **src/shells/run_app**: 去除 strace 调试包裹
- **风险**: shmget 改为 IPC_CREAT 后，如果 SHM 段残留旧数据可能导致读取脏数据，需要在 attach 后 memset 清零

## 调试过程发现的新问题

### 6. USB/SD 设备检测误判内部 eMMC
`startrun` 和 `mu3/startrun` 用 `ls /dev/mmc*` 检测 SD 卡插入，但 D9360 内部 eMMC（`/dev/mmcblk0`）也被匹配到，导致脚本误判为有 SD 卡，进入升级流程而非启动 nccu_app。
- **修复**: `ls /dev/mmc*` → `ls /dev/mmcblk[1-9]*`，排除内部 eMMC
- **涉及**: `src/shells/startrun`, `src/shells/mu3/startrun`

### 7. log4crc 路径在 RO 文件系统不可写
`/etc` 在 D9360 上为 RO 文件系统，`cp log4crc /etc/log4crc` 静默失败，`log4c_init()` 找不到配置返回 FALSE，nccu_app 直接退出。
- **修复**: 优先拷贝到 `/opt/appdata/config/log4crc`（可写），兜底 `/etc`
- **涉及**: `src/shells/*run_app*` 共 4 个文件, `src/publib/iLog4c.c`

### 8. startrun else 分支启动 app_update 闪屏误报
非 mu3 版 `startrun` 的 else 分支（无 USB/SD 设备时）调用 `app_update -nomouse &` 作为闪屏，但 `app_update` 是升级脚本，忽略 QT 参数后执行升级扫描，找不到升级包时报 `CRITICAL ERROR`。
- **修复**: 移除 `app_update -nomouse` 调用，改为纯等待
- **涉及**: `src/shells/startrun`

### 9. syshand.run 标记文件缺失
当 syshand 已在运行时（进程检查检测到），run_app 跳过启动新 syshand，但 syshand.run 标记文件可能不存在（旧 syshand 未创建或分区切换后丢失），导致 nccu_app 等待超时退出。
- **修复**: run_app 检测到 syshand 已在运行时，补充创建 syshand.run
- **涉及**: `src/shells/mu3/run_app`, `src/shells/run_app`

### 10. libnopoll.so SONAME 版本链接缺失
`libcomm_ws_ex.so` 的 NEEDED 是 `libnopoll.so.0`（SONAME），但部署时只拷贝了 `libnopoll.so`，没有 `.0` 版本链接，dlopen 找不到依赖库。
- **修复**: `src/extensions/Makefile` 中安装后创建 `libnopoll.so.0 -> libnopoll.so` 符号链接
- **涉及**: `src/extensions/Makefile`

### 11. sampler_ccu.so 脏编译导致符号缺失
`sampler_ccu.so` 链接时未包含 `ccu_com_232.o`，导致 `CCU_Wait_ACCU_Reply_RS232` 符号 undefined，nccu_app 事件循环处理到第 4 个事件时动态链接器报错终止进程。
- **修复**: 清理重建（`rm ccu_com_232.o && rebuild`）
- **根因**: 构建系统依赖跟踪不完整，脏 `.o` 未被重新编译
- **涉及**: `src/moduals/samplers/ccu_sampler/Makefile`

## Verification Results

### 硬件验证（D9360 开发板）

| 验证项 | 结果 | 说明 |
|--------|:----:|------|
| 构建通过，app.img + RAUC bundle | ✅ | build.sh 零错误完成 |
| RAUC compatible 默认值 | ✅ | `compatible=bmu02_a00` |
| RAUC compatible 环境变量覆盖 | ✅ | `RAUC_COMPATIBLE=bmu05_a00` 生效 |
| syshand 启动无 shmget ERRRRR | ✅ | IPC_EXCL 已移除 |
| nccu_app 启动并持续运行 | ✅ | 已验证运行 70+ 秒，主循环迭代 700+ 次 |
| 无 /usr/lib cp 错误 | ✅ | LD_LIBRARY_PATH 替代 |
| 无 /etc/log4crc cp 错误 | ✅ | 路径已改为 /opt/appdata/config |
| USB/SD 不误判（无外部设备时） | ✅ | mmcblk[1-9]* 排除 mmcblk0 |
| libnopoll.so.0 dlopen | ✅ | SONAME 链接已创建 |
| sampler_ccu.so 符号解析 | ✅ | CCU_Wait_ACCU_Reply_RS232 已定义 |

## 经验教训

### 1. 嵌入式系统调试方法论

```
问题表象 → 逐一排除 → 精确定位 → 最小修复 → 验证
  (nccu_app     (启动脚本  (逐步骤加    (每处只改    (硬件验证      
   退出)        →shmget    [BOOT]      必要行)      确认修复)
               →log4crc   追踪日志)
               →USB/SD
               →nopoll
               →sampler)
```

每一个阻塞点都通过 `[BOOT]` 标记精确定位，避免盲目修改。

### 2. 启动链依赖关系

nccu_app 的启动是一整条依赖链，任何一个环节断裂都会导致启动失败：

```
app_entry → startrun → run_app → syshand(IPC) → nccu_app → .so模块加载 → 事件循环
                             ↓                       ↓           ↓              ↓
                         USB/SD检测             shmget容错    libnopoll.so    ThreadMgr
                          syshand.run           log4crc路径   sampler_ccu    心跳/看门狗
```

### 3. 构建系统健壮性

- **SONAME 管理**: 共享库的 SONAME 与实际部署文件名必须一致，否则 dlopen 失败
- **脏编译**: Makefile 依赖跟踪不完整导致 `.o` 未重编、符号缺失，手动 `make clean` 或删除 `.o` 可解决
- **跨架构**: `.o` 文件是架构相关的，在不同架构间复用会导致链接错误

### 4. 调试工具

- **串口 vs SSH**: SSH 执行长时间命令（如 `rauc install`）容易超时，串口更可靠
- **MCP 工具**: bmu05-debug 的 sftp 上传和 serial 读写对远程调试非常有效
- **stdout 缓冲**: printf 到文件是全缓冲的，需要 `fflush(stdout)` 确保日志及时输出
- **dlerror()**: dlopen 失败后必须立即调用 dlerror() 获取错误信息，第二次调用返回 NULL

### 5. 本次调试的关键决策

| 决策 | 理由 |
|------|------|
| 先禁用看门狗和心跳重启 | 避免调试过程中设备反复重启 |
| 全部归入一个变更提案 | 所有修复指向同一目标（nccu_app 启动） |
| 使用 [BOOT] 标记逐级追踪 | 精确定位每个阻塞点，避免猜测 |
| 优先使用串口调试 | SSH 在长时间命令时不稳定 |

---

## ⚠️ 归档前修订（2026-09-16）：delta 被重写并胝 8 条 → 3 条

本变更于 2026-09-16 归档前，其 spec delta **被重写**。原因：原 delta 写于本仓 OpenSpec 化之前，指向**不存在**的 capability `legacy-app-runtime-fix`，且 8 条 Requirement 中大部分已被后续 canonical spec 吸收。

### 原 delta 逐条处置（位于 `specs/legacy-app-runtime-fix/spec.md`，`## MODIFIED`）

| # | 原 Requirement | canonical 覆盖 | 处置 |
|---|---|---|---|
| 1 | `mu3 startup script synchronized` | ❌ | **保留** → `app-lifecycle` **R12** |
| 2 | `shmget tolerant of existing segment` | ✅ `system-hand`（`:152/:173/:190` 均无 `IPC_EXCL`） | 删除 |
| 3 | `nccu_app launched without strace` | ⚠️ 半：`app-lifecycle` R7 `:278` 有正确启动命令，但未点明「不得用 `strace` 包裹」 | **保留** → **R13**（作为回归守卫） |
| 4 | `RAUC compatible configurable` | ✅ `app-lifecycle` R1 脚注已列此写死值 | 删除 |
| 5 | `SD card detection excludes internal eMMC` | ✅ `app-lifecycle` R7 `:252/:256`（连 2026-07 修正都写了） | 删除 |
| 6 | `log4crc copied to writable path` | ✅ `app-lifecycle` R7 `:237/:264` + `publib` `:102` | 删除 |
| 7 | `syshand.run marker file ensured` | ✅ `app-lifecycle` R7 `:269` | 删除（已作为 R12 的附属 Scenario 保留一句交叉引用） |
| 8 | `libnopoll SONAME compatibility` | ❌（specs 中 `nopoll`/`SONAME`/`.so.0` 命中 **0**） | **保留** → **R14** |

### 修订说明

1. **`## MODIFIED` → `## ADDED`**：宿主 `legacy-app-runtime-fix` 在 `openspec/specs/` 下不存在。`specs-apply.js:145-148` 明确规定「target spec does not exist; only ADDED requirements are allowed for new specs」。若照原样归档会**直接报错**
2. **宿主改为 `app-lifecycle`**：R12-R14 接在 R11（`decouple-from-sdk-and-ro-rootfs-adapt`）之后
3. **补 `R<n>` 前缀**：原 delta 全为英文裸名，违反 `openspec/config.yaml:17` 约定
4. **修掉原 delta 的自身缺陷**：其 `RAUC compatible configurable` 条目内有一段 Scenario（`override compatible`）**被整块重复两遍**；该条目已整体删除，缺陷随之消除
5. **本 Proposal 正文的 `Capabilities` 段（写 `legacy-app-boot` + `app-build-entry`）保持原样**作为历史记录；实际生效的 delta 见上表

### 归档顺序约束

**必须在 `decouple-from-sdk-and-ro-rootfs-adapt` 之后归档**（它先占用 R11），否则 `app-lifecycle` 的 Requirement 编号会乱序。