## ADDED Requirements

### Requirement: R10 trace_* 追踪函数的格式化安全

系统 MUST 保证 `trace_*` 家族（`iBasic_Funs.c` 的 `trace_debug` / `trace_info` / `trace_warn` / `trace_error` / `trace_fatal`，经 `TRACED` / `TRACEI` / `TRACEW` / `TRACEE` / `TRACEF` 宏调用）在格式化与落盘时**不会二次解释消息内容**，且 MUST NOT 复用已被消费的 `va_list`。

**既有缺陷**（`iBasic_Funs.c:2634-2652`，修复前）：
- 先 `vprintf(format, args)`，随后用**同一个** `args` 调 `vsnprintf` —— `va_list` 已被消费，属未定义行为；aarch64 上 `va_list` 为结构体，二次读取会越界
- `LogOut(..., szBuff)` 把消息缓冲区**直接当作格式串**传入 —— 消息中出现 `%s` / `%n` 等会被二次解释（格式串漏洞，可致崩溃或任意内存写）

**要求**：
- 每个变参列表 MUST 只被解析一次；若确需二次使用，MUST 先用 `va_copy` 复制
- 任何以「消息内容」为实参的日志调用 MUST 使用显式格式串（如 `"%s"`）；MUST NOT 把消息缓冲区直接作为格式串
- 修复 MUST NOT 改变既有可见输出（`trace_header` → 消息 → `trace_tail` 的顺序与文本）

#### Scenario: 消息内含百分号

- **WHEN** 调用 `TRACEF("%s failed", path)`，或消息文本本身含 `%s` / `%n`
- **THEN** 消息被原样输出并落盘，MUST NOT 触发二次格式化
- **AND** 进程 MUST NOT 因格式串解释而崩溃或产生越界写

#### Scenario: 变参只解析一次

- **WHEN** 任一 `trace_*` 函数执行
- **THEN** 其 `va_list` MUST NOT 在被 `vprintf` / `vsnprintf` 消费后再次使用
- **AND** 输出到 stdout 与写入日志库（`LogOut`）的消息文本 MUST 一致

#### Scenario: 可见输出不变

- **WHEN** 该修复生效
- **THEN** `trace_header` → 消息 → `trace_tail` 的可见顺序 MUST 与修复前一致
- **AND** 消息前缀 `F[<file>] L[<line>] ` 的格式 MUST 保持不变
