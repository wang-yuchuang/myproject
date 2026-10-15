# 已知问题索引（known-issues）

> **性质**：索引
> **需求权威源**：暂无（索引类，无需求源；逐条依据指向 spec / 快照 / 归档变更）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`507729a`（dev，2026-09-17 核对）
> **状态**：✅ 已核对（K1至K4、K6至K8 经代码与快照复核；K5 标 ⚠️ 见行内说明）

## 用途与范围

收录**当前仍然成立**的真实问题，按严重度排序。本表是索引：每条只登记"现在坏了什么、为什么、归谁"，处置方案与改造去向见 `tech-debts.md` 与 `openspec/`（specs 为需求权威源，`changes/` 含 archive 为决策与历史）。已结案条目不进表，见文末「已结案存档」。

### 严重度定义

- 🔴 **运行阻断**：直接影响"装得上、起得来、跑得住"，冷启动或停止链路不可用
- 🟠 **能力缺失/高风险**：某项能力实际不存在或与表象不符，排障、评审、审计易踩坑
- 🟡 **隐患**：当前不影响功能，但离开特定环境（网段、流程）即暴露

### 「状态/归属」列含义

- **BSP 承接**：修复主体在 BSP 仓（d9_sdk），本仓只跟踪其交付判据
- **`app-supervision`**：归未来进程监管变更，本仓已登记方案方向
- **待确认/长期存在**：无既定立项，处置建议见 `tech-debts.md` 对应条目

## 问题索引

| # | 严重度 | 现象 | 成因 | 状态/归属 | 依据 |
|---|---|---|---|---|---|
| K1 | 🔴 | 冷重启后无人启动 app；仅 OTA 安装路径能拉起应用 | 启动责任真空：BSP `add-system-mgmt-service` 的 R1 已完成（`S92app_mount` 移除 `run_app`/`app_entry` 调用，不得加回），R2（`app_sysmgr` 门控启动）未交付：`sysmgr_process.c:506 start_app()` 有实现但 Makefile 未编译 | BSP 承接（R2 交付判据见快照漂移清单）；本仓调查记录见 `keep-app-alive-on-bmu05/design.md` D8 | `.code-facts-snapshot.md:425,443`；`d9_sdk/openspec/changes/add-system-mgmt-service/specs/app-boot/spec.md` |
| K2 | 🔴 | `app_entry` 的 SIGTERM trap 不可达，`/var/run/app.pid` 是死 PID，`S92app_mount stop` 停止链路不可用 | `app_entry` 是非驻留脚本：调用 `startrun` 后即退出（startrun 将 syshand/nccu_app 后台化），shell 退出则 trap 随之消失 | 归 `app-supervision`（正解需前台 supervisor，持有子进程并转发信号）；当前停止应用用 `/app/stoprun`（killall + PID 文件 SIGTERM，本身可用） | `openspec/specs/app-lifecycle/spec.md` R9（:341-368）；`keep-app-alive-on-bmu05/design.md`「记录但不修复」 |
| K3 | 🟠 | 硬件看门狗在当前构建实际未启用（"假绿"） | `src/main/watch_dog.c:6` 写死 `WATCH_DOG_NAME "/dev/wd"`，该字符设备是旧 IMAU/IMMU2 BSP 遗留，D9360 上不存在（实测仅 `/dev/watchdog1..4`，Semidrive virtual watchdog 且 state=inactive）→ `open()` 失败、`s_hWatchDog=-1` | 已决策不恢复（方向：应用故障不重启整机）；由 `app-supervision` 承接，见 `keep-app-alive-on-bmu05/design.md` D6 | `src/main/watch_dog.c:6`；design.md D6 |
| K4 | 🟠 | 平台变体（IMMU3 → BMU05/EMU01）靠编译宏 + 双份脚本，改一处易漏另一处 | `#ifdef IMMU3` 散布于 `main.c`、`config_mangmt.c`、`reporters_loader.c` 等源码；`src/shells/mu3/` 另有整套 `startrun`/`run_app`/`stoprun`/`modem_reset` 与根版并存；生产构建 `MU_TYPE=mu3` 实际生效的是 mu3 版 | 债务 A1：暂不处理、先标记；未来改配置驱动（见 `tech-debts.md` T4） | `src/shells/mu3/`；`openspec/specs/app-lifecycle/spec.md` R9 mu3 变体分列（stoprun:13 vs mu3/stoprun:7） |
| K5 | 🟠 | ⚠️ 板端 RTC 时间过期（断电重启后时钟回落）会导致 RAUC 验签失败（"certificate is not yet valid"），OTA 安装被拒 | RAUC 用 X.509 证书验签，`notBefore` 检查依赖板端系统时间；时钟早于证书生效期即判失败 | ⚠️ 机制成立但**仓内无实证记录**（openspec/文档中未找到发生案例）；标 ⚠️ 待真机复现后补充证据 | 机制：`src/include/3rd_party/packages/openssl-1.1.1k/crypto/x509/x509_txt.c:43`（该错误串即 notBefore 检查产物） |
| K6 | 🟠 | 磁盘上存在未编译的采样器/上报器目录，易误判"有代码就有功能"（排障、评审、安全审计均可能踩坑） | `src/Makefile` 的 `DIRS` 只编译一部分：samplers 17 个子目录中 10 个编译（charger/aems/pv_inverter/imeu/can2_metershare 不在 DIRS）；reporters 的 `modbustcp2_reporter/`、`iec_101/104/61850` 等不在编译清单（modbustcp2 连注释行都没有） | 长期存在；清单真值以 `src/Makefile` 与快照为准 | `.code-facts-snapshot.md:300-316,358-364`；`src/Makefile`（grep modbustcp2 无匹配） |
| K7 | 🟠 | DAI（Init_LoadDAI，主进程加载第 3/6 步）没有任何 capability spec，行为无权威源可查 | 7 个 spec（app-lifecycle/comm-ports/data-processor/publib/reporters/samplers/system-hand）均不覆盖 DAI | 文档真空，待补 spec 或并入现有 capability | `src/main/main.c:165-166`；`ls openspec/specs/`（7 项无 DAI） |
| K8 | 🟡 | 构建签名依赖写死开发网 IP 与 token 字面量，离开开发网段无法出包，token 随源码泄露 | `build.sh:23-24` 将 `SIGN_URL="http://192.168.34.207:8900"` 与 `SIGN_TOKEN` 硬编码 | 待确认（生产环境签名服务可用性）；建议改环境变量注入（见 `tech-debts.md` T6） | `build.sh:23-24,122-123,150-151`；`.code-facts-snapshot.md:444` |

### 排查入口速查

- K1：板上 `ls /dev/by-name/`（app_a/app_b 是否存在）、`strings /usr/sbin/app_sysmgr | grep app_entry`（R2 是否已编译进 v2）
- K2：`cat /var/run/app.pid` 后查 `/proc/<pid>` 是否存在（多半已死）；停止应用直接用 `/app/stoprun`
- K3：板上 `ls -l /dev/wd`（预期不存在）；`dmesg | grep -i watchdog` 看实际注册状态
- K6：`grep '^DIRS' src/Makefile` 与 `ls src/moduals/samplers/ src/moduals/reporters/` 对照
- K8：核对 `build.sh` 头部 `SIGN_URL`/`SIGN_TOKEN` 与当前网络环境
- K5/K7：`date`（板端时钟是否回落）；`ls openspec/specs/` 与 `grep -rn Init_LoadDAI src/main/` 对照

## 复查触发

定期按 `openspec/.code-facts-snapshot.md` 的 **Drift Detection Checklist** 复查本表：K1 对应其高优先级第 4 条（`app_sysmgr` 是否已具备启动能力 = R2 交付判据）；K8 对应其 SIGN_URL 条目。代码变更后若某条不再成立，移入「已结案存档」并注明结案依据。

## 已结案存档（防止重复上报）

- **syshand 心跳丢失 180s 后 `system("reboot -f")`**：结案于 2026-09-16。非"被注释的调试试态"，该调用已从分支移除；心跳丢失后仅记录并继续服务（`no reboot, no exit`）是既定设计，重启应用的职责归未来 `app-supervision`。依据：`openspec/specs/system-hand/spec.md` R1（:37-42）、`handler.c:114-125`；残留注释仅在 `handler.c:173`/`:215` 其它分支。
- **`S92app_mount.run_app()` 从未被调用**：非缺陷。BSP `app-boot` R1 明确要求 S92 仅挂载、移除启动调用；若它被调用反而是回归（依据：快照漂移清单高优先级第 3 条）。

## 新增条目约定

新条目必须带可回溯依据（`file:line` 或 spec/归档链接）与核实日期；机制推断未实证的标 ⚠️。处置类内容（怎么改、谁承接）写入 `tech-debts.md` 或 openspec 变更，不在本表展开。

> 处置方向见 `tech-debts.md` 与 `openspec/`。
