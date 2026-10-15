> **性质**：操作手册
> **需求权威源**：`openspec/specs/app-lifecycle/spec.md`（R6/R7/R10）+ [`rauc-compatible.md`](rauc-compatible.md)
> **代码事实基线**：`openspec/.code-facts-snapshot.md`（`## app-lifecycle`）
> **对应提交**：`97cac555`
> **状态**：🟠 部分

# RAUC OTA 出包与安装

出包是 `./build.sh` 的最后一环，安装走设备端 `rauc install`。A/B 槽位架构、挂载链与确认机制的全景见 [ota-and-ab.md](../10-architecture/ota-and-ab.md)，`compatible` 一致性问题见 [rauc-compatible.md](rauc-compatible.md)，本文只给操作步骤。

## 前置条件

- 构建机：`./build.sh` 依赖齐备（见 [build.md](build.md)），签名服务器可达。
- 设备可达：`ping <board>` 通过，能 `ssh root@<board>`。
- 设备时间正确：RTC 未同步时，远端 CMS 签名会因有效期校验失败，报 `certificate is not yet valid`。⚠️ 该报错来自 CMS 有效期校验，具体复现条件未在真机核对。
- 卸载 /app：安装前先 `S92app_mount stop`（spec R6 Scenario；/app 处于 dm-verity 只读挂载时槽位写入无法进行）。

## 出包

```bash
./build.sh          # 输出 remote_pkg/app-<version>.raucb
```

- 版本号由 `git describe --tags --always` + 日期自动生成，如 `dev-826c8021-20260702`（`build.sh:32-48`）。
- bundle 格式为 `verity`（RAUC 1.15+），manifest 由 `ota-bundle.sh:199-210` 生成：`compatible=<RAUC_COMPATIBLE>`、`version=<APP_VERSION>`，默认 `compatible=bmu02_a00`（`build.sh:27`）。
- 签名默认走远端 CMS：临时 RSA-2048 密钥出 bundle、提取并剥离临时证书、远端重签（3 次重试）、`replace-signature` 替换，临时密钥 `shred -u` 删除（`ota-bundle.sh:112-176`）。
- 本地证书路径：`scripts/ota-bundle.sh ... --rauc-cert <crt> --rauc-key <key>` 且不传 `--sign-url`（`ota-bundle.sh:97-109`）。

## 安装

```bash
scp remote_pkg/app-*.raucb root@<board>:/tmp/
ssh root@<board> 'S92app_mount stop'
ssh root@<board> 'rauc install /tmp/app-*.raucb'
```

- 安装只写非活动槽（app.0 / app.1 中的备用槽，spec R6），活动槽不动，随时可回退。
- 安装链内部先停旧实例：`pkill -9 app_entry syshand nccu_app`（`app-post-install:159-164`），随后 `app-post-install` 直接启动 `app_entry`。这是当前唯一确定会启动应用的路径（boot 路径的启动职责悬空，见 ota-and-ab.md「RAUC MARK_GOOD 的归属」一节）。

## 版本查询

```bash
ssh root@<board> "cat /var/lib/rauc/slot-status | grep bundle.version"
```

## slot 与 MARK_GOOD

- `MARK_GOOD` 不是本仓代码：由 BSP 侧 `/usr/sbin/app_sysmgr` 在 R 核信息查询成功后执行，日志 `MARK_GOOD ok (slot %c GPT retry reset)`；失败仅 WARN，boot 重试计数继续累加（spec R10 Scenario）。
- `app_sysmgr` 现役 v2 不启动应用（对 `app_entry` 的 strings 匹配数为 0，spec R10 v1→v2 追溯），boot 路径是**启动责任真空**（`S92app_mount` 只挂载、其 `run_app()` 是死代码不得加回；BSP `add-system-mgmt-service` R2 未交付），**唯一会自动启动应用的路径是 OTA 的 `app-post-install`**。
- OTA 后重启：`S92app_mount` 选新槽，验 D9APPMD1 magic，dm-verity open 后只读挂载 /app。

## 失败与回滚

- `Compatible mismatch: Expected '<device_compatible>' but bundle manifest has '<bundle_compatible>'`：设备端 `/etc/rauc/system.conf` 的 `compatible` 与 bundle 不一致，用 `RAUC_COMPATIBLE=<值> ./build.sh` 重出包（默认 `bmu02_a00`，详见 rauc-compatible.md）。
- `certificate is not yet valid`：同步设备时间（RTC/NTP）后重试安装。
- 新槽位启动失败：RAUC 按 boot 重试计数与 good/bad 标记回退到旧槽位；旧槽位未被安装触碰，回退即恢复。既定方向是不重启整机，应用级自愈设计见 `openspec/changes/keep-app-alive-on-bmu05/design.md`。
- 安装中途掉电：损坏的是非活动槽，活动槽不受影响，恢复后重新 `rauc install` 即可。

## 注意事项

- 不要 `dd` 写已挂载的 app 分区，部署一律走 `rauc install`。
- ⚠️ boot 重试计数的具体阈值与设备端 RAUC 版本行为属 BSP 侧，本仓未核对。
- 设备端 bundle.version 与文件名的版本串同源（`git describe`），查询结果可直接和构建日志比对。
