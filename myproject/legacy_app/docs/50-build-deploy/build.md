> **性质**：操作手册
> **需求权威源**：`openspec/specs/app-lifecycle/spec.md`
> **代码事实基线**：`openspec/.code-facts-snapshot.md`
> **对应提交**：`c48d724e`
> **状态**：✅ 已核对

# 构建与 OTA 部署

## 构建

```bash
./build.sh                    # 一键构建（编译 → 签名 → RAUC bundle）
./build.sh build              # 仅编译
./build.sh clean              # 清理
```

构建产物：`remote_pkg/app-<version>.raucb`。

## RAUC OTA 升级

```bash
# 构建端：./build.sh 生成 remote_pkg/app-<version>.raucb

# 设备端：上传并安装
scp remote_pkg/app-*.raucb root@<board>:/tmp/
ssh root@<board> 'S92app_mount stop'
ssh root@<board> 'rauc install /tmp/app-*.raucb'

# 查询版本号
ssh root@<board> "cat /var/lib/rauc/slot-status | grep bundle.version"
```
