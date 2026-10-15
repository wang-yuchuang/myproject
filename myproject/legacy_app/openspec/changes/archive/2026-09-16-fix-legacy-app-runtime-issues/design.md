## Context

### mu3 启动脚本与主脚本不同步

`build.sh` 设置 `MU_TYPE="mu3"`，构建时 `src/shells/mu3/run_app` 被 install到 `out/run_app`。但此前的修复（路径迁移、LD_LIBRARY_PATH、cp 命令移除）只改了 `src/shells/run_app`，mu3 版本未同步：

| 修复项 | src/shells/run_app | src/shells/mu3/run_app |
|--------|:------------------:|:---------------------:|
| LD_LIBRARY_PATH 添加 extensions | ✅ | ❌ |
| 移除 cp 到 /usr/lib | ✅ | ❌ |
| PID 文件检查改为进程检查 | ✅ | ❌ |
| TRACE_LOG 变量 | ✅ | ❌ |
| /opt/appdata 目录创建 | ✅ | ✅ (已迁移) |
| log4crc 拷贝容错 | ✅ | ❌ |

### shmget IPC_EXCL 冲突

`handler.c` 中 syshand 初始化共享内存：

```c
#define _KEY_T_SYSHAND  (ftok("/app",15))
shmid_cmd = shmget(keySys, CMD_MEM_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
if(shmid_cmd == -1) {
    printf("\n\n[sysHandler]:shmget ini ERRRRR!!!");
    return FALSE;  // ← 返回失败，syshand 初始化失败
}
```

`IPC_EXCL` 标志要求 SHM 段必须不存在。但以下场景会导致失败：
1. OTA 升级后重启，旧的 SHM 段未释放（kernel IPC namespace 未完全清理）
2. syshand 崩溃重启，自身创建的 SHM 段仍然存在
3. app 分区卸载/重新挂载后，ftok key 可能不变但 SHM 段残留

syshand 返回 FALSE 后，`nccu_app` 检查 syshand 状态发现未就绪，直接退出。

### RAUC compatible 版本号

```bash
# build.sh 当前写死
RAUC_COMPATIBLE="bmu02_a00"
```

D9 SDK 当前项目是 `bmu05_a00`，设备端 `/etc/rauc/system.conf` 中 `compatible=bmu02_a00`。虽然目前能匹配（因为 rootfs 也用 bmu02），但长期来看版本号需要统一管理。

### strace 调试残留

`src/shells/run_app` 的 nccu_app 启动行：

```bash
strace -o $TRACE_LOG -e openat,stat ./bin/nccu_app -pid=$APP_PID_FILE &
```

`strace` 会显著降低性能，且 `strace` 可能不在 target rootfs 中（需额外安装），这是调试残留。

## Decisions

### Decision 1: mu3/run_app 完全从 src/shells/run_app 复制

**方案**：将 mu3/run_app 的内容完全替换为 src/shells/run_app 的内容（已含所有修复），保留 mu3 特有的注释差异。

**理由**：两个文件功能完全相同，差异仅为历史遗留。不再维护两份副本。

### Decision 2: shmget 改为 IPC_CREAT（无 IPC_EXCL）

**方案**：
```c
// 修改前
shmid_cmd = shmget(keySys, CMD_MEM_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
// 修改后
shmid_cmd = shmget(keySys, CMD_MEM_SIZE, 0666 | IPC_CREAT);
if(shmid_cmd != -1) {
    char *p = shmat(shmid_cmd, NULL, 0);
    if(p != (char*)-1) { memset(p, 0, CMD_MEM_SIZE); shmdt(p); }
}
```

**理由**：syshand 是首个使用此 SHM 段的进程，IPC_EXCL 是为了检测重复启动。但 S91app_mount 已在启动前清理 PID 文件，且 syshand 自身有进程检查逻辑，IPC_EXCL 变得多余且有害。attach 后 memset 清零确保数据干净。

### Decision 3: RAUC_COMPATIBLE 从环境变量读取

**方案**：
```bash
# build.sh
RAUC_COMPATIBLE="${RAUC_COMPATIBLE:-bmu02_a00}"
```

**理由**：默认保持 bmu02_a00 向后兼容，可通过环境变量覆盖。

### Decision 4: 去除 strace 包裹

**方案**：
```bash
# 修改前
strace -o $TRACE_LOG -e openat,stat ./bin/nccu_app -pid=$APP_PID_FILE &
# 修改后
./bin/nccu_app -pid=$APP_PID_FILE 2>>$TRACE_LOG &
```

**理由**：生产环境不需要 strace 的系统调用追踪。stderr 重定向到 TRACE_LOG 保留错误日志能力。

### Decision 5: SD 卡检测排除内部 eMMC

**方案**：`ls /dev/mmc*` → `ls /dev/mmcblk[1-9]*`

**理由**：D9360 内部 eMMC 为 `/dev/mmcblk0`，`mmc*` 通配符匹配到它。外部 SD 卡通常注册为 `mmcblk1+`。此模式与 `app_update` 脚本一致。

### Decision 6: log4crc 路径改为可写目录

**方案**：
1. `run_app` 脚本优先拷贝 log4crc 到 `/opt/appdata/config/`（可写），兜底 `/etc`
2. `set_log4c_envirment()` 优先设 `LOG4C_RCPATH=/opt/appdata/config`，兜底 `/etc`

**理由**：`/etc` 在 D9360 rootfs 上可能是 RO 文件系统，拷贝静默失败。`/opt/appdata/` 来自 data 分区绑定挂载，始终可写。

### Decision 7: syshand.run 文件自动补齐

**方案**：run_app 检测到 syshand 已在运行时，执行 `test -e /opt/appdata/data/syshand.run || touch /opt/appdata/data/syshand.run`

**理由**：syshand.run 由 syshand 初始化时创建。当 syshand 已运行（进程检查检测到），不会启动新实例，也不会重新创建该文件。

### Decision 8: libnopoll.so.0 符号链接

**方案**：extensions Makefile 的 install 目标中创建 `libnopoll.so.0 -> libnopoll.so`

**理由**：`libnopoll.so` 的 SONAME 是 `libnopoll.so.0`，`libcomm_ws_ex.so` 的 NEEDED 条目也是 `libnopoll.so.0`。动态链接器按 SONAME 查找而非文件名，缺少该链接导致 dlopen 失败。

### Decision 9: 调试阶段禁用看门狗/心跳重启

**方案**：
1. handler.c: `system("reboot -f")` 注释掉
2. watch_dog.c: WatchDog_Open 直接返回 TRUE，不打开 `/dev/wd`

**理由**：调试过程中 nccu_app 反复崩溃会导致设备不断重启，干扰排查。**上线前必须恢复。**

## Verification Results

| 验证项 | 结果 | 说明 |
|--------|:----:|------|
| 构建通过 | ✅ | build.sh 零错误完成 |
| RAUC compatible 默认值 | ✅ | `compatible=bmu02_a00` |
| RAUC compatible 环境变量覆盖 | ✅ | `RAUC_COMPATIBLE=bmu05_a00` 生效 |
| syshand 启动无 shmget ERRRRR | ✅ | IPC_EXCL 已移除 |
| nccu_app 启动并持续运行 | ✅ | 已验证 PID 1451，70+秒，主循环700+次 |
| 无 /usr/lib cp 错误 | ✅ | LD_LIBRARY_PATH 替代 |
| 无 /etc/log4crc cp 错误 | ✅ | 路径已改为 /opt/appdata/config |
| USB/SD 不误判（无外部设备时） | ✅ | mmcblk[1-9]* 排除 mmcblk0 |
| libnopoll.so.0 dlopen 成功 | ✅ | SONAME 链接已创建 |
| sampler_ccu.so 符号解析 | ✅ | CCU_Wait_ACCU_Reply_RS232 已定义 |

### Decision 10: sampler_ccu.so 脏编译清理

**方案**：删除 `ccu_com_232.o` 后全量重建

**根因**：构建系统 Makefile 依赖跟踪不完整，`ccu_com_232.o` 未被重新编译，导致 `sampler_ccu.so` 链接时缺少 `CCU_Wait_ACCU_Reply_RS232` 符号定义。nccu_app 事件循环中调用该符号时动态链接器报错终止进程。

**教训**：共享库的 Makefile 需要显式声明所有 `.o` 的依赖关系，避免脏编译。

## 启动链全景

```
OTA升级后nccu_app启动诊断流程
═══════════════════════════════════════════════════════════════

Layer 1: 分区挂载              S91app_mount → dm-verity → /app (ro)
                                      │
Layer 2: 入口脚本              app_entry → startrun
                                      │
Layer 3: USB/SD检测            ┌─ EXTENS_MARK_FILE 存在？→ 跳过检测
                                      │  ├─ ls /dev/sd* → USB设备？→ app_update 升级
                                      │  ├─ ls /dev/mmcblk[1-9]* → SD卡？→ app_update 升级
                                      │  └─ 无设备 → run_app
                                      │
Layer 4: 环境初始化            run_app → LD_LIBRARY_PATH → TRACE_LOG → mkdir
                                      │
Layer 5: syshand               ┌─ syshand 已运行？(ps) → 创建 syshand.run
                                      │  └─ 未运行 → ./bin/syshand & → 等待1秒
                                      │
Layer 6: nccu_app启动          ./bin/nccu_app >>$TRACE_LOG 2>>&1 &
                                      │
Layer 7: nccu_app初始化        ① sqlite → ② DB → ③ DataProc → ④ DB Init
                                      │         ⑤ CreadDataDB → ⑥ log4c → ⑦ Charger
                                      │         ⑧ syshand.run → ⑨ WatchDog → ⑩ SigHandler
                                      │
Layer 8: 模块加载              ⑪ Main_Loop → ⑫ ThreadMgr → ⑬ LoadModuals
                                      │          ├─ Init_CreateDataProcessor
                                      │          ├─ LoadConfigAndData
                                      │          ├─ Init_LoadDAI
                                      │          ├─ Init_Load_AllCommPorts (18个 .so)
                                      │          │    └─ libcomm_ws_ex.so → 依赖 libnopoll.so.0
                                      │          ├─ Init_Load_AllSamplers
                                      │          │    └─ sampler_ccu.so → 依赖 CCU_Wait_...
                                      │          └─ Init_Load_AllReporters
                                      │
Layer 9: 事件循环              ⑭ Main_Run → ⑮ while(!bExit)
                                            ├─ Do_Proc_HeartBeat (→ syshand IPC)
                                            ├─ Queue_Get (等待事件)
                                            ├─ DoEvent (处理事件)
                                            └─ s_nThreadManagerTicks (60s超时)
```