## MODIFIED Requirements

### Requirement: R4 dm-verity 完整性校验

系统 MUST 在构建期生成 dm-verity hashtree（Merkle tree），用于运行时挂载时验证 app 分区完整性。

**实现**：`scripts/generate_verity.sh` 使用**纯 Python 实现**（不依赖 `veritysetup` CLI），参数由 `config/verity.conf` 提供。

**参数**（`config/verity.conf`）：

| 参数 | 值 |
|------|-----|
| `HASH_ALGORITHM` | sha256 |
| `DATA_BLOCK_SIZE` | 4096 |
| `HASH_BLOCK_SIZE` | 4096 |
| `SALT_SIZE` | 32 |

#### Scenario: hashtree 生成（构建期）
- **WHEN** `generate_verity.sh --image app.ext4 --output-dir pkg/ --block-size 4096`
- **THEN** Python 脚本逐 block 读取 ext4 镜像，计算 `sha256(salt + block_data)` 叶子节点哈希
- **AND** 每个数据 block 产生 32 字节哈希，128 个哈希打包为一个 hash block（4096 / 32 = 128）
- **AND** 自底向上构建多级 Merkle tree，直到只剩一个根哈希
- **AND** Salt 为 32 字节随机数（`os.urandom`），每次构建重新生成
- **AND** 输出 `pkg/app_hashtree.bin`（树数据，不含 root hash）和 `pkg/verity_info.txt`（含 Root hash / Salt / Hash blocks 数量）

#### Scenario: 运行时 verity 挂载（SDK 侧，非本仓库）
- **WHEN** 设备启动时 SDK 的 `S92app_mount` 挂载 app 分区
- **THEN** 使用元数据中的 Root hash 建立 dm-verity 设备
- **AND** 只读挂载（`-o ro`）防止 superblock 被修改
- **AND** 任何 block 被篡改时 dm-verity 校验失败，拒绝挂载

> **注意**：原 spec 描述的 "使用 `veritysetup open` 建立 dm-verity 设备" 是**设备端运行时行为**，由 SDK 侧 S92app_mount 实现，不在本仓库代码中。本仓库仅负责构建期生成 hashtree 和元数据。

### Requirement: R6 RAUC OTA Bundle 生成

系统 MUST 将签名后的 app.img 封装为 RAUC verity bundle，支持通过 RAUC 框架进行 A/B 槽位 OTA 升级。

**实现**：`scripts/ota-bundle.sh`，优先使用 `tools/bin/rauc`（自包含），回退到 Buildroot host-rauc，最后系统 rauc。

#### Scenario: Bundle manifest 生成
- **WHEN** `ota-bundle.sh` 构建 bundle
- **THEN** 生成 `manifest.raucm`：
  ```
  [update]
  compatible=bmu02_a00
  version=<APP_VERSION>
  description=App partition update for bmu02_a00

  [bundle]
  format=verity

  [image.app]
  filename=app.img
  ```
- **AND** bundle format 为 `verity`（RAUC 1.15+ 支持的增量格式）

#### Scenario: 远程 CMS 签名
- **WHEN** 使用远程 CMS 签名（`--sign-url` 提供）
- **THEN** 流程为：生成临时 RSA-2048 密钥 → 用临时密钥创建 verity bundle → 提取 CMS 签名 → 剥离临时证书 → 远程 CMS 重签（3 次重试）→ 替换签名
- **AND** 远程签名 URL：`${SIGN_URL}/api/v1/sign/rauc/cms?project=d9_sdk&key_id=ota_signing`
- **AND** 临时密钥用完后 `shred -u` 安全删除

#### Scenario: 本地签名回退
- **WHEN** 提供 `--rauc-cert` 和 `--rauc-key` 而非远程签名
- **THEN** 使用 `rauc bundle --cert=<cert> --key=<key>` 直接签名
- **AND** 证书和密钥文件必须存在

#### Scenario: RAUC 安装（设备端）
- **WHEN** 在设备端执行 `rauc install remote_pkg/app-<version>.raucb`
- **THEN** RAUC 将 bundle 解包到备用槽位（app.0 或 app.1）
- **AND** 下次重启后从新槽位启动
- **AND** 安装前 MUST 先执行 `S92app_mount stop`（卸载当前 /app）

### Requirement: R9 优雅停止与信号处理

系统 MUST 在收到停止信号时优雅停止 syshand 和 nccu_app 进程，并清理 PID 文件。

**实现**：`src/shells/app_entry`（SIGTERM trap）+ `src/shells/stoprun`（停止脚本）

#### Scenario: SIGTERM 触发停止
- **WHEN** S92app_mount 执行 stop（发送 SIGTERM 到 app_entry 进程）
- **THEN** app_entry 的 trap 被触发，依次执行：
  1. 打印停止日志
  2. 调用 `${APP_SHELL_PATH}/stoprun`
  3. 删除 `/var/run/app.pid`
  4. `exit 0`

#### Scenario: stoprun 停止顺序
- **WHEN** `stoprun` 被调用
- **THEN** 先 `killall -9 app_update` 和 `killall -9 nccu_ui_mm`（强制停止升级和 UI 进程）
- **AND** 然后通过 PID 文件（`/opt/appdata/data/syshand.pid`）向 syshand 发送 SIGTERM
- **AND** 最后通过 PID 文件（`/opt/appdata/data/nccu.pid`）向 nccu_app 发送 SIGTERM
- **AND** 每个进程停止后删除对应的 PID 文件

#### Scenario: PID 文件管理
- **WHEN** 进程启动和停止时
- **THEN** app_entry PID → `/var/run/app.pid`（供 S92app_mount stop 使用）
- **AND** syshand PID → `/opt/appdata/data/syshand.pid`（供 stoprun 使用）
- **AND** nccu_app PID → `/opt/appdata/data/nccu.pid`（由 `nccu_app -pid=` 参数写入，供 stoprun 使用）
- **AND** 进程退出时 MUST 删除对应 PID 文件，避免僵尸 PID
