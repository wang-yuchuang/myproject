## ADDED Requirements

### Requirement: R12 mu3 平台启动脚本与主脚本功能一致

`MU_TYPE=mu3` 时构建实际加载的是 `src/shells/mu3/run_app`（**不是** `src/shells/run_app`）。因此对任一 `run_app` 变体的修复 MUST 同步到另一个变体，二者 MUST 在以下方面行为一致：

- `LD_LIBRARY_PATH` MUST 包含 `/app/extensions`
- MUST NOT 存在任何向 `/usr/lib/` 拷贝 `.so` 的命令
- syshand 存活判定 MUST 使用**进程检查**（`pidof syshand` 或 `ps | grep '[s]yshand'`），MUST NOT 仅依赖 PID 文件存在性
- MUST 定义 `TRACE_LOG` 并创建 `/opt/appdata` 运行目录
- log4crc 拷贝 MUST 带容错（`2>/dev/null || true`）

**背景**（历史事实，供追溯）：`MU_TYPE=mu3` 的构建实际使用 `mu3/run_app`，但早期修复只做在主 `run_app` 上，导致 mu3 版缺 `LD_LIBRARY_PATH`、仍向 `/usr/lib` 拷贝、且用 PID 文件判定 syshand —— 应用启动后立即退出，核心业务无法运行。

#### Scenario: 修改任一变体需同步

- **WHEN** 修改 `src/shells/run_app` 或 `src/shells/mu3/run_app` 中的任一文件
- **THEN** 另一个变体同步修改，二者在上述各方面行为一致

#### Scenario: mu3 变体的 syshand 存活判定

- **WHEN** `mu3/run_app` 判定 syshand 是否已运行
- **THEN** 使用进程检查而非 PID 文件存在性
- **AND** 判定为已运行时 MUST 确保 `/opt/appdata/data/syshand.run` 标记文件存在（供 `nccu_app` 就绪检查使用）

### Requirement: R13 nccu_app 以生产模式直接启动

`run_app` MUST 直接启动 `nccu_app`，MUST NOT 使用 `strace` 等调试包装器。stderr MUST 重定向到追踪日志，以保留错误可见性。

**背景**（历史事实，供追溯）：`run_app` 曾以 `strace -o $TRACE_LOG ... ./bin/nccu_app` 的形式启动（调试残留），不应出现在生产启动链中。

#### Scenario: 生产启动

- **WHEN** `run_app` 启动 nccu_app
- **THEN** 命令形式为 `./bin/nccu_app -pid=$APP_PID_FILE 2>>$TRACE_LOG &`
- **AND** MUST NOT 出现 `strace` 包装

### Requirement: R14 libnopoll SONAME 兼容（libnopoll.so.0）

构建产物 MUST 在部署 `libnopoll.so` 的同时创建指向它的 **`libnopoll.so.0`** 链接，以满足动态链接器对 SONAME 的解析需求。

**机制**：`libnopoll.so` 的 SONAME 是 `libnopoll.so.0`，而 `libcomm_ws_ex.so`（comm_websocket_ex 模块）的 `NEEDED` 条目同样是 `libnopoll.so.0`。动态链接器按 **SONAME** 查找而非文件名，因此部署中仅有 `libnopoll.so` 而无 `.so.0` 链接时，`dlopen` 会失败。

**实现**：`src/extensions/Makefile` 安装目标中创建链接 —— `-ln -sf libnopoll.so $(NCCU_TARGET_DIR)/extensions/libnopoll.so.0`。

#### Scenario: dlopen libcomm_ws_ex.so

- **WHEN** 通信层通过 `dlopen` 加载 `libcomm_ws_ex.so`
- **THEN** 其依赖 `libnopoll.so.0` MUST 可在库搜索路径中解析
- **AND** MUST NOT 出现 `cannot open shared object file` 错误
