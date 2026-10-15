## Context

### 符号链接依赖

legacy_app 的 `scripts/` 目录包含 5 个指向 d9_sdk 的符号链接：

```
scripts/assemble_image.sh -> ../../../d9_sdk/source/test_app/scripts/assemble_image.sh
scripts/create_ext4.sh   -> ../../../d9_sdk/source/test_app/scripts/create_ext4.sh
scripts/generate_verity.sh -> ../../../d9_sdk/source/test_app/scripts/generate_verity.sh
scripts/sign_metadata.sh -> ../../../d9_sdk/source/test_app/scripts/sign_metadata.sh
scripts/ota-bundle.sh    -> ../../../d9_sdk/source/test_app/scripts/ota-bundle.sh
```

这要求 legacy_app 必须与 d9_sdk 在同一目录层级下，且开发人员需要 d9_sdk 的读权限。

### 只读 rootfs 运行时冲突

legacy_app 有三类运行时写操作在 d9_sdk 只读 rootfs 上失败：

| 操作 | 失败原因 |
|------|---------|
| `/var/volatile/syshand.pid` | rootfs 无此目录，且只读 |
| `cp -f /app/extensions/log4crc /etc/log4crc` | /etc 只读 |
| `cp -f /app/extensions/lib*.so /usr/lib/` | /usr/lib 只读 |

D9 SDK 侧已提供：
- `/opt/appdata/data/` 可写目录（绑定挂载自 data 分区）
- `/etc/log4crc` 绑定挂载（d9_sdk OpenSpec: harden-app-mount-and-volatile-paths）
- `/var/volatile` 绑定挂载（兼容路径）

## Decisions

### Decision 1: 脚本实体化

将 5 个符号链接脚本替换为实体文件副本。不使用 git submodule 或其他间接方式，因为：
- 脚本变更频率低（dm-verity/ext4 打包逻辑稳定）
- 减少构建时外部依赖
- `UPSTREAM.md` 记录上游来源和手动同步策略

### Decision 2: /var/volatile → /opt/appdata/data/

全局 sed 替换，而非修改单个宏定义，因为：
- `/var/volatile/` 分散在 25+ 个源文件的字符串常量中，无统一宏
- `/opt/appdata/data/` 是 d9_sdk S00data_mount 已提供的可写绑定挂载路径
- 替换后所有 PID 文件、锁文件、临时文件透明写入 data 分区

### Decision 3: 库加载方式改为 LD_LIBRARY_PATH

将 `cp -f /app/extensions/lib*.so /usr/lib/` 替换为 `LD_LIBRARY_PATH` 增加 `/app/extensions`，因为：
- rootfs 只读，无法运行时拷贝到 /usr/lib
- LD_LIBRARY_PATH 是 Linux 标准的动态库搜索路径机制
- `/app/extensions` 是 app 分区内的固定路径

## Verification

### 独立构建验证

| 测试项 | 结果 |
|--------|------|
| d9_sdk 目录移走后构建 | ✅ 通过 |
| app.img D9APPMD1 magic | ✅ |
| RAUC bundle 生成 | ✅ 15MB |

### OTA 升级 + 运行验证（2026-06-25）

| 测试项 | 结果 |
|--------|------|
| RAUC install 成功 | ✅ app_b 激活 |
| syshand 自动启动 | ✅ PID 897 |
| PID 文件写入 /opt/appdata/data/ | ✅ |
| 无 /usr/lib cp 错误 | ✅ |