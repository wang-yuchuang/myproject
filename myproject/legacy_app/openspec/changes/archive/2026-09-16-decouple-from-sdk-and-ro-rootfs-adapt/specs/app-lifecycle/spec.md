## ADDED Requirements

### Requirement: R11 构建脚本自包含

本仓库的构建流水线 MUST NOT 依赖指向外部仓库的符号链接。`scripts/` 下的构建脚本 MUST 是本仓库内的**实体文件**，使不具备 `d9_sdk` 访问权限的开发人员能够独立完成构建。

**背景**（历史事实，供追溯）：改造前 `scripts/` 下 5 个构建脚本均为指向 `d9_sdk/source/test_app/scripts/` 的**符号链接**，离开 d9_sdk 树即无法构建。本变更将其替换为实体文件副本，并新增 `scripts/UPSTREAM.md` 记录上游来源与同步策略。

**同步策略**：`scripts/UPSTREAM.md` MUST 记录每个脚本的上游仓库路径与同步方式。上游变更时由人工同步，MUST NOT 通过符号链接实时跟随。

#### Scenario: 无 d9_sdk 权限时独立构建

- **WHEN** 不具备 `d9_sdk` 仓库访问权限的开发人员执行 `./build.sh all`
- **THEN** 构建成功完成，产出 `app.img` 与 RAUC bundle
- **AND** 构建全程 MUST NOT 访问本仓库之外的路径

#### Scenario: 脚本来源可追溯

- **WHEN** 需要核对 `scripts/` 下某个脚本与其上游的差异
- **THEN** `scripts/UPSTREAM.md` 提供该脚本的上游仓库路径与同步记录
- **AND** `scripts/` 目录下 MUST NOT 存在任何符号链接
