> **性质**：操作手册
> **需求权威源**：`openspec/specs/app-lifecycle/spec.md`
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`c48d724e`
> **状态**：✅ 已核对

# RAUC OTA 兼容性

RAUC bundle 的 `compatible` 字段必须与设备端一致，否则安装失败。构建时通过 `RAUC_COMPATIBLE` 环境变量控制该字段。

## RAUC_COMPATIBLE

默认值为 `bmu02_a00`：

```bash
# 默认构建（compatible=bmu02_a00）
RAUC_COMPATIBLE=bmu02_a00 ./build.sh
# 或直接
unset RAUC_COMPATIBLE && ./build.sh

# 设备端 compatible 为 bmu05_a00 时
RAUC_COMPATIBLE=bmu05_a00 ./build.sh
```

## 设备端一致性要求

设备端 `/etc/rauc/system.conf` 中的 `compatible` 必须与 bundle 一致，否则 RAUC 安装失败：

```
LastError: Compatible mismatch: Expected '<device_compatible>' but bundle manifest has '<bundle_compatible>'
```
