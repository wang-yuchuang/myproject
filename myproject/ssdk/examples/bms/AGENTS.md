# AGENTS.md - BMS Application Layer

## 1. OVERVIEW

Battery Management System application code. Core algorithms for cell monitoring, diagnosis, SOC calculation, and protection logic.

## 2. STRUCTURE

| Directory | Purpose |
|-----------|---------|
| `Functions/` | Core algorithms: calculation, diagnosis, parameters, SOC, statistics |
| `BSPSys/` | Board support: config, device drivers, utilities, user hooks |
| `Logics/` | Business logic: balance strategy, CAN messaging, protection tactics |
| `Addition/` | Extensions: user-defined tactics, operation authority |

## 3. WHERE TO LOOK

| Task | Location |
|------|----------|
| Cell voltage/current calculation | `Functions/DataCalc/` |
| Fault detection & filtering | `Functions/Diagnosis/` |
| SOC/SOH estimation | `Functions/SocLib/` |
| Parameter storage (EEPROM) | `Functions/Parameter/` |
| ADC/CAN/GPIO drivers | `BSPSys/Device/` |
| Board configuration | `BSPSys/BSPCfg/` |
| Balance control | `Logics/Balance/` |
| Protection tactics | `Logics/Tactics/` |
| CAN communication | `Logics/CanUser/`, `Functions/Canlib/` |

## 4. MODULE INTERFACES

| Module | Key Headers |
|--------|-------------|
| DataCalc | `CalcInterface.h`, `VoltLimCalc.h`, `CurrLimCalc.h` |
| Diagnosis | `DiagInterface.h`, `DiagFilter.h`, `DiagCheck.h` |
| Parameter | `ParaInterface.h`, `GroupPara.h`, `PublicPara.h` |
| SocLib | `CapEnerCorr.h` |
| SystemInfo | `GroupInfo.h`, `PublicInfo.h` (shared data structures) |
| BSP | `BSPConfig.h`, `BSPTypeDef.h` (custom types: u8/u16/u32/s8/s16/s32) |
| Tactics | `TacInterface.h`, `GroupProtect.h`, `GroupState.h` |

## 5. ENTRY POINTS

**Note**: `main.c` is in `boards/<board>/secure/projects/<project>/`, not here.

Init sequence (called from main):
1. `GroupInfoInit()` - `Functions/SystemInfo/GroupInfo.c` - initialize group data structures
2. `BSPDriverInit()` - `BSPSys/BSPCfg/BSPConfig.c` - initialize hardware drivers
3. `UserTaskCreate()` - `Functions/OSUser/OSUserHook.c` - create RTOS tasks

## 6. ANTI-PATTERNS (BMS-Specific)

- **Cluster indexing**: Many functions take `clunum` parameter (0 or 1). Static variables must be array-indexed by cluster, not single variables shared across clusters.
- **Diagnosis filter tables**: `gErrFilTab[clunum][index]` - always check `clunum < CLUTER_MAX_NUM` before access.
- **Parameter versioning**: Parameters have version suffixes (e.g., `gGBmuGenPara_102`). Match version when reading/writing.
- **Memory allocation**: Large tables use `ExtMalloc()` for external memory, not standard malloc.
- **Counter overflow**: Fault counters use `if(counter < 254) counter++` pattern to prevent rollover.