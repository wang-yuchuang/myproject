## 0. 前置条件

- [x] 0.1 验证 Docker 环境（`docker --version`）和 `aarch64-linux-gnu-gcc --version` 交叉编译器可用，不可用时报错并提示
- [x] 0.2 验证签名服务器可达（如有 `--sign-url`）：`curl -s --connect-timeout 5 <SIGN_URL>/../health` 或明确跳过

## 1. 构建脚本基础结构

- [x] 1.3 创建顶层 `build.sh`，作为唯一构建入口
- [x] 1.4 build.sh 所有参数写死（签名 URL/token/key_id/分区大小/RAUC compatible/MU_TYPE），无多余配置项
- [x] 1.5 版本号自动从 `git describe` 生成，写入 D9APPMD1 metadata、镜像文件名、nccu_app 二进制
- [x] 1.6 更新 `.gitignore`：添加 `pkg/`、`output/`、`*.img`、`*.raucb`、`*.rpkg`
- [x] 1.7 创建 `scripts/UPSTREAM.md`，记录上游脚本路径和追踪的 commit hash
- [x] 1.8 grep 并清理旧脚本中 `openssl rsautl` 加密调用点，确认 Makefile install 中该步骤已通过条件跳过或移除
- [x] 1.9 删除冗余文件：`config/build.conf`、`src/shells/rsa_prublic_key`、`src/root_passwd`
- [x] 1.10 注释 Makefile 中已废弃的 `make_md5sum` 调用和 dead variable 引用

## 2. 编译阶段适配

- [x] 2.1 `build.sh` 编译阶段正确调用 `source src/mkenv_arm_st && src/mk_define release ${MU_TYPE:-mu3} && make install`，环境隔离在子 shell
- [x] 2.2 验证编译阶段输出到 `out/` 目录结构完整（bin、config、lib、sampler、reporter 等子目录）

## 3. 打包阶段适配

- [x] 3.1 分区大小默认 256M（与硬件 app_a/app_b 分区一致，可从 `config/build.conf` 配置）
- [x] 3.2 调用 `create_ext4.sh` 用 `-b 4096` 从 `out/` 创建 ext4 镜像
- [x] 3.3 调用 `generate_verity.sh` 生成 dm-verity hashtree
- [x] 3.4 调用 `sign_metadata.sh` 生成 D9APPMD1 metadata 并通过 `--sign-url` 在线签名
- [x] 3.5 调用 `assemble_image.sh` 组装最终 `pkg/app-${VERSION}.img`
- [x] 3.6 打包阶段输出校验：app.img 大小精确等于分区大小（256M），metadata 在尾部 4096 字节位置
- [x] 3.7 构建时验证 ext4 block size 为 4096（`dumpe2fs -h`）
- [x] 3.8 处理 Makefile 中 openssl rsautl 加密步骤：`ifndef NCCU_SKIP_ENCRYPT` 条件跳过

## 4. OTA Bundle 集成

- [x] 4.1 复制 `test_app/tools/`（含 rauc 二进制 + 依赖库）和 `ota-bundle.sh` 到 legacy_app
- [x] 4.2 支持远程 CMS 签名（`--sign-url` 两阶段替换）和本地证书签名（`--rauc-cert`/`--rauc-key`）
- [x] 4.3 `build.sh` 集成 `--ota` 参数，在打包 app.img 后自动生成 RAUC bundle
- [x] 4.4 `build.sh` 集成 `--legacy-rpkg` 参数，额外生成旧格式 `app_nccu.rpkg`

- [x] 5.1 本机执行完整构建：`./build.sh`，生成带签名、带版本号、256MB 精确对齐的 app.img
- [x] 5.2 验证构建产物完整性：metadata magic `D9APPMD1` ✅、app.img 大小 268435456 B（精确 256M）✅、metadata.sig 256 B ✅、ext4 block size 4096 ✅
- [x] 5.3 设备端验证：RAUC bundle 安装到 app_b，`S91app_mount` 读取 metadata `ver=1.0.0 key=ota_signing`，签名验证通过，dm-verity 挂载到 `/app` ✅
- [x] 5.4 验证 app 业务功能：`/app/bin/nccu_app` 启动（ELF 64-bit ARM aarch64），app_entry 运行 ✅

## 6. 构建精简与可写目录

- [x] 6.1 build.sh 精简：移除 `--sign-url`、`--sign-token`、`--key-id`、`--docker`、`--skip-sign`、`--legacy-rpkg`、`--ota`、`--mu-type`、`--partition-size` 等多余参数，所有配置写死
- [x] 6.2 build.sh 编译前自动生成 `version_patch.h`，将 git describe 版本号注入 nccu_app 二进制
- [x] 6.3 `run_app` 启动时创建 `/opt/appdata/{log,config,data}` 可写目录，首次启动复制默认配置
- [x] 6.4 `run_app` TRACE_LOG 路径从 `/data/log/trace.log` 改为 `/opt/appdata/log/trace.log`
- [x] 6.5 `app_entry` 写 PID 到 `/var/run/app.pid`，注册 SIGTERM trap 调用 `stoprun` 优雅退出
- [x] 6.6 创建 `app_exit` 脚本，用于独立停止 app
- [x] 6.7 删除冗余文件：`config/build.conf`、`src/shells/rsa_prublic_key`、`src/root_passwd`
- [x] 6.8 注释 Makefile 中已废弃的 `make_md5sum` 调用
- [x] 6.9 Makefile install target 中 dead variable 引用清理
