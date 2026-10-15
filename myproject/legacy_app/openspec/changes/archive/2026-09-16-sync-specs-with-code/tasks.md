## 1. 覆盖缺口核实

- [x] 1.1 核实 `aircon_sampler`：Sampler_ID=11、CommPort 13 (ttyS9)、Device 951/DevModel 950、TF 与 EU3_LEGACY 参数空间、通信失效门控
- [x] 1.2 核实 `auxdev_sampler`：Sampler_ID=12、CommPort 14 (ttyS5/UART5)、U3Z 水冷机 961/960 + IMLU2 灯带 966/970、LED 自动判决数据源
- [x] 1.3 核实 `thermal_control`：Sampler_ID=13、无 CommPort、6 态状态机、读写设备（500/501 → 951）、公式系数参数段与模式开关 2203
- [x] 1.4 复核 `reporters` capability 是否存在同类缺口（结论：无，R6-R9 已带编译状态标注，`ess_maintenance` 已并入 R10）

## 2. 规格写入

- [x] 2.1 编写 `specs/samplers/spec.md`，以 `## ADDED Requirements` 新增 R12 / R13 / R14
- [x] 2.2 编写 `proposal.md` / `tasks.md` / `design.md`

## 3. 验证

- [x] 3.1 `openspec validate sync-specs-with-code` 通过，无 ERROR
- [x] 3.2 每条新 Requirement 均含 ≥1 个 `#### Scenario:`，且 Requirement 文本含 MUST
- [x] 3.3 归档后 `openspec validate --all` 仍全绿（10/10）
