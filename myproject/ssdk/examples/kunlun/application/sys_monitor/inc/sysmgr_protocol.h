/**
 * @file sysmgr_protocol.h
 * @brief sysmgr 监管协议定义 — 双核共享
 *
 * R 核 source/ssdk/examples/kunlun/application/sys_monitor/inc/sysmgr_protocol.h
 * A 核 meta-semidrive/recipes-bsp/app-sysmgr/app-sysmgr/inc/sysmgr_protocol.h
 * 两份必须完全一致。
 *
 * 关联变更: openspec/changes/add-system-mgmt-service/
 */
#ifndef SYSMGR_PROTOCOL_H
#define SYSMGR_PROTOCOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 消息头 (固定 8 字节) ---- */
typedef struct __attribute__((packed)) {
    uint8_t  msg_type;     /* 命令类型 (见下方枚举) */
    uint8_t  reserved;
    uint16_t payload_len;  /* 载荷长度 */
    uint32_t seq;          /* 序列号 (请求-响应匹配) */
} sysmgr_hdr_t;

/* ---- 消息类型 (msg_type) ---- */
enum {
    /* A 核 → R 核 (AP 请求) */
    SYSMGR_GET_VERSION      = 0x01,  /* → RESPONSE_VERSION */
    SYSMGR_GET_READY_STATUS = 0x02,  /* → RESPONSE_READY_STATUS */
    SYSMGR_PING             = 0x03,  /* → RESPONSE_PONG */
    SYSMGR_PLANNED_REBOOT   = 0x04,  /* → RESPONSE_REBOOT_ACK */
    /* 0x05 TIME_SYNC 已砍掉 v1 */
    SYSMGR_GET_RCORE_STATUS = 0x06,  /* → RESPONSE_RCORE_STATUS */
    SYSMGR_GET_SLOT_STATUS  = 0x07,  /* → RESPONSE_SLOT_STATUS (R 核当前活动槽) */
    SYSMGR_CHANNEL_STATUS   = 0x08,  /* → RESPONSE_CHANNEL_ACK */

    /* OTA 升级 (rauc-xip-ota) */
    SYSMGR_OTA_BEGIN        = 0x09,  /* → RESPONSE_OTA_BEGIN_ACK (声明+启动擦除) */
    SYSMGR_OTA_DATA         = 0x0A,  /* → RESPONSE_OTA_DATA_ACK (流式 chunk) */
    SYSMGR_OTA_COMMIT       = 0x0B,  /* → RESPONSE_OTA_COMMIT_ACK (回读校验+切 GPT) */
    SYSMGR_OTA_STATUS       = 0x0C,  /* → RESPONSE_OTA_STATUS (进度查询) */
    SYSMGR_MARK_GOOD        = 0x0D,  /* → RESPONSE_MARK_GOOD (S3.3: A 核确认 R 核 boot 成功后
                                     * 重置运行槽 GPT retry/successful, 防 ALWAYS_ROLL_BACK_CHECK
                                     * 4 次 boot 自动回滚) */

    /* R 核 → A 核 (响应) */
    SYSMGR_RESPONSE_VERSION       = 0x81,
    SYSMGR_RESPONSE_READY_STATUS  = 0x82,
    SYSMGR_RESPONSE_PONG          = 0x83,
    SYSMGR_RESPONSE_REBOOT_ACK    = 0x84,
    SYSMGR_RESPONSE_RCORE_STATUS  = 0x86,
    SYSMGR_RESPONSE_SLOT_STATUS   = 0x87,
    SYSMGR_RESPONSE_CHANNEL_ACK   = 0x88,
    SYSMGR_RESPONSE_OTA_BEGIN_ACK  = 0x89,
    SYSMGR_RESPONSE_OTA_DATA_ACK   = 0x8A,
    SYSMGR_RESPONSE_OTA_COMMIT_ACK = 0x8B,
    SYSMGR_RESPONSE_OTA_STATUS     = 0x8C,
    SYSMGR_RESPONSE_MARK_GOOD      = 0x8D,
};

/* MARK_GOOD 响应 */
typedef struct __attribute__((packed)) {
    uint8_t ok;       /* 1=已 mark (运行槽 GPT retry 重置) */
    uint8_t slot;     /* 被 mark 的槽 (0=A, 1=B, 0xFF=失败) */
} sysmgr_mark_good_ack_t;

/* ---- 各消息的载荷结构体 ---- */

/* GET_VERSION 响应 */
typedef struct __attribute__((packed)) {
    char version[64];   /* 统一版本串 SSDK_VERSION ({CHIP}-{BOARD}-{BODY}, ≤63B, NUL 结尾) */
    char board_name[32];/* 适配板名 CONFIG_BOARD_NAME_STR (GAP-3 AP/R 形态比对), 如 bmu05_a00 */
} sysmgr_version_t;

/* GET_READY_STATUS 响应 */
typedef struct __attribute__((packed)) {
    uint8_t ready;          /* 1 = 全部业务 endpoint 就绪 */
    uint8_t endpoint_count; /* 已注册 iccmsg endpoint 数 */
} sysmgr_ready_status_t;

/* PING 响应 (PONG) */
typedef struct __attribute__((packed)) {
    uint32_t rcore_uptime_ms; /* R 核 uptime (osKernelGetTickCount) */
} sysmgr_pong_t;

/* PLANNED_REBOOT 响应 (REBOOT_ACK) */
typedef struct __attribute__((packed)) {
    uint8_t  ack;              /* 1 = 接受 */
    uint16_t grace_period_sec; /* 宽限期 (默认 60) */
} sysmgr_reboot_ack_t;

/* GET_SLOT_STATUS 响应 */
/* boot_media: R 核固件启动介质 (0=eMMC 加载 OCRAM, 1=NOR XIP; 读取失败填 0xFF) */
enum {
    SYSMGR_BOOT_MEDIA_EMMC    = 0,
    SYSMGR_BOOT_MEDIA_NOR_XIP = 1,
    SYSMGR_BOOT_MEDIA_UNKNOWN = 0xFF,
};
typedef struct __attribute__((packed)) {
    uint8_t active_slot;    /* 0=A, 1=B */
    uint8_t retry_count;    /* 当前槽 retry 计数 (读取失败填 0xFF) */
    uint8_t boot_media;     /* 启动介质 (SYSMGR_BOOT_MEDIA_*; 保留字节改造) */
    uint8_t reserved[1];
} sysmgr_slot_status_t;

/* CHANNEL_STATUS 载荷 (A→R 通知) */
typedef struct __attribute__((packed)) {
    char     name[32];    /* endpoint 名称 */
    uint8_t  connected;   /* 1 = 已连接, 0 = 已断开 */
} sysmgr_channel_status_t;

/* CHANNEL_ACK 响应 */
typedef struct __attribute__((packed)) {
    uint8_t ack;  /* 1 = 已更新通道状态表 */
} sysmgr_channel_ack_t;

/* ---- R 核状态 (GET_RCORE_STATUS 响应载荷) ---- */

/* board_halt_reason 枚举 (与 SSDK board.h 一致) */
enum {
    SYSMGR_BOOT_UNKNOWN      = 0,
    SYSMGR_BOOT_POR          = 1,  /* Cold-boot */
    SYSMGR_BOOT_HW_WATCHDOG  = 2,  /* HW watchdog timer */
    SYSMGR_BOOT_LOWVOLTAGE   = 3,
    SYSMGR_BOOT_HIGHVOLTAGE  = 4,
    SYSMGR_BOOT_THERMAL      = 5,
    SYSMGR_BOOT_SW_RECOVERY  = 6,
    SYSMGR_BOOT_SW_RESET     = 7,  /* Generic Software Initiated Reboot */
    SYSMGR_BOOT_SW_WATCHDOG  = 8,
    SYSMGR_BOOT_SW_PANIC     = 9,
    SYSMGR_BOOT_SW_UPDATE    = 10,
    SYSMGR_BOOT_SW_GLOBAL_POR = 11,
};

#define RCORE_TASK_MAX 32

typedef struct __attribute__((packed)) {
    /* 基本信息 */
    uint8_t  boot_reason;       /* SYSMGR_BOOT_* 枚举 */
    uint8_t  ssdk_ready;        /* 业务 endpoint 全部就绪 */
    uint8_t  endpoint_count;    /* 已注册 endpoint 数 */
    uint8_t  reserved;
    uint32_t uptime_ms;         /* R 核 uptime */
    /* 内存 */
    uint32_t heap_free;         /* xPortGetFreeHeapSize */
    uint32_t heap_min;          /* xPortGetMinimumEverFreeHeapSize */
    /* WDT */
    uint32_t wdt_timeleft_sec;  /* watchdog_get_timeleft */
    /* 通讯统计 */
    uint32_t iccmsg_tx_cnt;
    uint32_t iccmsg_rx_cnt;
    uint32_t iccmsg_err_cnt;    /* ack_timeout + reassembly_fail + crc_fail */
    /* task 监管统计 */
    uint8_t  monitored_task_count;
    uint8_t  task_crash_total;
    uint16_t reserved2;
} sysmgr_rcore_status_t;  /* ~40 字节 */

/* ---- 通道名称最大长度 ---- */
#define SYSMGR_CHANNEL_NAME_MAX 32

/* ==================== OTA 升级协议 (rauc-xip-ota) ==================== */

/* OTA 阶段枚举 (ota_status/响应 phase 字段) — staging 模式: 先 DDR 缓存传输, 后擦写 flash */
enum {
    SYSMGR_OTA_PHASE_IDLE          = 0,
    SYSMGR_OTA_PHASE_RECEIVING     = 1,  /* DDR 缓存接收中 (flash 零接触) */
    SYSMGR_OTA_PHASE_RECEIVED      = 2,  /* 当前镜像收满 (可接下一镜像 BEGIN 或最终 COMMIT) */
    SYSMGR_OTA_PHASE_VERIFYING     = 3,  /* DDR 缓存 CRC 双校验 */
    SYSMGR_OTA_PHASE_WRITING       = 4,  /* flash 擦写 (ssystem_X + vbmeta-ssdk_X, 从 DDR 缓存) */
    SYSMGR_OTA_PHASE_VERIFY_FLASH  = 5,  /* flash 回读 CRC 双校验 */
    SYSMGR_OTA_PHASE_SWITCHING     = 6,  /* 切 NOR GPT (活动槽永不触碰) */
    SYSMGR_OTA_PHASE_DONE          = 7,
    SYSMGR_OTA_PHASE_FAILED        = 0xFF,
};

/* OTA_BEGIN 拒绝原因 */
enum {
    SYSMGR_OTA_REJ_NONE            = 0,
    SYSMGR_OTA_REJ_BUSY            = 1,  /* 已有 OTA 进行中 */
    SYSMGR_OTA_REJ_TARGET_ACTIVE   = 2,  /* target_slot == 当前活动槽 (非法) */
    SYSMGR_OTA_REJ_SIZE_INVALID    = 3,  /* size 超限/为 0 */
    SYSMGR_OTA_REJ_NOT_READY       = 4,  /* 系统未就绪 (未握手) */
};

/* OTA 镜像标识 (双镜像会话: 同一 target_slot 成对传输) */
enum {
    SYSMGR_OTA_IMAGE_SSYSTEM    = 0,   /* 大镜像: ssystem_X (主固件) */
    SYSMGR_OTA_IMAGE_VBMETA_SSDK = 1,  /* 小镜像: vbmeta-ssdk_X (per-slot hash, 须在 ssystem 之后) */
};

/* OTA_BEGIN 请求载荷 (A→R) */
typedef struct __attribute__((packed)) {
    uint32_t image_size;     /* 本镜像字节数 */
    uint32_t crc32;          /* 本镜像 CRC32 (IEEE) */
    uint8_t  target_slot;    /* 0=A, 1=B —— R 核校验 == 非活动槽 */
    uint8_t  image_id;       /* SYSMGR_OTA_IMAGE_* */
    uint8_t  reserved[2];
} sysmgr_ota_begin_t;

/* OTA_BEGIN 响应 (R→A) */
typedef struct __attribute__((packed)) {
    uint8_t ack;             /* 1=接受(已开始擦除), 0=拒绝 */
    uint8_t phase;           /* sysmgr_ota_phase_t */
    uint8_t reject_reason;   /* SYSMGR_OTA_REJ_* */
    uint8_t reserved;
} sysmgr_ota_begin_ack_t;

/* OTA_DATA 载荷 (A→R, 单帧内) */
typedef struct __attribute__((packed)) {
    uint32_t offset;         /* 镜像内字节偏移 */
    uint16_t len;            /* data[] 长度 */
    uint8_t  data[];         /* len 字节 */
} sysmgr_ota_data_t;

/* OTA_DATA 响应 (R→A, 每 chunk ACK) */
typedef struct __attribute__((packed)) {
    uint8_t ack;             /* 1=已写, 0=失败(offset 不连续等) */
    uint8_t phase;
    uint16_t reserved;
    uint32_t written;        /* 累计已写字节 */
} sysmgr_ota_data_ack_t;

/* OTA_COMMIT 请求 (A→R) */
typedef struct __attribute__((packed)) {
    uint32_t crc32;          /* 复核用 (与 BEGIN 相同) */
    uint8_t  switch_flag;    /* 1=本镜像提交后执行 NOR GPT 切槽 (最后一个镜像) */
    uint8_t  reserved[3];
} sysmgr_ota_commit_t;

/* OTA_COMMIT 响应 (R→A) */
typedef struct __attribute__((packed)) {
    uint8_t ack;             /* 1=成功(已切槽), 0=失败(活动槽不变) */
    uint8_t phase;
    uint8_t active_slot;     /* 提交后的活动槽 (成功=新槽) */
    uint8_t reserved;
} sysmgr_ota_commit_ack_t;

/* OTA_STATUS 响应 (R→A) */
typedef struct __attribute__((packed)) {
    uint8_t  phase;          /* sysmgr_ota_phase_t */
    uint8_t  error;          /* 0=无 */
    uint16_t reserved;
    uint32_t processed;      /* 已处理字节 (擦除/写入进度) */
    uint32_t total;          /* 总字节 (BEGIN 的 image_size) */
} sysmgr_ota_status_t;

/* OTA chunk 数据最大长度 (iccmsg 单帧净荷约束) */
#define SYSMGR_OTA_CHUNK_MAX   400   /* 帧数据 ≤ ~475B, 留协议头余量 */

/* ---- 默认配置值 ---- */
#define SYSMGR_REMOTE_EPT_DEFAULT  500   /* R 核 sysmgr endpoint (iccmsg 首个分配) */
#define SYSMGR_HEARTBEAT_INTERVAL  5     /* 秒 */
#define SYSMGR_HEARTBEAT_TIMEOUT   30    /* 秒 */
#define SYSMGR_WDT_TIMEOUT         10    /* 秒 */
#define SYSMGR_GRACE_PERIOD        60    /* 秒 */
#define SYSMGR_MONITOR_TASK_PERIOD 1     /* 秒 (monitor_task 循环间隔) */

#ifdef __cplusplus
}
#endif

#endif /* SYSMGR_PROTOCOL_H */
