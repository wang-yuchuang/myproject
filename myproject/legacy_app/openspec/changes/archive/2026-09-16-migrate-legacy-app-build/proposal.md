## Why

legacy_app 当前使用传统 ext4 镜像打包方式（`mkfs.ext4 -d out/`），缺乏 dm-verity 完整性校验和签名验签机制，无法与 D9 SDK rootfs 的 `S91app_mount` 验签挂载流程兼容。现 D9 SDK 已建立完整的在线签名基础设施（签名服务器 + trust anchor + D9APPMD1 metadata 协议），legacy_app 需改造构建方法以利用此基础设施，实现与 test_app 一致的构建输出格式和 OTA 升级能力，为后续正式升级迁移做准备。

## What Changes

- **构建入口重构**：将 `mkenv_arm_st → mk_define → make → do_pack()` 流程改造为 `build.sh` 统一入口，保留源码编译逻辑
- **打包方式替换**：废弃 `do_pack()` 的原始 ext4 打包，改用 `create_ext4 → generate_verity → sign_metadata → assemble_image` 流水线
- **在线签名集成**：移除本地 RSA 密码加密，集成远程签名服务器 API（`--sign-url`），metadata 签名走在线服务
- **输出格式对齐**：输出 D9APPMD1 自包含镜像 `[ext4 + dm-verity hashtree + signature + metadata]`，精确对齐分区大小
- **OTA bundle 集成**：基于 test_app 的 `ota-bundle.sh` 生成 RAUC 兼容的 OTA 升级包
- **rootfs 适配**：输出镜像与 `S91app_mount` 完全兼容，支持 `/dev/by-name/app_a` / `app_b` 双槽位

## Capabilities

### New Capabilities

- `app-image-builder`: 自包含 app 分区镜像构建流水线，支持 ext4 + dm-verity + D9APPMD1 metadata + 在线签名
- `app-ota-bundle`: RAUC 兼容的 app OTA 升级包构建，支持双槽位升级和回滚
- `app-build-entry`: 统一的 build.sh 入口，封装编译、打包、签名全流程

### Modified Capabilities

- (无 — legacy_app 之前没有 OpenSpec specs，所有能力均为新建)

## Impact

- **源码目录**：`src/` 下的 C/C++ 模块代码不受影响，仅修改构建脚本
- **构建脚本**：新增 `build.sh`（单一入口）、`scripts/`（流水线步骤），废弃 `mkenv_arm_st`/`mk_define`
- **include.mk 微调**：`=` 改为 `?=` 赋值，支持 Docker 环境变量覆盖工具链路径
- **输出目录**：`out/` 仍为编译产物中间目录，`pkg/` 和 `remote_pkg/` 的输出格式改为 `app.img`（D9APPMD1 格式）
- **外部依赖**：新增依赖 `openssl`（用于 hash 计算）、`mke2fs`（ext4）、Python 3（hashtree 生成）、远程签名服务器
- **rootfs 侧**：无变更 — 使用 D9 SDK 现有的 `S91app_mount` + trust anchor 即可挂载和验签
