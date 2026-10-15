# SEC Domain Memory Quick Reference

**Version**: 3.0  
**Date**: 2026-04-07

---

## ⭐ Quick Optimization

**禁用 STR 功能 = 节省 144KB**

| 场景 | 建议 |
|------|------|
| 工业控制、桌面设备 | ✅ 禁用 STR |
| 电池供电、汽车电子 | ❌ 保留 STR |

**影响**: STR 休眠不可用，其他功能正常

---

## 架构要点

### SEC 域内存架构

```
┌─────────────────────────────────────┐
│ Boot Flow:                          │
│   Boot ROM → SRAM → Execute        │
│               ↑                     │
│          DDR 未初始化               │
├─────────────────────────────────────┤
│ SRAM (Code Running):                │
│   Address: 0x140000 ~ 0x200000     │
│   Size:    768KB (fixed)           │
│   Usage:   .text, .data, Heap      │
├─────────────────────────────────────┤
│ DDR SEC Region (rpmsg):            │
│   Address: 0x30000000              │
│   Size:    6MB                     │
│   Note:    NOT for running code!   │
└─────────────────────────────────────┘
```

### 关键事实

```
✅ SEC 代码运行在 SRAM (IRAM)
✅ IRAM = Internal SRAM (片内内存)
✅ CONFIG_MEM_SIZE 固定 768KB (硬件限制)
✅ DDR 区域不可执行代码 (MPU XN 标志)
❌ 链接脚本 ddr.ld 名字误导 (实际链接 SRAM)
```

---

## 内存布局

### SRAM 分配（优化后）

```
SRAM (768KB):
├── .text    133KB  代码段
├── .rodata   26KB  只读数据
├── .data     58KB  初始化数据
├── .bss      51KB  未初始化数据 + 栈
└── Heap     525KB  动态内存
    ────────────────
    总计:     767KB  ✅
    剩余:       1KB
```

### IRAM 组成

```
IRAM1: 0x100000 ~ 0x140000 (256KB)
IRAM2: 0x140000 ~ 0x180000 (256KB) ← SEC 固件
IRAM3: 0x180000 ~ 0x1C0000 (256KB)
IRAM4: 0x1C0000 ~ 0x200000 (256KB)
```

---

## 内存优化方案

### 方案对比

| 方案 | 节省 | 风险 | 推荐度 |
|------|------|------|--------|
| 禁用 STR | 144KB | 低 | ⭐⭐⭐⭐⭐ |
| 调整 Heap | 10-20KB | 低 | ⭐⭐⭐⭐ |
| 优化栈 | 5-10KB | 中 | ⭐⭐⭐ |
| 移除测试 | 10-20KB | 低 | ⭐⭐⭐ |

### 优化决策表

```
需要 STR 休眠功能？
├─ 是 → 保留 STR，优化栈/Heap
└─ 否 → 禁用 STR，节省 144KB ✅

剩余空间不足？
├─ 是 → 调整 Heap 大小
└─ 否 → 维持现状

需要更多空间？
├─ 是 → 优化栈 + 移除测试
└─ 否 → 已满足需求
```

---

## ⭐ 设计约束与限制

### 硬件约束（不可修改）

```
✅ SRAM 768KB 固定（CONFIG_MEM_SIZE 不可修改）
✅ DDR 不可执行代码（MPU XN 标志）
✅ 启动时 DDR 未初始化
✅ SRAM 地址固定 (0x140000 ~ 0x200000)
```

### 内存布局限制

```
⚠️ 静态内存编译时固定
✅ Heap 动态计算 (SRAM - 静态内存)
✅ 减少静态内存会自动增加 Heap
⚠️ 使用率建议 < 80%
```

### FreeRTOS 限制

```
⚠️ 任务栈从 Heap 分配
⚠️ 队列/信号量从 Heap 分配
✅ 任务通知在 TCB 内 (不占 Heap)
⚠️ Heap 使用率建议 < 70%
```

### 编译优化限制

```
✅ -O2 优化已启用
✅ -ffunction-sections 已启用
✅ -fdata-sections 已启用
✅ --gc-sections 已启用
⚠️ strip 对 binary 无效 (objcopy 已处理)
```

---

## 常见误解

| 误解 | 真相 |
|------|------|
| SEC 代码运行在 DDR | ❌ 运行在 SRAM |
| CONFIG_MEM_SIZE 可调整 | ❌ 硬件固定 |
| SEC_MEMBASE 是代码地址 | ❌ DDR rpmsg 空间 |
| ddr.ld 链接 DDR | ❌ 链接 SRAM |
| IRAM 是外部 DDR | ❌ 片内 SRAM |

---

## 配置文件

| 文件 | 用途 |
|------|------|
| `defconfig` | SRAM 配置 |
| `config.h` | 栈大小 |
| `image_cfg.h` | DDR 区域 |
| `ddr.ld` | 链接脚本 |

---

## Heap 动态计算

### 计算机制

```
Heap 大小 = _heap_end - _heap_start
          = 0x200000 - 静态内存结束位置
          
✅ 无需手动配置
✅ 编译器自动计算
✅ 静态内存减少 → Heap 自动增加
```

### 调整示例

| 场景 | 静态内存 | Heap | 说明 |
|------|----------|------|------|
| 禁用 STR 前 | 387KB | 392KB | 超限 |
| 禁用 STR 后 | 243KB | 525KB | ✅ |

---

## Heap 需求估算

### FreeRTOS 内存分配

| 对象 | 来源 | 单个大小 |
|------|------|----------|
| 任务栈 | Heap | 4-10KB |
| TCB | Heap | ~300B |
| 队列 | Heap | ~80B + buffer |
| 信号量 | Heap | ~80B |
| 互斥量 | Heap | ~100B |

### 当前配置 (bmu02_a00)

| 任务 | 栈大小 | 配置项 |
|------|--------|--------|
| Main Thread | 4KB | CONFIG_MAIN_THREAD_STACK_SIZE |
| CLI Task | 10KB | CONFIG_CLI_STACK_SIZE |
| Timer Task | 8KB | FreeRTOS config |
| Idle Task | 2KB | CONFIG_IDLE_TASK_STACK_SIZE |
| **任务栈合计** | **24KB** | - |

### 需求总结

```
基础需求: ~31KB (任务栈 + 内核对象 + 碎片)
当前 Heap: 525KB
富余空间: ~494KB ✅
安全阈值: < 70% 使用 (约 367KB)
```

---

## 设计建议

### SEC 域应该做

```
✅ 核心安全功能（最小化）
✅ 安全启动验证
✅ 密钥管理
✅ 权限控制
✅ 安全监控
```

### SEC 域不应该做

```
❌ 复杂业务逻辑 → AP 域
❌ 大数据处理 → AP 域
❌ 非 Real-time 任务
❌ 丰富调试功能
```

### 内存预算

```
SRAM 768KB 建议分配:
├─ 核心功能: < 200KB
├─ Heap:     200-400KB
└─ 预留:     100-200KB

目标: 使用率 < 80%
```

---

**See Also**: [SEC_MEMORY_DESIGN_GUIDE.md](SEC_MEMORY_DESIGN_GUIDE.md)  
**Last Updated**: 2026-04-07