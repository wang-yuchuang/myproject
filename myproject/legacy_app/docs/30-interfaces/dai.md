# DAI（Data Access Interface）

> **性质**：接口契约
> **需求权威源**：暂无（无对应 capability，见 [§缺少 spec 的说明](#缺少-spec-的说明)）
> **代码事实基线**：[`openspec/.code-facts-snapshot.md`](../../openspec/.code-facts-snapshot.md)
> **对应提交**：`c48d724e`（最后核对时的代码版本）
> **状态**：🟠 部分

## 调用方 / 被调方

DAI 是 nccu_app 进程内的一层函数指针包装：它把"按 设备ID + 参数ID 取值/写值"这类高频跨模块数据访问，包装在 DPR（data_processor）之上，给动态加载的业务模块一个稳定的调用面。

- **被调方**：DAI 自己不持有数据。`Get`/`Set` 全部转发给 DPR 的函数指针（`g_pCharger->dataprocessor.Get/Set`，`src/data_interfaces/dai_loader.c:94`、`:576`），`Query` 则直接读运行数据 SQLite 库（`src/data_interfaces/dai_loader.c:818` 起）。
- **调用方（当前代码中确认的）**：8 个 reporter。加载器在每个 reporter 上注入接口指针：`pReporter->pDAI = &pCharger->dai`（`src/moduals/reporters/reporters_loader.c:48`），之后 reporter 通过 `pDAI->Get/Set/Regsiter_EvHandler` 取数、写参、订阅事件。
- **samplers 不经过 DAI**：采样器拿到的是 `pSampler->hDPR`，直接调用 DPR 的 `Get`（例：`src/moduals/samplers/inter_slave_sampler/inter_slv_cmd_handler.c:86`、`:93`）。在 samplers 源码目录 grep 不到任何 `pDAI` 注入或 `->dai` 访问（负证据，⚠️ 待核实：是否存在其他访问路径）。
- **宿主对象**：DAI 是全局单实例 `CHARGER` 的成员（`DAI dai;`，`src/include/data_model/charger_model.h:88`）。`dai_loader.c:5` 的 `g_pCharger` 全局指针指向它，因此 DAI 函数实现隐式绑定单实例，不支持多 `CHARGER` 并存。
- **加载时机**：nccu_app 六步加载的第 3 步（`src/main/main.c:165-171`，`Init_LoadDAI`）。这一步只做函数指针装配，不加载 .so。

```
reporters (8 个, .so)
    │  pReporter->pDAI
    ▼
DAI 结构体 (g_pCharger->dai)          samplers ──pSampler->hDPR──┐
    │  Get/Set (转发)                                            │
    ▼                                                            ▼
DPR 函数指针 (dataprocessor.Get/Set = Intern_Get/Intern_Set) ────┘
    │
    ├─ 内存中的参数/告警/设备表 (DPR 内部)
    └─ DAI_Query 直接走 SQLite (Data_Alarms / Data_ChargeMeterValues)
```

## 原型 / 数据格式

### 入口点（`dai_loader.c` 导出与装配）

| 入口 | 原型摘要 | 位置 | 说明 |
|---|---|---|---|
| `Init_LoadDAI` | `BOOL Init_LoadDAI(CHARGER* pCharger)` | 声明 `src/data_interfaces/dai_loader.h:6`；实现 `dai_loader.c:1045-1058` | 装配 5 个函数指针，总是返回 TRUE |
| `Get` | `ERRCODE_DAI (*)(HANDLE hSender, DAI_TYPE, int nArg1, int nArg2, void* pBuffer, UINT nBuffSize)` | typedef `src/include/data_model/dai_model.h:141-143`；实现 `static DAI_Get`，`dai_loader.c:91-519` | 单值/结构读取 |
| `Set` | 同上 + `IN int nTimeout` | typedef `dai_model.h:145-147`；实现 `dai_loader.c:521-657` | 写参数，`nTimeout` 直接透传 DPR（`dai_model.h:144` 定义 `DAI_NONEBLOCK_MODE = -1`，但 `DAI_Set` 内未见对 -1 的分支，DPR 侧如何处理非阻塞语义 ⚠️ 待核实） |
| `Query` | `ERRCODE_DAI (*)(HANDLE, QUERY_TYPE, QUERY_COND, void* pBuffer, UINT *pGetNum)` | typedef `dai_model.h:149-151`；实现 `static DAI_Query`，`dai_loader.c:747-964` | 批量历史查询，`*pGetNum` 既是入参容量上限也是出参实得条数（`:850`、`:899`、`:944`） |
| `Signal` | `ERRCODE_DAI (*)(HANDLE, EVENT_ARG*)` | typedef `dai_model.h:153`；实现 `dai_loader.c:966-979` | **占位桩**：正文被注释（"we do not support msg.send"，`:970`），无条件返回 `ERRCODE_DAI_INVALID_ARG` |
| `Regsiter_EvHandler` | `ERRCODE_DAI (*)(char*, HANDLE, EVENT_HANDLER, UINT, UINT, BOOL)` | typedef `dai_model.h:155-160`；实现 `dai_loader.c:981-1002` | 转发到 `dataprocessor.Register_EvHandler`（`:993`）。注意结构体成员名拼写就是 `Regsiter_EvHandler`（`dai_model.h:176`），调用侧必须用这个拼写 |

### 宿主结构体

`DAI` 结构体（`dai_model.h:170-179`）只有 5 个函数指针：`Get` / `Set` / `Query` / `Signal` / `Regsiter_EvHandler`。曾经计划的 `CalculateExp` / `FreeExp` 已被注释（`dai_model.h:177-178`，对应实现 `dai_loader.c:1004-1043` 也整体注释）。

### 读取缓冲：`PARAM_VAL_GET`

参数值读写共用一个缓冲结构（`dai_model.h:130-139`）：

```c
typedef struct tagGetValue {
    int     nGetSize;        // 调用前必须填：期望值字节数（例中为 4）
    VAR_VALUE val;           // 值本体；值无效时为最后有效值
    UINT    nValueAttribute; // 位标志
    time_t  tmRefresh;       // 刷新时间
} PARAM_VAL_GET;
```

属性位（`dai_model.h:134-136`）：`VALUE_ATTRIBUTE_VALID`(0x01) / `VALUE_ATTRIBUTE_DISPLAY`(0x02) / `VALUE_ATTRIBUTE_CTRLSETABLE`(0x04)。DAI 侧在返回前按参数元数据拼装这些位（`dai_loader.c:369-370`、`:390-392`）。

## `DAI_TYPE` 类型清单

枚举定义：`src/include/data_model/dai_model.h:7-40`，共 23 个值。"实现位置"指 `dai_loader.c` 中对应 `case` 分支的行号。

| `DAI_TYPE` | 枚举行 | Get 实现 | Set 实现 | 备注 |
|---|---|---|---|---|
| `DAITYPE_DEV_TOTAL_R` | :10 | :116 | （无） | 返回设备总数（int，要求 nBuffSize==4） |
| `DAITYPE_DEV_INFO_R` | :11 | :126 | （无） | 按下标取 `DEVICE_INFO` |
| `DAITYPE_PARAM_TOTAL_R` | :12 | :156 | （无） | nArg1 = PARAM_TYPE_STATUS/CONTROL/SETTING |
| `DAITYPE_PARAM_INFO_R` | :13 | :181 | （无） | 按下标取参数元数据 |
| `DAITYPE_PARAM_INFO_BY_PARAMID_R` | :39 | :228 | （无） | 按参数 ID 线性查找元数据（ Jimmy 2022 增） |
| `DAITYPE_ALARM_TOTAL_R` | :14 | :277 | （无） | |
| `DAITYPE_ALARM_INFO_R` | :15 | :287 | （无） | |
| `DAITYPE_GETDEVNAME_BYID_R` | :36 | :143 | （无） | 按设备 ID 取名称，截断到 32 字节 |
| `DAITYPE_GETAALARMNAME_BYID_R` | :37 | :304 | （无） | 按告警 ID 取名称 |
| `DAITYPE_GETAALARM_LEVEL_BYID_R` | :38 | :317 | （无） | 按告警 ID 取级别 |
| `DAITYPE_APP_VRESION_R` | :17 | :330 | （无） | 返回 `szSWVer` 字符串（要求 nBuffSize>=16；填充来源 ⚠️ 待核实） |
| `DAITYPE_PKG_VRESION_R` | :18 | :342 | （无） | 返回 `nPkgVer`（int；填充来源 ⚠️ 待核实） |
| `DAITYPE_STAT_PARAMVALUE_R` | :20 | :355 | （无） | 实时量读，nArg1=DevId, nArg2=ParamId |
| `DAITYPE_SETT_PARAMVALUE_RW` | :21 | :376 | :546 | **最常用**；Set 要求参数 `bCtrlSetAble` 为真（`:566-570`） |
| `DAITYPE_CTRL_PARAMVALUE_RW` | :22 | :420 | :611 | 控制量读写，Set 同样检查 `bCtrlSetAble`（`:629-633`） |
| `DAITYPE_READBACK_VAL` | :23 | :398 | （无） | 读写回值（GETTYPE_READBACK_VAR） |
| `DAITYPE_ALARMSTAT_R` | :24 | :442 | （无） | 直接转发 GETTYPE_ALMSTATE |
| `DAITYPE_COMMPORT_R` | :30 | :447 | （无） | 按端口号取整个 `COMMPORT` 结构 |
| `DAITYPE_VIRTUAL_PARAMVALUE_W` | :32 | （无） | :584 | reporter 写虚拟状态量（SETTYPE_LOGIC_SP），**无 bCtrlSetAble 检查**（`:593-596` 注释） |
| `DAITYPE_COMM_DATA_DUMP` | :34 | （无） | :533-542 | 在 switch 之前特判，转发 `DumpOut`（实现在 `src/publib/iDataBase.c:740`） |
| `DAITYPE_GETLOCALLIST_VERSION` | :26 | （无） | （无） | **未实现**：case 整体注释（`:464-512`），落入 default 返回 INVALID_ARG |
| `DAITYPE_GETLOCALLIST_TOTAL` | :27 | （无） | （无） | 同上（OCPP LocalList 遗留） |
| `DAITYPE_GETLOCALLIST_ITEMS` | :28 | （无） | （无） | 同上 |

`QUERY_TYPE`（`dai_model.h:42-51`）6 个值中，`DAI_Query` 只实现了 3 个：`QUERYTYPE_METERVALUS`（`:818`）、`QUERYTYPE_ALARM`（`:866`，注意实现忽略了查询条件，直接 `SELECT * FROM Data_Alarms`）、`QUERYTYPE_ACTV_ALARM`（`:911`，仅取进行中告警）。`QUERYTYPE_CHARGEREC` 整体注释（`:765-817`），`QUERYTYPE_HISEVENT` / `QUERYTYPE_HISDATA` 落入 default。

## 语义与约束

### 参数读写的 (DevId, ParamId) 模式

`DAITYPE_STAT_PARAMVALUE_R` / `SETT_PARAMVALUE_RW` / `CTRL_PARAMVALUE_RW` / `READBACK_VAL` / `VIRTUAL_PARAMVALUE_W` 全部采用同一调用约定：

- `nArg1` = 设备 ID，`nArg2` = 参数 ID；
- `pBuffer` 指向调用方栈上的 `PARAM_VAL_GET`，且**调用前必须先填 `nGetSize`**（DAI 把它作为值大小透传 DPR，`dai_loader.c:372-373`）；
- `nBuffSize` 必须等于 `sizeof(PARAM_VAL_GET)`，否则直接返回 `ERRCODE_DAI_INVALID_ARG`（`:356-359` 等）；
- DAI 先用 `GetParam_ByPR`（实现在 `src/moduals/data_processor/dpr/config_mangmt.c:16`）做元数据查找，找不到返回 INVALID_ARG（`:362-366`）。

### 具体消费示例（reporter 读平台类型）

reporters 加载器用它读平台类型参数 `1/10`，决定 6/7/8/9 号 reporter 是否加载（`src/moduals/reporters/reporters_loader.c:80-94`）：

```c
valGet.nGetSize = 4;
pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
    1, 10, &valGet, sizeof(PARAM_VAL_GET));
nPlatformType = valGet.val.emValue;
if (valGet.val.emValue == 0) { /* 跳过 6/7/8/9 号 reporter */ }
```

再如 `GetLCCfg_Reporter` 批量读取负载配置（`src/moduals/reporters/reporters_pubfun.c:32-82`）：系统类型 `1/100`、应用场景 `1/7`、客户端 LC 索引 `1/42`、LC 参数 `1001/3313-3315`、柜表 `407/401`、关口表 `408/401`，每项失败时按缺省值兜底继续。

reporters 需求 spec 中引用的 DAI 参数（只链接不复制断言，见 `openspec/specs/reporters/spec.md`）：平台类型 `1/10`（:45）、控制开关 `1/16`（:143）、`1/47`（:203）、`1/51`（:315）、`1/42`（:333）、`1/57`（:356）。

### 写约束

- `DAITYPE_SETT_PARAMVALUE_RW` / `DAITYPE_CTRL_PARAMVALUE_RW` 写入前检查 `pParam->bCtrlSetAble`，为假则返回 `ERRCODE_DAI_NOTSETTABLE` 并记错误日志（`dai_loader.c:566-570`、`:629-633`）。
- 曾经存在的"可写表达式"（settable/controlable expression）检查被注释（`:553-563`、`:618-628`），当前唯一闸门是 `bCtrlSetAble` 标志。

## 错误处理

`ERRCODE_DAI` 枚举（`dai_model.h:118-128`）8 个值：`OK` / `INVALID_ARG` / `NOTSETTABLE` / `NOTCONTROABLE` / `ARG_MEMNOTENOUGH` / `EXECUTE_ERR` / `TIMEOUT` / `NOTSUPPORTED`。

DAI 与 DPR 的错误码经 `GetErrCode` 映射（`dai_loader.c:7-27`）：

```
ERR_DPR_INVALID_ARG / WRONG_SIZE / MUTEX_ERR / TIMEOUT  →  ERRCODE_DAI_INVALID_ARG
ERR_DPR_NOTSUPPORTED                                    →  ERRCODE_DAI_NOTSUPPORTED
其余（含 ERR_DPR_OK）                                    →  ERRCODE_DAI_OK（default 分支）
```

注意映射表中 `ERR_DPR_MUTEX_ERR`、`ERR_DPR_TIMEOUT` 也折叠为 `INVALID_ARG`，调用方无法从 DAI 返回值区分"参数错"与"锁/超时错"。

## 与 DPR `Intern_Get`/`Intern_Set` 的关系

data-processor spec 的 R6 定义了 DPR 自身的跨模块接口 `Intern_Get`/`Intern_Set`（`openspec/specs/data-processor/spec.md:219-267`；实现在 `src/moduals/data_processor/dpr_loader.c:26`、`:648`，装配于 `:1486-1487`）。**DAI 与 R6 不是同一个接口，而是叠在其上的包装层**：

| 维度 | DPR `Intern_Get`/`Intern_Set`（R6） | DAI（本页） |
|---|---|---|
| 调用面 | `GETTYPE_*`/`SETTYPE_*` 枚举 + `VAR_VALUE` | `DAITYPE_*` 枚举 + `PARAM_VAL_GET` |
| 调用方 | samplers 直接调用（经 `pSampler->hDPR`） | reporters 经 `pReporter->pDAI` |
| 错误码 | `ERRCDDE_DPR` | `ERRCODE_DAI`（经 `GetErrCode` 折叠映射） |
| 元数据 | 不查，按枚举直接分发 | 先查 `GetParam_ByPR` 元数据，附送值属性位与刷新时间 |
| 写闸门 | 内部各 SETTYPE 自行处理 | 统一 `bCtrlSetAble` 检查 + 日志 |
| 历史查询 | 有（`GETTYPE_CHARGE_REC` 未实现，见 R6 注意事项 spec.md:251） | `DAI_Query` 独立直读 SQLite |

转发链举例：reporter 调 `pDAI->Get(DAITYPE_SETT_PARAMVALUE_RW, dev, param)` → DAI 查元数据、拼属性位 → 以 `GETTYPE_ST_VAR` 调 `dataprocessor.Get`（`dai_loader.c:394`）→ 落到 DPR 的 `Intern_Get`。写方向同理：`SETT_PARAMVALUE_RW` → `SETTYPE_INTERN_SETT`（`:576`），`VIRTUAL_PARAMVALUE_W` → `SETTYPE_LOGIC_SP`（`:602`），`CTRL_PARAMVALUE_RW` → `SETTYPE_INTERN_CTRL`（`:639`）。

## 缺少 spec 的说明

> ⚠️ `src/data_interfaces/` 目录**没有**对应的 `openspec/specs/<capability>/spec.md`。`openspec/specs/` 现存 7 个 capability：app-lifecycle、comm-ports、data-processor、publib、reporters、samplers、system-hand，均不覆盖 DAI。
>
> 因此本页**仅描述现状**（descriptive-only），不是需求契约：文中所有行为都以 `对应提交` 头注明的代码版本为准，改代码后必须回来重新核对。若后续要给 DAI 立capability，建议从本页的 `DAI_TYPE` 清单与转发链中提炼验收断言。

## 版本与兼容性

- DAI 无独立版本号。枚举值的演进靠注释留痕：`DAITYPE_PKG_VRESION_R`（20200307）、`DAITYPE_GETDEVNAME_BYID_R` 等三个（20210426，"for MaintainBE reporter"）、`DAITYPE_PARAM_INFO_BY_PARAMID_R`（2022.325）（`dai_model.h:18`、`:35-39`）。
- 接口以**结构体内函数指针**形式分发，没有 dlsym 动态加载环节（与 comm-ports 的 6 函数 HAL 不同）：reporter .so 拿到的是加载器注入的指针，链接期即依赖 `dai_model.h` 的结构布局。增删 DAI 成员会破坏已编译 reporter 的 ABI。
- `Signal` 入口当前是必失败的桩（`dai_loader.c:966-979`），消费方不应依赖它成功返回。

## 代码位置

- 定义：`src/include/data_model/dai_model.h:7-40`（`DAI_TYPE`）、`:42-51`（`QUERY_TYPE`）、`:118-128`（`ERRCODE_DAI`）、`:130-139`（`PARAM_VAL_GET`）、`:141-160`（函数指针 typedef）、`:170-179`（`DAI` 结构体）；宿主成员 `src/include/data_model/charger_model.h:88`
- 实现：`src/data_interfaces/dai_loader.c`（`DAI_Get` :91、`DAI_Set` :521、`DAI_Query` :747、`DAI_Signal` :966、`DAI_RegisterHandler` :981、`Init_LoadDAI` :1045）；编译进主进程 `src/main/Makefile:14`
- 下游依赖：`src/moduals/data_processor/dpr_loader.c:26`（`Intern_Get`）、`:648`（`Intern_Set`）、`:1486-1487`（装配）；`src/moduals/data_processor/dpr/config_mangmt.c:16`（`GetParam_ByPR`）；`src/publib/iDataBase.c:740`（`DumpOut`）
- 注入点：`src/moduals/reporters/reporters_loader.c:48`；加载时机：`src/main/main.c:165-171`
