## ADDED Requirements

### Requirement: R12 空调采样（aircon_sampler）

系统 MUST 通过 `sampler_aircon.so`（Sampler_ID=11）经 RS485 Modbus RTU 采集空调/水冷机组（Device 951 / DeviceModel 950）的运行数据与故障位图，并承接 `thermal_control` 下发的控制设定，形成「策略下发 → 执行 → 回读」闭环。

**绑定与周期**（`src/config/System.sql:4346`、`aircon_sampler_main.h:14,30-35`）：
- Sampler_ID = 11，CommPort_ID = 13（`RS485_AIRCON` / `ttyS9`），串口参数 `9600,N,8,1`，采样周期 1000 ms
- 驱动来源：`aircon_sampler_main.c`，复用 `pv_inverter_sampler/pv_modbus_rtu.c` 的 Modbus RTU 层（`aircon_sampler/Makefile:17`）

**参数空间**：
- 同飞（TF）液冷机组：`TF_PARAM_*` 1770-1810 —— 启停/工作模式/控制对象、冷热设定温度与温差、液温/回液温/蒸汽进出口温、环境温、冷凝温、EEV 开度、泵速、压缩机转速、风机转速、水压，以及系统高低压、液位、泵过载/堵转/缺相/欠压/超压、热管过载、风机过载、加热带、泵空转保护等故障位（`aircon_sampler_main.h:41-87`）
- 旧机型兼容：`EU3_PARAM_LEGACY_*` 1583-1597，从 `sensor0` / `syssta0` / `comp0` / `compon` 四个位图按位解析压缩机与系统故障（`aircon_sampler_main.c:632-647`）

**通信失效门控**：连续失败达阈值后置通信中断状态（`aircon_sampler_main.h:26`）；风冷/液冷安装类型切换时清通信告警并门控采样。

#### Scenario: 周期采集与故障位图解析

- **WHEN** 采样周期到达（1000 ms）且 RS485 通信正常
- **THEN** 通过 Modbus RTU 读取 TF 参数块，按类型推送：枚举原样、浮点按 `raw*0.1` 缩放（如液温、压力），经 `pushParamEnum` / 浮点推送进入 DPR
- **AND** 对旧机型额外解析 `EU3_PARAM_LEGACY_*` 位图，缺失位在 V3.00 上固定推 0

#### Scenario: 通信连续失败

- **WHEN** 读取连续失败次数达到阈值
- **THEN** 标记为通信中断状态并触发相应告警
- **AND** 按安装类型（风冷/液冷）决定是否门控采样

#### Scenario: 承接热管理策略下发

- **WHEN** `thermal_control` 通过 `Push_VarValue(SETTYPE_INTERN_SETT, …, devId=951, param=2203, …)` 下发模式或设定值
- **THEN** `Control()` 回调将设定写入机组 Modbus 寄存器
- **AND** 回读结果以 `SETTYPE_RAW_BACK_CTRL` 回写 DPR


### Requirement: R13 辅助设备采样（auxdev_sampler）

系统 MUST 通过 `sampler_auxdev.so`（Sampler_ID=12）在共享 RS485 总线上同时驱动 **U3Z 水冷机**与 **IMLU2 灯带**两类辅助设备，并根据电池簇/系统状态自动判决灯带颜色。

**绑定与周期**（`src/config/System.sql:4347`、`auxdev_sampler_main.h:12,40-43`）：
- Sampler_ID = 12，CommPort_ID = 14（`RS485_AUXDEV` / `ttyS5` = UART5），`9600,N,8,1`，采样周期 500 ms
- 设备：水冷机 `CHILLER_DEV_ID = 961`（DeviceModel 960）；灯带 `LED_DEV_ID = 966`（DeviceModel 970）
- 驱动来源：`auxdev_sampler_main.c` + `u3z_protocol.c` + `imlu2_protocol.c`（`auxdev_sampler/Makefile:17`）
- 角色区分：`AUXDEV_ROLE_LED` 标识灯带分支（`auxdev_sampler_main.h:83`）

**灯带自动判决数据源**（`auxdev_sampler_main.h:67-77`）——只读取，不产生这些量：
- 电池簇：`LED_DPR_DEV_BATTGRP` = 501 / `LED_DPR_DEV_BATTGRP2` = 502，读取 `LED_DPR_PARAM_RUNSTATUS` = 550（4=充电 5=放电）与 `LED_DPR_PARAM_WORKSTATUS` = 552（3=故障），由 `st_batt_sampler` 经 RPMsg 推送
- 系统级：`LED_DPR_DEV_CCU` = 200，读取 `LED_DPR_PARAM_SYSSTATUS` = 103（2=Faulted），由 `ccu_sampler` 推送
- 集群数量：`LED_DPR_BATT_GROUP_NUM` = 2（两簇共用水冷机/灯带）
- 灯带通信状态：`LED_PARAM_COMMSTAT` = 1939

#### Scenario: 水冷机采集

- **WHEN** 采样周期到达（500 ms）
- **THEN** 通过 U3Z 协议在 UART5 上读取水冷机状态并推入 DPR（devId 961）
- **AND** 失败时按通信失效策略处理

#### Scenario: 灯带颜色自动判决

- **WHEN** 任一电池簇运行状态（550）或告警状态（552）变化，或系统状态（103）变为 Faulted
- **THEN** 组合 2 个电池簇与系统级状态，计算目标颜色
- **AND** 与 `lastSentColor` 比较，仅在变化时经 IMLU2 协议下发（`pendingColor` 缓存未确认的目标值）


### Requirement: R14 热管理策略采样（thermal_control）

系统 MUST 以**纯计算型采样器** `sampler_thermal.so`（Sampler_ID=13，无 CommPort）实现公式化热管理策略：读取电池与核间聚合数据，经 `T_decision` 公式 + 滞回状态机 + 时间保护计算出目标动作，再下发给空调采样器。

**绑定与周期**（`src/config/System.sql:4348`、`thermal_control_main.h:15,24-28`）：
- Sampler_ID = 13，`Used_CommPort = 0`（无物理端口，纯计算），采样周期 1000 ms
- 自身设备：`THM_DEV_ID = 992`（DeviceModel 950 空调域）
- 驱动来源：`thermal_control_main.c`（`thermal_control/Makefile:17`）
- 设计定位：替代 R 核原有的 13 级 if-else 判定（`thermal_control_main.h:12-14`）

**数据输入**：
- 核间聚合：`THM_RPMSG_DEV_ID = 500`（`thermal_control_main.h:159`）
- 电池簇基准：`THM_BATT_DEV_BASE = 501`（`:332`）
- 关键量：`THM_SRC_T_CELL_MAX` / `THM_SRC_T_CELL_MIN` / `THM_SRC_HV_STATUS`（`thermal_control_main.c:164-245`）

**状态机**（`thermal_control_main.h:33-40`）：`STANDBY` / `COOLING` / `DEEPCOOL` / `HEATING` / `EMERGENCY_COOL` / `SELFCIRC` 六态，带滞回与时间保护。

**输出**：`Push_VarValue` → devId `THM_DST_DEV_ID = 951`（空调采样器），使其经 Modbus RTU 落到机组寄存器。

**参数空间**：
- 公式系数与策略数据：设置量 2408-2431、2443-2451、2452-2493（属 DeviceModel 950）
- 模式开关：`THM_PARAM_AIRCON_CTRL_MODE = 2203`（devId 951，ENUM 0=Auto / 1=Manual）

#### Scenario: 策略计算与下发

- **WHEN** 采样周期到达（1000 ms）
- **THEN** 从 DPR 读取电池温度极值、高压状态与核间聚合数据
- **AND** 代入 `T_decision` 公式，经滞回状态机与时间保护得到目标状态
- **AND** 以 `Push_VarValue(SETTYPE_INTERN_SETT, …, devId=951, …)` 下发设定值

#### Scenario: Web 修改系数即时生效

- **WHEN** 通过 Web 修改 2408-2493 区间的公式系数或策略参数
- **THEN** 下一次 `Sample()` 周期重新读取设置量并采用新值
- **AND** 无需重启进程

#### Scenario: 手动模式屏蔽自动策略

- **WHEN** devId 951 的 2203 被置为 1（Manual）
- **THEN** 自动策略不覆盖人工设定值
