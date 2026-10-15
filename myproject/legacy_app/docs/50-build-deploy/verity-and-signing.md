> **性质**：操作手册
> **需求权威源**：`openspec/specs/app-lifecycle/spec.md`（R6/R7）
> **代码事实基线**：`openspec/.code-facts-snapshot.md`（`## app-lifecycle`）
> **对应提交**：`97cac555`
> **状态**：🟠 部分

# dm-verity 与镜像签名

从 `out/` 编译产物到 `pkg/app-<version>.img` 共四步：生成 ext4 → 生成 dm-verity hashtree → 生成并签名 D9APPMD1 元数据 → 组装镜像。`./build.sh` 会自动跑完整条链（`build.sh:99-139`），本文说明每步在做什么、如何单独执行、以及构建后的验证手段。

## 前置条件

- 构建依赖：`aarch64-linux-gnu-gcc`（`build.sh:63` 启动时检查）、`mke2fs`、`dumpe2fs`、`python3`、`curl`、`openssl`。
- 已完成编译：`./build.sh build`，产物在 `out/`。
- 签名服务器可达：`http://192.168.34.207:8900`（写死于 `build.sh:23-24`，生产环境需确认）。

## 步骤

### 1. 生成 ext4（scripts/create_ext4.sh）

```bash
scripts/create_ext4.sh --input-dir out --output pkg/app.ext4 --size <ext4_bytes>
```

- `ext4_bytes = 256M - 2% - 4096`（`build.sh:101-103`）。分区固定 256MB，扣除的部分留给 hashtree 与尾部元数据。
- 内部执行 `mke2fs -t ext4 -b 4096 -d <dir>`，并先 `truncate` 到 4096 对齐的精确大小（`create_ext4.sh:39-46`）。block 固定 4096，dm-verity 按此对齐。

### 2. 生成 dm-verity hashtree（scripts/generate_verity.sh）

```bash
scripts/generate_verity.sh --image pkg/app.ext4 --output-dir pkg --block-size 4096
```

- 纯 Python 逐 block 计算 sha256 Merkle 树，不依赖 `veritysetup` CLI（脚本第 2 行即声明）。动因：构建机不假设安装 veritysetup，目标 rootfs 也没有，纯 Python 让本仓自包含（spec R11）。设备端的 verity 挂载由 SDK 侧 `S92app_mount` 完成，本仓只管构建期生成。
- 参数来自 `config/verity.conf`：`HASH_ALGORITHM=sha256`、`DATA_BLOCK_SIZE=4096`、`HASH_BLOCK_SIZE=4096`、`SALT_SIZE=32`。
- salt 每次构建随机生成（`generate_verity.sh:54`），两次构建 root hash 不同属正常；输出 `pkg/app_hashtree.bin` 与 `pkg/verity_info.txt`（`Root hash:` / `Salt:` / `Hash blocks:` 三行）。打包阶段 `build.sh:110-112` 会校验 block size 为 4096。

### 3. 生成并远端签名 D9APPMD1 元数据（scripts/sign_metadata.sh）

```bash
scripts/sign_metadata.sh \
  --image pkg/app.ext4 --hashtree pkg/app_hashtree.bin \
  --version <APP_VERSION> --metadata-version 1 --key-id ota_signing \
  --output-dir pkg \
  --sign-url http://192.168.34.207:8900/api/v1/sign \
  --sign-token token-d9-sdk-001
```

- 元数据二进制以 ASCII `D9APPMD1` 开头，字段含 root hash、salt、镜像与 hashtree 的偏移/大小、block size、app 版本、`rollback_index=0`、`signature_alg=RSA-SHA256`、签名长度 256（RSA-2048）。逐字段格式见 `sign_metadata.sh:57-64` 注释。
- 远端签名是 `curl POST`，头部 `Authorization: Bearer <token>`，body 为 JSON（`sign_metadata.sh:150-153`）；服务未返回签名则报错退出（`sign_metadata.sh:161-163`）。
- 不传 `--sign-url` 时元数据不签名，`metadata.sig` 为空文件（`sign_metadata.sh:184-186`）。

### 4. 组装 app.img（scripts/assemble_image.sh）

```bash
scripts/assemble_image.sh \
  --ext4 pkg/app.ext4 --hashtree pkg/app_hashtree.bin \
  --metadata pkg/metadata.bin --signature pkg/metadata.sig \
  --output pkg/app-<version>.img --partition-size 256M
```

- 尾部布局（`assemble_image.sh:39-57`）：`[ext4][hashtree][zero 填充][签名][元数据]`，元数据固定占分区尾 4096 字节，RSA 签名紧邻其前，总大小严格 256MB。设备端 `S92app_mount` 从 `partition_end-4096` 读元数据、`partition_end-4096-512` 读签名（`assemble_image.sh:3`）。（⚠️ 该脚本第 3 行内联注释仍写 `S91app_mount`，未随 S91→S92 更名同步，属遗留；此处证据是脚本行为而非该注释。）

## 验证

```bash
# 1. 尾部 magic，输出应为 D9APPMD1
magic=$(dd if=pkg/app-*.img bs=1 skip=$((256*1024*1024 - 4096)) count=8)
echo $magic

# 2. root hash，输出 Root hash: <64 位十六进制>
grep '^Root hash:' pkg/verity_info.txt

# 3. ext4 block size，输出 Block size: 4096
dumpe2fs -h pkg/app.ext4 | grep 'Block size'
```

⚠️ `./build.sh` 完整跑完后会删除 `pkg/app.ext4`、`app_hashtree.bin`、`verity_info.txt` 等中间产物（`build.sh:154-159`）。上面 2、3 两项要在分步执行时验证，或赶在 bundle 阶段结束前看构建日志。

## 回滚 / 失败

- 各脚本 `set -e`，任一步失败立即退出；显式校验失败点有两处：block size 不为 4096（`build.sh:112`）、magic 不为 D9APPMD1（`build.sh:137`）。修正输入后重跑 `./build.sh`，中间产物被覆盖，无需手工清理。
- magic 校验失败：常见原因是 assemble 的 `--partition-size` 与 dd 的 `skip` 不一致，或元数据生成失败后拼了旧文件。清空 `pkg/` 后重跑打包阶段。
- 签名服务不可达：`sign_metadata.sh` 报 `ERROR: Signing service returned no signature` 后退出。`ota-bundle.sh` 支持本地证书回退（`--rauc-cert/--rauc-key`），但 `sign_metadata.sh` 本身只有远端 http 签名分支 ⚠️ 本地签名元数据未验证可行。

## 注意事项

- 签名服务器地址与 token 写死为字面量（`http://192.168.34.207:8900` / `token-d9-sdk-001`，`build.sh:23-24`），生产环境需确认，建议改为环境变量注入。root hash 的有效载体是镜像内的元数据，`verity_info.txt` 只是构建期附件。
- 全链不需要 root 权限，不要 `dd` 写已挂载的 app 分区，部署走 RAUC（见 [rauc-ota.md](rauc-ota.md)）；架构背景与镜像尾部布局图见 [ota-and-ab.md](../10-architecture/ota-and-ab.md)。
