## MODIFIED Requirements

### Requirement: R7 运行时启动链

系统 MUST 按固定顺序启动，从 SDK 分区挂载到 syshand 守护进程和 nccu_app 主进程的就绪。`app_entry` MUST 是唯一的应用入口，由 BSP 启动脚本（boot）与 RAUC 安装后钩子（OTA）**共同调用**，两条调用路径的行为 MUST 一致。

**调用方**：

- boot：BSP `/etc/init.d/S92app_mount` 的 `start)` 分支，在 app 分区挂载与校验通过后执行 `/app/app_entry /app &`，并带幂等守卫避免与 OTA 路径重复启动
- OTA：`/usr/lib/rauc/app-post-install` 的 `start_app()` 执行 `"$APP_POINT/app_entry" "$APP_POINT" &`

**启动链**：

```text
S92app_mount start  (SDK 侧: dm-verity 挂载 /app 只读)
  └─ /app/app_entry /app &           ← 唯一入口；PID 写入 /var/run/app.pid
       ├─ 残留 PID/锁清理（liveness-aware）
       ├─ trap SIGTERM → stoprun
       └─ startrun
            ├─ 检测 USB/SD 升级介质
            ├─ 若有升级 → app_update -fwupdt
            └─ 若无升级 → /app/run_app
                 ├─ 设置环境变量 (NCCU_RUN_PATH / LD_LIBRARY_PATH)
                 ├─ 创建 /opt/appdata/{log,config,data}
                 ├─ 首次复制 /app/config/ → /opt/appdata/config/ (幂等 cp -rn)
                 ├─ 复制 log4crc 到 /opt/appdata/config/
                 ├─ ./bin/syshand &                    ← 守护进程
                 ├─ 等待 syshand 存活检查
                 └─ ./bin/nccu_app -pid=$APP_PID_FILE  ← 主进程
```

#### Scenario: app_entry 残留清理与信号处理

- **WHEN** `app_entry` 被调用（boot 的 `S92app_mount` 或 OTA 的 `app-post-install`，形式均为 `/app/app_entry /app`）
- **THEN** 清理上次异常退出残留的运行标记：对 `/opt/appdata/data/nccu.pid` 与 `/opt/appdata/data/syshand.pid`，**仅当该 PID 对应的进程不是预期进程时**才删除（即：进程已不存在，或该 PID 已被回收复用为其它进程 —— 通过 `/proc/<pid>/comm` 核对名称）
- **AND** 删除纯运行时产物 `/var/volatile/{syshand,nccu}.pid`；`/opt/appdata/data/.syscmd_locker` MUST NOT 被删除（`flock` 随持有者退出由内核自动释放，残留文件不会阻塞后续加锁；unlink 则会使新进程在新 inode 上加锁，与旧持有者各持一锁，破坏跨进程互斥）
- **AND** `/opt/appdata/data/syshand.run` MUST NOT 被删除：`mu3/run_app:87` 仅在"syshand 已在运行"分支才重建该标记，而 `app_entry` 先于 `run_app` 执行；在 syshand 存活时删除会使 `main.c:316` 的就绪等待失败
- **AND** 写入当前 PID 到 `/var/run/app.pid`
- **AND** 注册 SIGTERM trap：收到信号时调用 `stoprun`、删除 PID 文件、退出
- **AND** 调用 `startrun` 启动业务
- **AND** 若 `nccu_app` 已在运行（`pidof nccu_app`），MUST 幂等退出（不重复启动、不改动任何 PID 标记），以避免 boot 与 OTA 重叠或 `S92app_mount restart` 产生第二实例

> **为什么必须做残留清理**：`/app/run_app` 在 `nccu.pid` 存在时会 `exit 1` 拒绝启动。该文件位于持久化的 `/opt/appdata/data`（`/dev/mmcblk0p45`），若上次为异常退出（未经 `stoprun`，例如整机重启），残留文件会让**后续每一次启动永久失败**。
>
> **为什么仍要 liveness-aware**：两条调用路径到达 `app_entry` 时进程都已终止 —— OTA 路径由 `app-post-install` 的 `stop_app()` 执行 `pkill -9 app_entry syshand nccu_app`（`:159-164`）并用 `close_verity_device()` 的 `fuser -k` 兜底，boot 路径是全新启动。但整机重启后 **PID 会被回收复用**，仅用 `kill -0` 会把恰好占用该 PID 的无关进程误判为"存活"，从而保留死 PID 文件并使 `run_app` 永久 `exit 1`；反之，若将来在应用仍在运行时调用本脚本，盲删 `syshand.pid` 又会让后续 `stoprun` 静默失效并遗留 System V 共享内存与信号量。因此需要按**进程名**核对。

#### Scenario: 升级介质检测（startrun）

- **WHEN** startrun 检测启动路径
- **THEN** 检查 `/var/.ext_mark` 文件存在性：存在则跳过升级检测（升级后首次启动）
- **AND** 否则检测 USB 设备（`/dev/sd*`）和 SD 卡设备（`/dev/mmcblk[1-9]*`）
- **AND** 若发现可移动设备 → 执行 `/app/bin/app_update`（**mu3 变体**为无参调用，`src/shells/mu3/startrun:88,94`；非 mu3 变体为 `-qws -nokeyboard -fwupdt`，`src/shells/startrun:99,105`）
- **AND** 若无可移动设备 → 调用 `wait_appupdt_exe_done`（等待 firmwareupdt.flag 消失）后调用 `/app/run_app`

> **SD 卡检测修正**（2026-07）：原代码使用 `ls /dev/mmc*` 会误检测内部 eMMC（`/dev/mmcblk0`），已修正为 `ls /dev/mmcblk[1-9]*` 以排除 mmcblk0。

#### Scenario: run_app 环境准备

- **WHEN** `run_app` 执行
- **THEN** 设置 `NCCU_RUN_PATH=/app`、`NCCU_DATA_PATH=/data`、`LD_LIBRARY_PATH` 包含 `/app/lib:/app/comm_port:/app/extensions`
- **AND** 通过 hostname 判断运行平台（`bmu*` → `VAR_DIR=/opt/appdata/data`）
- **AND** 创建运行时可写目录：`/opt/appdata/{log,config,data}`
- **AND** 首次启动幂等复制配置：`cp -rn /app/config/* /opt/appdata/config/`
- **AND** 复制 log4crc：`/app/extensions/log4crc → /opt/appdata/config/log4crc`

#### Scenario: syshand 守护进程启动

- **WHEN** `run_app` 启动 syshand
- **THEN** 检查 syshand 是否已在运行。该判定 MUST 使用按**进程名精确匹配**的方式，首选 `pidof syshand`（bmu05 实测对 `./bin/syshand` 有效）；MUST NOT 使用任何可被子串命中的模式（如 `[s]yshand`）—— 否则会因并发的 `rm -f .../syshand.pid` 等进程造成假阳性，进而跳过 syshand 启动并在后续存活检查处以 `exit 0` 终止，导致**应用永不启动**
- **AND** 若已在运行 → 确保 `/opt/appdata/data/syshand.run` 标记文件存在
- **AND** 若未运行 → `./bin/syshand &` 启动
- **AND** 等待 BSP 扩展安装完成（`/app/extensions/install.sh` 进程退出）
- **AND** 存活检查失败 → `exit 0` 终止启动

> **修正说明（2026-09，实测）**：原实现使用 `ps | grep -q '[s]yshand'`，会匹配**任何 argv 含 `syshand` 子串的进程**（例如并发的 `rm -f .../syshand.pid`）。在 bmu05 上实测：即使 syshand 确实未运行，该判定仍返回 MATCH（假阳性）；随后 `bin/syshand` 存在性检查判定为否 → `exit 0` → **应用永不启动**。改用 `'[b]in/syshand'` 或 `pidof syshand` 后判定正确（同文件 `:117` 原本已使用 `bin/syshand`，本次统一为精确模式）。

#### Scenario: nccu_app 启动

- **WHEN** syshand 存活检查通过
- **THEN** 检查 nccu_app 是否已在运行（避免重复启动）
- **AND** 执行 `sync && echo 3 > /proc/sys/vm/drop_caches` 清理缓存（**仅 mu3 变体**，`src/shells/mu3/run_app:143-144`；非 mu3 版 `run_app` 无此操作）
- **AND** 启动 `./bin/nccu_app -pid=$APP_PID_FILE`，stdout/stderr 重定向到 `/opt/appdata/log/trace.log`
