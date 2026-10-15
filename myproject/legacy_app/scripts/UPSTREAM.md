# 上游脚本追踪（2026-06-24 更新）

**⚠️ 脚本已独立化：不再依赖 d9_sdk 符号链接**

## 变更记录

原先本目录的脚本是到 `d9_sdk/source/test_app/scripts/` 的符号链接，
这会导致没有 d9_sdk 权限的开发人员无法构建。

2026-06-24：将 5 个脚本从 d9_sdk 直接复制到本目录，替换符号链接。

## 脚本来源

所有 5 个脚本最初源自 `d9_sdk/source/test_app/scripts/`：

| 脚本 | 原位置 |
|------|--------|
| `create_ext4.sh` | `d9_sdk/source/test_app/scripts/create_ext4.sh` |
| `generate_verity.sh` | `d9_sdk/source/test_app/scripts/generate_verity.sh` |
| `sign_metadata.sh` | `d9_sdk/source/test_app/scripts/sign_metadata.sh` |
| `assemble_image.sh` | `d9_sdk/source/test_app/scripts/assemble_image.sh` |
| `ota-bundle.sh` | `d9_sdk/source/test_app/scripts/ota-bundle.sh` |

## 同步策略

当 d9_sdk 侧脚本有更新时，手动同步：
1. 对比差异：`diff scripts/create_ext4.sh /home/zhang_jie/d9_sdk/source/test_app/scripts/create_ext4.sh`
2. 复制到本目录
3. 提交变更

## 其他独立依赖

- `tools/` — RAUC 二进制工具链（自包含）
- `config/verity.conf` — dm-verity 配置参数（自包含）
