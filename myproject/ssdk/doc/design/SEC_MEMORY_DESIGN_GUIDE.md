# SEC Domain Memory Design Guide

**文档版本**: 3.0  
**最后更新**: 2026-04-07  
**适用平台**: D9360, bmu02_a00

---

## 1. 架构概述

### 1.1 SEC 域内存架构

**核心原则**: SEC 域代码运行在 SRAM，不是 DDR！

```
启动流程：
┌──────────────────────────────────────────┐
│ Boot ROM → 加载 SEC 固件 → SRAM → 执行  │
│                            ↑             │
│                       DDR 未初始化       │
└──────────────────────────────────────────┘

SEC 域职责：
├─ 安全启动验证
├─ 密钥管理
├─ 权限控制
└─ 安全监控
```

### 1.2 内存区域定义

#### 1.2.1 SRAM（代码运行空间）

| 属性 | 值 |
|------|-----|
| **物理地址** | 0x140000 ~ 0x200000 |
| **大小** | 768KB（硬件固定，不可修改） |
| **配置** | CONFIG_MEM_BASE/SIZE |
| **用途** | .text, .data, .bss, Heap, Stack |

**SRAM 组成** (IRAM = Internal SRAM):
```
IRAM1: 0x100000 ~ 0x140000 (256KB)
IRAM2: 0x140000 ~ 0x180000 (256KB) ← SEC 固件起始
IRAM3: 0x180000 ~ 0x1C0000 (256KB)
IRAM4: 0x1C0000 ~ 0x200000 (256KB)

SEC 使用: IRAM2 + IRAM3 + 部分 IRAM4
```

#### 1.2.2 DDR SEC 区域（rpmsg 通讯）

| 属性 | 值 |
|------|-----|
| **物理地址** | 0x30000000 ~ 0x30600000 |
| **大小** | 6MB |
| **配置** | SEC_MEMBASE/SIZE |
| **用途** | rpmsg 缓冲区、共享内存 |
| **注意** | ❌ 不用于运行 SEC 代码 |

---

## 2. 内存布局设计

### 2.1 启动时序与内存初始化

```
T0: Boot ROM 执行
    └─ DDR 未初始化 ❌

T1: 加载 SEC 固件到 SRAM
    └─ DDR 仍不可用 ❌

T2: SEC 在 SRAM 运行
    ├─ .text 在 SRAM 执行
    ├─ .data/.bss 在 SRAM
    └─ Heap/栈在 SRAM

T3: SEC 初始化 DDR
    └─ DDR 可用 ✅

T4: AP 域启动
    └─ Linux 在 DDR 运行
```

### 2.2 SRAM 内存分配

```
SRAM (768KB):
├── .text    133KB  代码段
├── .rodata   26KB  只读数据
├── .data     58KB  初始化数据
├── .bss      51KB  未初始化数据 + 栈
└── Heap     525KB  动态内存
    ────────────────
    总计:     767KB
    剩余:       1KB
```

### 2.3 栈空间设计

#### 异常栈（5KB）

| 栈名 | 大小 | 用途 |
|------|------|------|
| FIQ Stack | 256B | FIQ 中断 |
| IRQ Stack | 512B | IRQ 中断 |
| SVC Stack | 2KB | 系统调用 |
| Abort Stack | 128B | 异常处理 |
| Undefined Stack | 128B | 未定义指令 |
| System Stack | 2KB | 系统模式 |

#### 任务栈（18KB）

| 任务 | 栈大小 | 配置项 |
|------|--------|--------|
| Main Thread | 4KB | CONFIG_MAIN_THREAD_STACK_SIZE |
| CLI Task | 10KB | CONFIG_CLI_STACK_SIZE |
| Timer Task | 8KB | FreeRTOS config |
| Idle Task | 2KB | FreeRTOS config |

---

## 3. Heap 动态内存设计

### 3.1 ⭐ Heap 动态计算机制

**关键特性**: Heap 大小由链接脚本动态计算，无需手动配置！

#### 计算原理

链接脚本 (`boards/bmu02_a00/secure/scripts/gcc/ddr.ld`):
```ld
_end = .;                      /* 静态内存结束位置 */

.heap : ALIGN(4) {
    _heap_start = .;           /* Heap 起始 = 静态内存结束 */
    *(.heap)
}

. = MEMBASE + MEMSIZE;         /* SRAM 结束 (0x200000) */
_heap_end = .;                 /* Heap 结束 = SRAM 结束 */
```

FreeRTOS 配置 (`rtos/FreeRTOS/Config/FreeRTOSConfig.h`):
```c
extern const unsigned long _heap_start;  /* 链接脚本符号 */
extern const unsigned long _heap_end;    /* 链接脚本符号 */

#define configTOTAL_HEAP_SIZE \
    ((size_t)(&_heap_end) - (size_t)(&_heap_start))
```

#### 计算公式

```
Heap 大小 = _heap_end - _heap_start
          = (CONFIG_MEM_BASE + CONFIG_MEM_SIZE) - 静态内存结束位置
          = 0x200000 - 静态内存结束位置
```

#### 动态调整示例

| 场景 | 静态内存 | Heap | 说明 |
|------|----------|------|------|
| 禁用 STR 前 | 387KB | 392KB | ddr_lowpower_image 占用 144KB |
| 禁用 STR 后 | 243KB | 525KB | 静态内存减少 → Heap 自动增加 |

**结论**: 
- ✅ 不需要手动配置 Heap 大小
- ✅ 编译器自动计算剩余空间
- ✅ 静态内存优化会自动增加 Heap
- ⚠️ Heap 上限 = SRAM 总大小 - 静态内存 - 栈空间

---

### 3.2 FreeRTOS 对 Heap 的需求分析

#### Heap 内存分配来源

FreeRTOS 从 heap 分配以下内核对象：

| 对象类型 | 分配函数 | 内存来源 |
|----------|----------|----------|
| 任务栈 | `xTaskCreate()` | Heap |
| 队列 | `xQueueCreate()` | Heap |
| 信号量 | `xSemaphoreCreate()` | Heap |
| 互斥量 | `xMutexCreate()` | Heap |
| 定时器 | `xTimerCreate()` | Heap |
| 事件组 | `xEventGroupCreate()` | Heap |
| 任务通知 | - | 任务控制块内（不占 Heap） |

#### 当前项目任务栈配置 (bmu02_a00)

| 任务 | 栈大小 | 来源 | 配置项 |
|------|--------|------|--------|
| Idle Task | 2KB | Heap | CONFIG_IDLE_TASK_STACK_SIZE |
| Timer Task | 8KB | Heap | configMINIMAL_STACK_SIZE × 4 |
| Main Thread | 4KB | Heap | CONFIG_MAIN_THREAD_STACK_SIZE |
| CLI Task | 10KB | Heap | CONFIG_CLI_STACK_SIZE |
| **任务栈合计** | **24KB** | - | - |

**异常栈（静态分配，不占 Heap）**:

| 栈名 | 大小 | 用途 | 配置项 |
|------|------|------|--------|
| FIQ Stack | 256B | FIQ 中断 | CONFIG_ARCH_FIQ_STACK_SIZE |
| SVC Stack | 2KB | 系统调用 | CONFIG_ARCH_SVC_STACK_SIZE |
| IRQ Stack | 512B | IRQ 中断 | 默认 |
| Abort Stack | 128B | 异常处理 | 默认 |
| Undefined Stack | 128B | 未定义指令 | 默认 |
| System Stack | 2KB | 系统模式 | 默认 |

#### FreeRTOS 内核对象开销估算

| 内核对象 | 单个大小 | 典型数量 | 总计 |
|----------|----------|----------|------|
| TCB (任务控制块) | ~300B | 5 tasks | ~1.5KB |
| Queue 控制块 | ~80B | 10 queues | ~0.8KB |
| Semaphore 控制块 | ~80B | 15 semaphores | ~1.2KB |
| Mutex 控制块 | ~100B | 5 mutexes | ~0.5KB |
| Timer 控制块 | ~80B | 5 timers | ~0.4KB |
| Event Group 控制块 | ~40B | 3 groups | ~0.1KB |
| **内核对象总计** | - | - | **~4.5KB** |

#### Heap 需求总结

```
最小 Heap 需求 = 任务栈 + 内核对象 + 碎片开销
              = 24KB + 4.5KB + ~3KB (碎片)
              ≈ 31KB (基础运行)
```

| 场景 | Heap 大小 | 使用率 | 状态 |
|------|-----------|--------|------|
| 当前可用 | 525KB | - | - |
| 基础需求 | ~31KB | ~6% | ✅ 充足 |
| 富余空间 | ~494KB | - | ✅ 可扩展 |

---

### 3.3 Heap 使用建议

#### 安全设计原则

1. **安全阈值**: 保持 Heap 使用率 < 70%
   ```
   推荐最大使用: 525KB × 0.7 ≈ 367KB
   预留应急空间: 525KB × 0.3 ≈ 158KB
   ```

2. **运行时监控**: 使用 FreeRTOS API
   ```c
   size_t freeHeap = xPortGetFreeHeapSize();
   size_t minEverFree = xPortGetMinimumEverFreeHeapSize();
   ```

3. **溢出检测**: 已启用配置
   ```c
   configCHECK_FOR_STACK_OVERFLOW = 2  /* 栈溢出检测 */
   configUSE_MALLOC_FAILED_HOOK = 1    /* 分配失败钩子 */
   ```

#### 扩展业务可用预算

| 新增对象 | 单个内存占用 | 可创建数量（预算 158KB） |
|----------|-------------|------------------------|
| 任务（4KB 栈） | ~4.5KB | ~35 个 |
| 队列（100 items） | ~200B | ~800 个 |
| 信号量 | ~80B | ~2000 个 |
| 互斥量 | ~100B | ~1600 个 |

#### 配置位置

```
任务栈配置:
├── boards/bmu02_a00/secure/projects/ref/defconfig
│   ├── CONFIG_MAIN_THREAD_STACK_SIZE=4096
│   ├── CONFIG_CLI_STACK_SIZE=10240
│   └── CONFIG_IDLE_TASK_STACK_SIZE=2048
│
└── rtos/FreeRTOS/Config/FreeRTOSConfig.h
    ├── configMINIMAL_STACK_SIZE = CONFIG_OS_MIN_STACK_SIZE
    └── configTIMER_TASK_STACK_DEPTH = configMINIMAL_STACK_SIZE * 4
```

---

### 3.4 分配方案

**FreeRTOS heap_4 算法**:
- ✅ 支持内存碎片合并
- ✅ 支持动态分配和释放
- ✅ 适合频繁分配/释放场景
- ⚠️ 分配时间不固定（非确定性）

---

## 4. 内存优化方案

### 4.1 ⭐ 优化方案 1: 禁用 STR 功能

**节省内存**: 144KB（推荐）

#### STR 功能说明

**STR = Suspend to RAM（挂起到内存）**

```
用途: 低功耗休眠模式
场景: 汽车电子、电池供电设备
占用: ddr_lowpower_image (144KB)
```

#### 使用场景判断

| 场景 | 建议 |
|------|------|
| 工业控制（常运行） | ✅ 禁用 STR |
| 桌面设备（一直通电） | ✅ 禁用 STR |
| 服务器设备 | ✅ 禁用 STR |
| 电池供电设备 | ❌ 保留 STR |
| 汽车电子 | ❌ 保留 STR |

#### 优化效果

| 项目 | 优化前 | 优化后 | 节省 |
|------|--------|--------|------|
| 静态内存 | 387KB | 243KB | **144KB** |
| .text | 275KB | 133KB | 142KB |
| .rodata | 168KB | 26KB | 142KB |

#### 影响说明

| 功能 | 影响 |
|------|------|
| STR 休眠 | ❌ 不可用 |
| 正常启动 | ✅ 不受影响 |
| 正常运行 | ✅ 不受影响 |
| AP 域 | ✅ 不受影响 |
| SEC 安全服务 | ✅ 不受影响 |

---

### 4.2 优化方案 2: 调整 Heap 大小

**节省内存**: 10-20KB

#### 原理

Heap 由链接脚本 `_heap_end` 符号决定：
```
Heap 大小 = _heap_end - _heap_start
```

#### 调整策略

**保守方案**（推荐）:
```
Heap: 525KB → 380KB
节省: 145KB
风险: 低（估算需求 27KB）
```

**激进方案**:
```
Heap: 525KB → 300KB
节省: 225KB
风险: 中（需评估实际使用）
```

---

### 4.3 优化方案 3: 精简栈大小

**节省内存**: 5-10KB

#### 栈优化建议

| 栈名 | 当前值 | 建议值 | 节省 |
|------|--------|--------|------|
| CLI Task | 10KB | 4KB | 6KB |
| Main Thread | 4KB | 2KB | 2KB |
| Timer Task | 8KB | 4KB | 4KB |

**注意**: 优化后需验证实际使用量

---

### 4.4 优化方案 4: 编译优化选项

**节省内存**: 5-20KB（需要测试验证）

#### 当前编译优化配置

| 优化项 | 配置位置 | 当前值 | 说明 |
|--------|----------|--------|------|
| `-O2` | `arch/arm/rules.mk:8` | 已启用 | 代码优化等级 |
| `-ffunction-sections` | `mk/build.mk:3` | 已启用 | 每个函数独立段 |
| `-fdata-sections` | `mk/build.mk:3` | 已启用 | 每个数据独立段 |
| `--gc-sections` | `mk/build.mk:4` | 已启用 | 链接时移除未使用段 |

#### 可选优化方案

**方案 A: 升级优化等级 `-Os`** (推荐)

```makefile
# arch/arm/rules.mk
ARCH_OPTFLAGS := -Os  # 当前是 -O2
```

| 效果 | 说明 |
|------|------|
| 代码大小减少 | **5-15%** (约 7-20KB) |
| 执行速度 | 略有下降 |
| 风险 | 低（适合嵌入式） |

**方案 B: LTO (Link Time Optimization)** (需测试)

```makefile
# mk/engine.mk
GLOBAL_COMPILEFLAGS += -flto
GLOBAL_LDFLAGS += -flto
```

| 效果 | 说明 |
|------|------|
| 代码大小减少 | **10-20%** (约 13-26KB) |
| 编译时间 | 增加 |
| 风险 | 中（需验证功能） |

#### ⚠️ Strip 优化说明

**重要**: `arm-none-eabi-strip` 对最终 binary **无效**！

```
原始 ELF:      2.3MB (含符号表、调试信息)
Strip 后 ELF:  315KB (移除符号表)
最终 binary:   243KB (objcopy -O binary 自动移除)

结论: objcopy 已自动移除所有 ELF 元数据，strip 无额外效果
```

#### 优化建议

1. **当前状态**: 已启用 `-O2` + `gc-sections`，效果良好
2. **如需进一步优化**: 升级 `-O2` → `-Os`（简单、低风险）
3. **谨慎使用**: LTO 需要完整测试验证
4. **无需操作**: strip 对 binary 无效果

---

### 4.5 优化方案 5: 移除测试功能

**节省内存**: 10-20KB

#### 可移除的模块

```
CONFIG_DRIVERS_EXAMPLE=y     # 示例驱动
CONFIG_MBOX_DEMO=y           # Mbox 演示
CONFIG_RPMSG_DEMO=y          # Rpmsg 演示
CONFIG_IRQ_TESTER=y          # IRQ 测试
CONFIG_CAN_TESTER=y          # CAN 测试
CONFIG_PIN_TESTER=y          # PIN 测试
CONFIG_DW_UART_TEST=y        # UART 测试
```

---

### 4.6 综合优化方案对比

#### 方案 A: 最小改动（推荐）

| 优化项 | 节省 | 风险 |
|--------|------|------|
| 禁用 STR | 144KB | 低 |
| 调整 Heap | 12KB | 低 |
| **总计** | **156KB** | ✅ |

**结果**: 387KB → 231KB，完全满足限制

#### 方案 B: 激进优化

| 优化项 | 节省 | 风险 |
|--------|------|------|
| 禁用 STR | 144KB | 低 |
| 优化栈 | 13KB | 中 |
| 移除测试 | 20KB | 低 |
| 优化配置表 | 5KB | 低 |
| **总计** | **182KB** | ✅ |

**结果**: 387KB → 205KB，剩余 563KB

---

## 5. ⭐ 关键设计约束与限制

### 5.1 硬件约束（不可修改）

```
┌─────────────────────────────────────────────────────────────┐
│ 硬件限制 - 设计前必读                                         │
├─────────────────────────────────────────────────────────────┤
│ ✅ SRAM 大小固定 768KB (CONFIG_MEM_SIZE 不可修改)            │
│ ✅ DDR 区域不可执行代码 (MPU XN 标志)                        │
│ ✅ 启动时 DDR 未初始化 (必须用 SRAM 运行代码)                 │
│ ✅ SRAM 物理地址固定 (0x140000 ~ 0x200000)                   │
│ ✅ IRAM = Internal SRAM (片内内存，不是 DDR)                 │
└─────────────────────────────────────────────────────────────┘
```

#### 配置限制

| 配置项 | 位置 | 限制 |
|--------|------|------|
| `CONFIG_MEM_SIZE` | `defconfig` | ❌ 不可修改 (硬件限制) |
| `CONFIG_MEM_BASE` | `defconfig` | ❌ 不可修改 (硬件地址) |
| `SEC_MEMBASE` | `image_cfg.h` | ⚠️ 仅用于 rpmsg，不可执行代码 |

---

### 5.2 内存布局限制

```
┌─────────────────────────────────────────────────────────────┐
│ 内存布局限制 - 编译时确定                                     │
├─────────────────────────────────────────────────────────────┤
│ ⚠️ 静态内存 (.text + .rodata + .data + .bss) 编译时固定     │
│ ✅ Heap 大小动态计算 (SRAM 总大小 - 静态内存)                │
│ ✅ 减少静态内存会自动增加 Heap                                │
│ ⚠️ 栈空间包含在 .bss 中，需预留足够空间                      │
└─────────────────────────────────────────────────────────────┘
```

#### 内存计算公式

```
总 SRAM = 768KB (固定)
静态内存 = .text + .rodata + .data + .bss (编译时确定)
Heap = 总 SRAM - 静态内存 (动态计算)

限制: 静态内存 + Heap ≤ 768KB
建议: 使用率 < 80% (约 614KB)
```

---

### 5.3 FreeRTOS 内存限制

```
┌─────────────────────────────────────────────────────────────┐
│ FreeRTOS 限制 - 运行时必须遵守                               │
├─────────────────────────────────────────────────────────────┤
│ ⚠️ 所有动态对象从 Heap 分配                                  │
│ ⚠️ 任务栈从 Heap 分配 (xTaskCreate)                         │
│ ⚠️ 队列/信号量/互斥量从 Heap 分配                            │
│ ✅ 任务通知在 TCB 内 (不占 Heap)                             │
│ ⚠️ Heap 使用率建议 < 70%                                     │
└─────────────────────────────────────────────────────────────┘
```

#### Heap 预算分配

```
当前 Heap: 525KB
基础需求: ~31KB (任务栈 + 内核对象 + 碎片)
可用预算: ~494KB (扩展业务)

安全阈值: 525KB × 0.7 ≈ 367KB
预留空间: 525KB × 0.3 ≈ 158KB
```

---

### 5.4 安全约束

```
✅ SEC 代码必须在可信内存（SRAM）
✅ DDR 可被外部 DMA 访问（不适合安全代码）
✅ 代码完整性要求（SRAM 无法被篡改）
✅ IRAM 是片内 SRAM（安全可信区域）
```

---

### 5.5 实时性约束

```
✅ SRAM 访问延迟固定（零等待状态）
✅ DDR 有刷新延迟（不适合实时响应）
✅ 确定性执行时间要求
✅ 中断响应时间要求
```

---

### 5.6 编译优化限制

```
┌─────────────────────────────────────────────────────────────┐
│ 编译优化限制 - 已自动优化                                    │
├─────────────────────────────────────────────────────────────┤
│ ✅ -O2 优化等级已启用                                        │
│ ✅ -ffunction-sections 已启用 (函数独立段)                   │
│ ✅ -fdata-sections 已启用 (数据独立段)                       │
│ ✅ --gc-sections 已启用 (移除未使用段)                       │
│ ⚠️ strip 对 binary 无效 (objcopy 已自动移除)                │
└─────────────────────────────────────────────────────────────┘
```

---

## 6. 常见设计误解

### ❌ 误解 1: SEC 代码运行在 DDR

**真相**: SEC 代码运行在 SRAM (IRAM)，DDR 只用于 rpmsg 通讯

### ❌ 误解 2: CONFIG_MEM_SIZE 可以调整

**真相**: CONFIG_MEM_SIZE 是硬件 SRAM 大小，不可修改

### ❌ 误解 3: SEC_MEMBASE 是代码运行地址

**真相**: SEC_MEMBASE 是 DDR 中给 rpmsg 预留的空间，不是代码运行位置

### ❌ 误解 4: 链接脚本 ddr.ld 链接 DDR 地址

**真相**: ddr.ld 链接 SRAM 地址，文件名误导

### ❌ 误解 5: IRAM 是外部 DDR 内存

**真相**: IRAM = Internal SRAM（片内内存）

---

## 7. 关键配置文件

| 文件 | 用途 |
|------|------|
| `boards/.../defconfig` | SRAM 配置（CONFIG_MEM_BASE/SIZE） |
| `boards/.../config.h` | 栈大小配置 |
| `boards/.../image_cfg.h` | DDR 区域配置（SEC_MEMBASE） |
| `boards/.../ddr.ld` | 链接脚本（链接 SRAM） |

---

## 8. 设计建议

### 8.1 SEC 域职责定位

**应该做**:
- ✅ 核心安全功能（最小化）
- ✅ 安全启动验证
- ✅ 密钥管理
- ✅ 权限控制
- ✅ 安全监控

**不应该做**:
- ❌ 复杂业务逻辑（迁移到 AP 域）
- ❌ 大数据处理（迁移到 AP 域）
- ❌ 非 Real-time 任务
- ❌ 丰富的调试功能

### 8.2 内存预算建议

```
SRAM (768KB) 建议分配:
├── 核心安全功能:  < 200KB
├── Heap:          200-400KB
└── 预留空间:      100-200KB

目标: 使用率 < 80% (614KB)
```

### 8.3 优化优先级

```
1️⃣ 禁用 STR 功能（144KB） - 低风险，高收益
2️⃣ 调整 Heap 大小（10-20KB） - 低风险
3️⃣ 精简栈大小（5-10KB） - 中风险，需验证
4️⃣ 移除测试功能（10-20KB） - 低风险
```

---

**文档维护**: BSP Team  
**反馈**: 如有问题或建议，请联系 BSP 团队