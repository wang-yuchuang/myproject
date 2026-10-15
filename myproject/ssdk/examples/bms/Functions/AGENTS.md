# AGENTS.md - Functions 模块指南

## OVERVIEW
BMS 核心功能模块，包含数据计算、诊断、参数管理、测量、SOC/SOE 等核心业务逻辑。

## STRUCTURE

| 模块 | 功能 |
|------|------|
| DataCalc | 电压/电流计算、特性计算、限值计算 |
| Diagnosis | 故障诊断、滤波处理、状态检查 |
| Parameter | 参数读写、分组参数、EEPROM 存储 |
| Measurement | AFE 采样、电流采样、端口检测 |
| SocLib | SOC/SOE 计算、容量能量校正 |
| Statistics | 能量统计、时间统计、DOD 记录 |
| EnergyCalc | SOC/SOE/SOH 显示、容量计算、电流积分 |
| SystemInfo | 系统枚举、工作状态、公共信息定义 |
| OSUser | RTOS 任务创建钩子、线程管理 |
| PCS | 外设控制（CAN/Delay/LCCU/ModN2302/ModN2402） |
| Canlib | CAN 通信封装、消息收发 |
| IccMsg | 核间消息创建、处理、周期发送 |

## WHERE TO LOOK

| 需求 | 模块 |
|------|------|
| 电压/电流限值计算 | DataCalc/VoltLimCalc.h, CurrLimCalc.h |
| 故障诊断与滤波 | Diagnosis/DiagInterface.h, DiagFilter.h |
| 参数读写操作 | Parameter/ParaInterface.h, ParaRead.h |
| SOC/SOE 计算 | SocLib/SocInterface.h, EnergyCalc/EnerInterface.h |
| AFE/电流采样 | Measurement/CellAFESamp.h, CurrSample.h |
| 系统状态枚举 | SystemInfo/PublicInfo.h |
| CAN 消息处理 | Canlib/CanUser.h, IccMsg/IccMsgInterface.h |
| 任务创建 | OSUser/OSUserHook.h |

## KEY INTERFACES

```
DataCalc/CalcInterface.h      - 计算模块主接口
Diagnosis/DiagInterface.h     - 诊断模块主接口
Parameter/ParaInterface.h     - 参数模块主接口
Measurement/MeaInterface.h    - 测量模块主接口
SocLib/SocInterface.h         - SOC 模块主接口
EnergyCalc/EnerInterface.h    - 能量模块主接口
SystemInfo/PublicInfo.h       - 系统公共信息定义
IccMsg/IccMsgInterface.h      - 核间消息主接口
```

## DATA FLOW

```
Measurement → DataCalc → Diagnosis → SystemInfo
     ↓           ↓          ↓
Parameter ← ← ← ← ← ← ← ← ←
     ↓
SocLib/EnergyCalc → Statistics
     ↓
IccMsg/Canlib → 外部通信
```

测量数据经 DataCalc 处理后送 Diagnosis 诊断，结果存入 SystemInfo。
Parameter 提供配置数据支撑各模块计算。
SocLib/EnergyCalc 基于测量和参数计算 SOC/SOE。
IccMsg/Canlib 负责数据对外输出。

## API PATTERN

函数命名：`<Module><Action>API`（如 `DiagGetErrStateAPI`、`CalcGetVoltAPI`）