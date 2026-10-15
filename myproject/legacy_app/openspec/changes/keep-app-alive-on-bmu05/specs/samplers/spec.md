## ADDED Requirements

### Requirement: R15 CCU 通信失败的系统重启门控

系统 MUST 在 CCU 通信中断时先确认**曾成功建立过通信**，才允许触发系统重启；对从未建立通信的 CCU，通信中断 MUST 仅产生日志与告警，MUST NOT 重启系统。

**既有行为**：`ess_sampler_main.c:1385-1431` 实现了通信中断自愈——CCU 心跳缺失 → `nCommFailTimes >= 10` → 每 30 秒调用 `CCU_Check_And_ReOpen_CommPort()` 并累加 `s_nCommFaileResetCounter` → 计数 `> 10`（约 330 秒）后发送 `EVENT_TYPE_SYSTEM_EXIT`，最终经 `event_manager.c:1663` 置 `bResetCmdPending`、由 `main.c:446` 执行 `_SYSTEM("reboot")` 重启整机。

**缺陷**：`ccu_sampler_main.c:446` 在初始化时把 `tmLastRecv_HEART` 预置为当前时间，因此 `ess_sampler_main.c:1388` 的 `tmNow - tmLastRecv_HEART > (CCU_HEART_PERIOD + 0.5)` 判定**无法区分"从未通信"与"通信中断"**。结果是任何未接入 CCU 的现场或台架都会在约 5.5 分钟后被无条件重启（bmu05 实测 4/4 复现）。

**要求**：

- 采样器 MUST 维护一个**只置位、不复位**的"曾成功通信"标记（建议为 `ROUGH_CCU_DATA` 的结构体字段，与 `kwhmeter_sampler` 的 `bLast_AllMeterCommOK` 同构）
- 该标记 MUST 在采样器一次性初始化时为 FALSE（`ccu_sampler_main.c:429` 的 `ZERO_POBJS(pCCU, 1)` 已覆盖）；MUST NOT 在 `InitCCUStuffData` 这类会被重复调用的"局部状态复位"函数中清零
- 该标记 MUST 在收到真实 CCU 心跳帧时置位（`ccu_protocol_parsers.c:752` 为唯一真实接收点）
- 发送 `EVENT_TYPE_SYSTEM_EXIT` MUST 以该标记为前置条件
- 未建立过通信时，通信中断 MUST 仅产生日志与告警，MUST NOT 触发系统重启
- 通信建立后发生的中断，其重启自愈行为 MUST 保持不变
- 30 秒周期的端口重试 MUST 保持不变，以便 CCU 恢复后可自愈

**参照实现**：同仓库 `kwhmeter_sampler` 已采用相同门控模式——`kwhmeter.c:261` 初始化为 FALSE，`rs485_whim_sampler_main.c:334/515/720` 在全表通信正常后置 TRUE，`:1107` 据此门控重启。
> **传输范围界定**：`Parse_HEART`（`ccu_protocol_parsers.c:737`）经 `INIT_NL_MSGINFO(pp, NL_MSG_HEART, Parse_HEART, 0xD0, 3)`（`ccu_sampler_main.c:557`）注册，仅由 `Find_NLMsgFrame()` 在 RS232（`ccu_com_232.c:107`）与 Ethernet（`ccu_com_eth.c:295,407`）路径调用；RPMSG 路径（`ccu_rpmsg_parsers.c`）使用独立的 `RPMSG_PARSER_RULE` 字典，**不解析 HEART**，因此不刷新 `tmLastRecv_HEART`。本看门狗读取的正是 `tmLastRecv_HEART`，故闩锁与看门狗**共用同一信号源**：若某部署的 CCU 走 RPMSG，`tmLastRecv_HEART` 自初始化起即不再更新，看门狗在本变更之前就已持续超时（属既有状况），变更后仅表现为不再重启整机。此为已知范围界定，非本次引入的新缺陷。

#### Scenario: 从未建立 CCU 通信时不重启

- **WHEN** 系统启动后 CCU 始终未响应（心跳帧从未到达）
- **THEN** 通信中断计数正常累加，日志与告警正常产生
- **AND** `EVENT_TYPE_SYSTEM_EXIT` MUST NOT 被发送
- **AND** 系统 MUST 保持运行，应用持续提供服务

#### Scenario: 通信建立后中断仍按设计重启

- **WHEN** CCU 曾成功通信（"曾通信"标记已置位）之后发生通信中断
- **THEN** 中断计数达到阈值后发送 `EVENT_TYPE_SYSTEM_EXIT`，系统按设计重启
- **AND** 每 30 秒的 `CCU_Check_And_ReOpen_CommPort()` 重试保持不变

#### Scenario: 标记只置位不复位

- **WHEN** 通信中断后 CCU 恢复并再次送达心跳
- **THEN** "曾通信"标记 MUST 保持置位状态
- **AND** 之后若再次中断，仍可触发重启自愈
