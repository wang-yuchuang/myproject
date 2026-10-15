# 真机验证（D9360 Hardware Verification）

> **性质**：操作手册
> **需求权威源**：暂无（真机验证流程，无对应 capability）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`507729a5`
> **状态**：🟠 部分（⚠️ 设备地址/凭据会变，以现场为准）

本文描述通过 MCP debug 工具对 D9360 真机做验证的操作方式，对接 `/hardware-verify` skill（其输出契约供 `/openspec-verify` 写入 `openspec/changes/<name>/README.md` §5-§6）。验证类操作与启停操作的关系见 `docs/60-operations/lifecycle.md`，OTA 部署背景见 `docs/10-architecture/ota-and-ab.md`。

## 设备与接入

| 板卡/工装 | 地址 | 接入方式 | 备注 |
|---|---|---|---|
| BMU05 | `192.168.1.100` | SSH `root@<board>:2237`，串口（AP/R 两核） | mu3 变体，运行 `src/shells/mu3/*` 脚本 |
| EMU01 | `192.168.100.101` | 同上 | ⚠️ 曾出现 ping 间歇不通；app 分区（app_a/app_b）缺失导致 app 未运行 |

- SSH 端口 2237 来自板端配置 `src/extensions/sshd_config:13`，root 登录。
- 串口经 USB 转接，`<prefix>_serial_detect` 可自动识别：AP 核对应 `MI_00`，R 核对应 `MI_02`。
- ⚠️ 以上地址与凭据以现场实际为准，变动时更新本表。
- MCP debug 工具按板卡/工装分三族，函数名相同、前缀不同：`bmu05-debug2_*`、`bmu05-debug3_*`、`emu01-debug_*`。下文以 `<prefix>_` 指代任一前缀。

## 常用操作

| 目的 | 工具 | 关键参数/命令 |
|---|---|---|
| Ping 连通性 | `<prefix>_ping_test` | `count=4`、`timeout_ms=3000` |
| 串口抓日志 | `<prefix>_serial_open` + `serial_read` | `core="ap"/"r"/"both"`、`baudrate=115200`；`serial_read` 用 `tail_lines` 取尾部 |
| 串口识别设备 | `<prefix>_serial_detect` | 无参数，枚举已连接板卡的串口 |
| 串口登录终端 | `<prefix>_serial_login` | `core="ap"`，自动等 login 提示符并完成登录（最多重试 3 次） |
| 串口执行命令 | `<prefix>_serial_send` | 需先 `serial_open` 拿到 `session_id` |
| SSH 执行命令 | `<prefix>_ssh_exec` | `command`、`timeout_sec=30` |
| 烧录 PAC | `<prefix>_flash_d9_pac` | `image_path`（PAC 路径，留空用默认）、`flash_mode="emmc"/"ospi2"`、`retry`（0~5）；异步执行，调用即返回 |
| 查询烧录进度 | `<prefix>_flash_status` | 轮询到 `running=false`，看 `last_result.success` 判定 PASS/FAIL |
| 烧录实时日志 | `<prefix>_flash_log` | `tail_lines=50`（0 取全部） |
| 上传构建产物到板 | `<prefix>_sftp_upload_from_remote` | `source_path`（构建服务器绝对路径）→ `target_path`（板端路径，如 `/tmp/`） |
| 板端文件取回 | `<prefix>_sftp_download_from_board` | `source_path`（板端，如日志、core dump）→ `target_path`（构建服务器） |
| 释放硬件资源 | `<prefix>_flash_cleanup` / `serial_close` | 结束时关继电器、关串口会话 |

另有 `flash_power_on/off`、`flash_reset`、`flash_boot_enter/exit`、`serial_enter_uboot`、`flash_diagnose` 可用于掉电重启、进 bootloader 与故障诊断，日常验证用不到。

## 典型验证流程

1. **取构建产物**：构建端 `./build.sh` 生成 `remote_pkg/app-<version>.raucb`（完整构建说明见 `docs/50-build-deploy/build.md`）。
2. **上传板端并安装**：`sftp_upload_from_remote(source_path=..., target_path="/tmp/app-<version>.raucb")`，再 `ssh_exec` 执行 `rauc install /tmp/app-<version>.raucb`。人工等价路径：`scp remote_pkg/app-*.raucb root@<board>:/tmp/` 后 `ssh root@<board> 'rauc install /tmp/app-*.raucb'`。RAUC 只写备用槽（app.0/app.1 中的非活动槽），活动槽保持可回退。
3. **重启或人工拉起**：OTA 安装路径会由 `app-post-install` 自动停旧进程并拉起 app。若是整机冷重启，当前 boot 路径无人启动应用，需人工执行 `/app/app_entry /app &`（`docs/60-operations/lifecycle.md`）。
4. **串口确认启动**：`serial_open(core="ap")` 后观察两件事：`/opt/appdata/data/syshand.run` 就绪标记，以及 nccu_app 的 6 步加载日志（DPR 核心 → SQLite 配置 → DAI → 通信模块 → 采样器 → 上报器，`src/main/main.c:147-202`）。任何一步失败都是记日志后进程退出，无部分降级。
5. **SSH 校验进程与数据**：`pidof nccu_app; pidof syshand` 应各返回一个 PID；`cat /var/lib/rauc/slot-status | grep bundle.version` 应为新版本号；`ls /data/` 应看到 `nccu.data` 等 DB 文件（DB 在 `/data` 顶层，不在 `/opt/appdata/data`，原因见快照的运行时可写路径一节）。
6. **判定并记录结果**：对照验证任务的 expected 逐条判定，按 `/hardware-verify` 的输出契约（status / flash / boot_log / connectivity / test_results）写回变更 README §5-§6。

### 结果判定基准

| 检查项 | 期望 | 依据 |
|---|---|---|
| `pidof nccu_app` / `pidof syshand` | 各返回一个 PID | `docs/60-operations/lifecycle.md` 停止后验证 |
| `cat /var/lib/rauc/slot-status \| grep bundle.version` | 与本次构建版本一致 | `docs/50-build-deploy/build.md:29` |
| 串口启动日志 | `syshand.run` 就绪标记出现，6 步加载全部走完 | `src/main/main.c:147-202`、`main.c:316` |
| `/data` 下数据库 | `nccu.data` 等文件存在且非空 | AGENTS.md「运行时可写目录」 |
| PID 文件 | `/opt/appdata/data/syshand.pid`、`nccu.pid` 存在 | 启动链 `run_app:141`、`run_app:191` |

以上任一项不符，先取串口日志与 `dmesg`（`sftp_download_from_board` 拉回）再定位，不盲目重烧。
## 注意事项

- 不要用 `dd` 写已挂载的 app 分区，升级统一走 RAUC 安装。
- RAUC 安装前需先 `S92app_mount stop` 卸载 `/app`（`docs/10-architecture/ota-and-ab.md:55`）。
- ⚠️ RTC 未同步时 RAUC 验签会因时间戳失效而失败，烧录/升级前先核对板端时间。
- 冷重启后无人拉起 app（boot 路径启动责任真空），验证前先确认进程存在，必要时人工 `/app/app_entry /app &`。
- 插着 U 盘或 SD 卡上电会进 `app_update` 而不起业务，上电前拔掉可移动介质（`docs/60-operations/lifecycle.md:72`）。
- 停止应用用 `/app/stoprun`，不要依赖 `/var/run/app.pid`（死 PID，见 lifecycle.md「停止」一节）。
- 串口会话用完 `serial_close`，烧录工装用完 `flash_cleanup`，避免占用继电器与串口影响下次接入。
