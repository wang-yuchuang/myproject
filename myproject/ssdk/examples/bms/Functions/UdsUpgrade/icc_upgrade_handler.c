#include "icc_upgrade_handler.h"
#include "iccmsg.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <cmsis_os2.h>

extern iccmsg_t *iccmsg_update;

extern void *ExtMalloc(size_t xWantedSize);
extern void ExtFree(void *pv);

/* 升级过程全局状态(由 ICC 处理器维护，供 manager 等模块读取)： */
uint8_t  *g_upgrade_file_buf = NULL;       // 固件镜像缓冲：分块接收后在此拼接成完整镜像
uint32_t  g_upgrade_file_size = 0;         // 固件镜像总字节数
uint16_t  g_upgrade_chunk_size = 0;        // 每个数据块大小(字节)
uint16_t  g_upgrade_chunk_count = 0;       // 数据块总数
uint32_t  g_upgrade_file_crc32 = 0;        // 固件镜像 CRC32(来自主控，用于完整性校验)
uint8_t   g_upgrade_dev_type = 0;          // 目标设备类型(1=BSU, 5=IMIU4, 6=IMEU3)
uint8_t   g_upgrade_dev_num = 0;           // 目标设备数量
uint8_t   g_upgrade_dev_addr[16] = {0};    // 目标设备地址列表(最多 16 台)
upgrade_state_t g_upgrade_state = STATE_IDLE;  // 升级整体状态机

static uint8_t  g_chunk_seq_next = 0;      // 期望接收的下一个数据块序号
static uint8_t  g_chunk_received = 0;      // 已成功接收的数据块计数
static uint8_t  g_chunk_seq_gap_count = 0; // 连续乱序(序号跳变)计数，用于防抖，达到阈值即中止
static uint8_t  g_chunk_seq_dup_count = 0; // 连续重复数据块计数
static uint8_t  g_chunk_bad_len_count = 0; // 连续长度异常计数
static uint32_t g_chunk_req_tick = 0;      // 上次发送数据块请求的时刻(用于超时重传)
static uint8_t  g_chunk_timeout_count = 0; // 数据块应答超时计数

#define CHUNK_TIMEOUT_MS        2000   // 单个数据块应答超时时间(ms)
#define CHUNK_TIMEOUT_MAX       3      // 连续超时最大次数，超过则中止升级
#define MAX_UPGRADE_FILE_SIZE   (512u * 1024u)   // 固件镜像大小上限(512KB)，防止异常值申请超大内存

#define ICCMSG_SYNC_FLAG    0          // ICC 消息同步标志(0=同步发送)

/* 设备类型编码转可读名称，用于日志打印 */
static const char *dev_type_name(uint8_t t)
{
    switch (t) {
    case 1:  return "BSU";
    case 5:  return "IMIU4";
    case 6:  return "IMEU3";
    default: return "UNKNOWN";
    }
}

/* 小端序读取 4 字节无符号整数(ICC 负载统一小端格式) */
static uint32_t read_u32_le(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

/* 小端序读取 2 字节无符号整数 */
static uint16_t read_u16_le(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

/* 小端序写入 4 字节无符号整数 */
static void write_u32_le(uint8_t *dst, uint32_t val)
{
    dst[0] = (uint8_t)(val & 0xFF);
    dst[1] = (uint8_t)((val >> 8) & 0xFF);
    dst[2] = (uint8_t)((val >> 16) & 0xFF);
    dst[3] = (uint8_t)((val >> 24) & 0xFF);
}

/* 小端序写入 2 字节无符号整数 */
static void write_u16_le(uint8_t *dst, uint16_t val)
{
    dst[0] = (uint8_t)(val & 0xFF);
    dst[1] = (uint8_t)((val >> 8) & 0xFF);
}

/*
 * 向主控上层发送一条 ICC 升级消息。
 * buf[0] 存放命令码，其后紧跟负载，通过 iccmsg 同步通道发出且无需对方 ACK。
 */
void icc_upgrade_send_msg(uint8_t cmd, const uint8_t *payload, uint32_t payload_len)
{
    uint8_t buf[64];

    if (iccmsg_update == NULL)
        return;   // ICC 通道未初始化，直接丢弃

    buf[0] = cmd;   // 首字节 = 命令码
    if (payload != NULL && payload_len > 0) {
        memcpy(buf + 1, payload, payload_len);   // 负载紧随命令码
    }
    iccmsg_send(iccmsg_update, ICCMSG_SYNC_FLAG, 0, buf, payload_len + 1, ICCMSG_ACK_NO_NEED);
}

/*
 * 回应主控的 FILE_START 指令：把处理结果状态、文件参数(总大小/块大小/块数/CRC)、
 * 设备类型与数量、以及设备地址列表按小端格式打包成 ACK 负载回传。
 */
void icc_upgrade_send_start_ack_ex(uint8_t status,
        uint32_t total_size, uint16_t chunk_size, uint16_t chunk_count,
        uint32_t crc32, uint8_t dev_type, uint8_t dev_num,
        const uint8_t *dev_addrs)
{
    uint8_t ack[32];
    uint8_t pos = 0;

    ack[pos++] = status;
    write_u32_le(ack + pos, total_size);  pos += 4;
    write_u16_le(ack + pos, chunk_size);  pos += 2;
    write_u16_le(ack + pos, chunk_count); pos += 2;
    write_u32_le(ack + pos, crc32);       pos += 4;
    ack[pos++] = dev_type;
    ack[pos++] = dev_num;

    for (uint8_t i = 0; dev_addrs != NULL && i < dev_num && i < 16; i++) {
        ack[pos++] = dev_addrs[i];   // 逐台设备地址拼入负载末尾
    }

    ssdk_printf(SSDK_INFO, "[ICC] FILE_START_ACK status=0x%02x type=%s(%d) devs=%d\r\n",
                status, dev_type_name(dev_type), dev_type, dev_num);
    icc_upgrade_send_msg(ICC_CMD_FILE_START_ACK, ack, pos);
}

/*
 * 向主控上报升级中止及原因(超时/CRC/序号错误/用户主动等)。
 */
void icc_upgrade_send_abort(uint8_t reason)
{
    uint8_t payload[1];
    payload[0] = reason;
    ssdk_printf(SSDK_WARNING, "[ICC] ABORT reason=%d\r\n", reason);
    icc_upgrade_send_msg(ICC_CMD_ABORT, payload, 1);
}

/*
 * 向主控请求第 seq 号数据块：携带请求状态(OK/GAP)与期望序号，
 * 同时记录请求时刻，供 icc_upgrade_process() 做超时重传判断。
 */
static void icc_upgrade_request_chunk(uint8_t status, uint8_t seq)
{
    uint8_t payload[2];
    payload[0] = status;
    payload[1] = seq;
    g_chunk_req_tick = (uint32_t)osKernelGetTickCount();   // 记录请求时刻，作为超时计时基准
    ssdk_printf(SSDK_DEBUG, "[ICC] CHUNK_REQ seq=%u status=%u\r\n", seq, status);
    icc_upgrade_send_msg(ICC_CMD_FILE_CHUNK_REQ, payload, 2);
}

/*
 * 清理升级过程的所有资源与状态：释放固件缓冲、清零文件参数/设备信息/分块统计，
 * 状态机回到 IDLE，供下一次升级复用。
 */
void icc_upgrade_cleanup(void)
{
    if (g_upgrade_file_buf != NULL) {
        ssdk_printf(SSDK_INFO, "[ICC] Freeing file buffer %u bytes\r\n",
                    (unsigned int)g_upgrade_file_size);
        ExtFree(g_upgrade_file_buf);   // 释放固件镜像缓冲
        g_upgrade_file_buf = NULL;
    }
    g_upgrade_file_size = 0;           // 以下清零所有文件与设备相关状态
    g_upgrade_chunk_size = 0;
    g_upgrade_chunk_count = 0;
    g_upgrade_file_crc32 = 0;
    g_upgrade_dev_type = 0;
    g_upgrade_dev_num = 0;
    memset(g_upgrade_dev_addr, 0, sizeof(g_upgrade_dev_addr));
    g_chunk_seq_next = 0;
    g_chunk_received = 0;
    g_chunk_seq_gap_count = 0;
    g_chunk_seq_dup_count = 0;
    g_chunk_bad_len_count = 0;
    g_chunk_req_tick = 0;
    g_chunk_timeout_count = 0;
    g_upgrade_state = STATE_IDLE;      // 回到空闲态，等待下一次升级
    ssdk_printf(SSDK_INFO, "[ICC] State -> IDLE, cleanup complete\r\n");
}

/* 计算标准 CRC32(表驱动、按半字节查表)，用于校验接收到的固件镜像完整性 */
static uint32_t icc_crc32_calc(const uint8_t *data, uint32_t len)
{
    static const uint32_t rtable[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < len; i++) {
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0x0F];
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0x0F];
    }
    return crc;
}

/*
 * 校验设备地址是否合法：按设备类型约束地址范围
 * (BSU: 0~15，IMIU4: 20/21，IMEU3: 31)。
 */
static int is_valid_dev_addr(uint8_t dev_type, uint8_t addr)
{
    switch (dev_type) {
    case 1:  return (addr <= 15);                 /* BSU: 0-15 */
    case 5:  return (addr == 20 || addr == 21);   /* IMIU4: 20, 21 */
    case 6:  return (addr == 30);                 /* IMEU3: 30 */
    default: return 0;
    }
}

/*
 * 处理主控的 FILE_START 指令：解析文件参数、设备类型/数量/地址列表，
 * 逐项校验合法性，申请文件缓冲，成功后回 ACK 并请求第 0 块数据开始接收。
 */
static void icc_handle_file_start(const uint8_t *data, uint32_t len)
{
    uint32_t total_size, file_crc32;
    uint16_t chunk_size, chunk_count;
    uint8_t dev_type, dev_num;
    uint8_t dev_addrs[16];

    // 仅允许在 IDLE 态接收新升级任务，防止覆盖正在进行的升级
    if (g_upgrade_state != STATE_IDLE) {
        ssdk_printf(SSDK_WARNING, "[ICC] FILE_START ignored, state=%d (not IDLE)\r\n",
                    (int)g_upgrade_state);
        return;
    }

    // 最小长度校验：负载至少含固定头 15 字节
    if (len < 15) {
        ssdk_printf(SSDK_ERR, "[ICC] FILE_START too short: len=%lu min=15\r\n",
                    (unsigned long)len);
        return;
    }

    // 按小端解析固定头(跳过首字节命令码)：总大小/块大小/块数/CRC/设备类型/设备数量
    total_size  = read_u32_le(data + 1 + 0);
    chunk_size  = read_u16_le(data + 1 + 4);
    chunk_count = read_u16_le(data + 1 + 6);
    file_crc32  = read_u32_le(data + 1 + 8);
    dev_type    = data[1 + 12];
    dev_num     = data[1 + 13];

    if (dev_type != 1 && dev_type != 5 && dev_type != 6) {
        ssdk_printf(SSDK_ERR, "[ICC] FILE_START invalid dev_type=%d\r\n", dev_type);
        icc_upgrade_send_start_ack_ex(ICC_START_STATUS_ERR_TYPE,
                total_size, chunk_size, chunk_count, file_crc32,
                dev_type, 0, NULL);
        return;
    }

    if (dev_num == 0 || dev_num > 16) {
        ssdk_printf(SSDK_ERR, "[ICC] FILE_START invalid dev_num=%d\r\n", dev_num);
        icc_upgrade_send_start_ack_ex(ICC_START_STATUS_ERR_ADDR_NUM,
                total_size, chunk_size, chunk_count, file_crc32,
                dev_type, dev_num, NULL);
        return;
    }

    // 文件大小上限校验：超过 512KB 拒绝，防止异常/恶意值申请超大内存
    if (total_size > MAX_UPGRADE_FILE_SIZE) {
        ssdk_printf(SSDK_ERR, "[ICC] FILE_START total_size too large: %lu (max=%u)\r\n",
                    (unsigned long)total_size, MAX_UPGRADE_FILE_SIZE);
        icc_upgrade_send_start_ack_ex(ICC_START_STATUS_ERR_MEMORY,
                total_size, chunk_size, chunk_count, file_crc32,
                dev_type, dev_num, NULL);
        return;
    }

    // 长度校验：确保负载内确实包含 dev_num 个设备地址，防止越界读取
    if (len < (uint32_t)(15 + dev_num)) {
        ssdk_printf(SSDK_ERR, "[ICC] FILE_START dev_addr overflow: len=%lu need=%d\r\n",
                    (unsigned long)len, 15 + dev_num);
        return;
    }

    ssdk_printf(SSDK_INFO, "[ICC] FILE_START: type=%s(%d) devs=%d size=%lu chunks=%u x %u crc=0x%08lx\r\n",
                dev_type_name(dev_type), dev_type, dev_num,
                (unsigned long)total_size, chunk_count, chunk_size,
                (unsigned long)file_crc32);

    // 逐台校验设备地址合法性
    for (uint8_t i = 0; i < dev_num; i++) {
        dev_addrs[i] = data[1 + 14 + i];
        if (!is_valid_dev_addr(dev_type, dev_addrs[i])) {
            ssdk_printf(SSDK_ERR, "[ICC] FILE_START invalid addr[%d]=%d for type=%d\r\n",
                        i, dev_addrs[i], dev_type);
            icc_upgrade_send_start_ack_ex(ICC_START_STATUS_ERR_ADDR,
                    total_size, chunk_size, chunk_count, file_crc32,
                    dev_type, dev_num, dev_addrs);
            return;
        }
    }

    // 申请固件镜像缓冲，失败则回错误 ACK
    uint8_t *buf = (uint8_t *)ExtMalloc(total_size);
    if (buf == NULL) {
        ssdk_printf(SSDK_ERR, "[ICC] ExtMalloc(%lu) FAILED\r\n",
                    (unsigned long)total_size);
        icc_upgrade_send_start_ack_ex(ICC_START_STATUS_ERR_MEMORY,
                total_size, chunk_size, chunk_count, file_crc32,
                dev_type, dev_num, dev_addrs);
        return;
    }

    // 记录文件与设备参数到全局，供后续分块接收使用
    g_upgrade_file_buf = buf;
    g_upgrade_file_size = total_size;
    g_upgrade_chunk_size = chunk_size;
    g_upgrade_chunk_count = chunk_count;
    g_upgrade_file_crc32 = file_crc32;
    g_upgrade_dev_type = dev_type;
    g_upgrade_dev_num = dev_num;

    for (uint8_t i = 0; i < dev_num; i++) {
        g_upgrade_dev_addr[i] = dev_addrs[i];
        ssdk_printf(SSDK_DEBUG, "[ICC]   dev[%d] addr=%d\r\n", i, g_upgrade_dev_addr[i]);
    }

    g_upgrade_state = STATE_FILE_RECEIVING;   // 进入接收文件状态
    g_chunk_seq_next = 0;                     // 从第 0 块开始接收
    g_chunk_received = 0;

    icc_upgrade_send_start_ack_ex(ICC_START_STATUS_OK,
            g_upgrade_file_size, (uint16_t)g_upgrade_chunk_size,
            g_upgrade_chunk_count, g_upgrade_file_crc32,
            g_upgrade_dev_type, g_upgrade_dev_num,
            g_upgrade_dev_addr);

    if (chunk_count > 0) {
        icc_upgrade_request_chunk(ICC_CHUNK_REQ_STATUS_OK, 0);   // 请求第一块数据
    } else {
        ssdk_printf(SSDK_INFO, "[ICC] Zero chunks, direct to UPGRADING\r\n");
        g_upgrade_state = STATE_UPGRADING;   // 无数据块(空文件)直接进入升级态
    }
}

/*
 * 处理主控发来的数据块(应答)：校验序号连续性(重复/乱序)、数据长度，
 * 将正确数据拷贝进文件缓冲；收齐全部块后做 CRC 校验，通过则进入 UPGRADING 状态。
 */
static void icc_handle_chunk_ack(const uint8_t *data, uint32_t len)
{
    uint8_t seq;

    // 仅接收文件阶段处理数据块
    if (g_upgrade_state != STATE_FILE_RECEIVING)
        return;

    if (len < 2)
        return;

    seq = data[1];
    uint32_t chunk_data_len = len - 2;   // 负载 = 首字节命令码 + 序号 + 数据

    if (seq >= g_upgrade_chunk_count) {
        ssdk_printf(SSDK_WARNING, "[ICC] CHUNK_ACK seq=%u out of range (max=%u)\r\n",
                    seq, g_upgrade_chunk_count);
        return;
    }

    ssdk_printf(SSDK_DEBUG, "[ICC] CHUNK_ACK seq=%u/%u len=%lu\r\n",
                seq, g_upgrade_chunk_count - 1, (unsigned long)chunk_data_len);

    // 重复块：序号小于期望值，说明主控重发了已收过的块，累计后防抖处理
    if (seq < g_chunk_seq_next) {
        g_chunk_seq_dup_count++;
        ssdk_printf(SSDK_WARNING, "[ICC] DUP seq=%u count=%d\r\n",
                    seq, g_chunk_seq_dup_count);
        if (g_chunk_seq_dup_count >= 3) {
            ssdk_printf(SSDK_ERR, "[ICC] DUP limit reached, ABORT\r\n");
            icc_upgrade_send_abort(ICC_ABORT_REASON_SEQ);
            icc_upgrade_cleanup();
            return;
        }
        icc_upgrade_request_chunk(ICC_CHUNK_REQ_STATUS_OK, g_chunk_seq_next);   // 重新请求期望块
        return;
    }

    // 乱序块：序号大于期望值，说明中间有块丢失，累计防抖后以 GAP 状态重新请求
    if (seq > g_chunk_seq_next) {
        g_chunk_seq_gap_count++;
        ssdk_printf(SSDK_WARNING, "[ICC] GAP: expected=%u got=%u count=%d, re-request\r\n",
                    g_chunk_seq_next, seq, g_chunk_seq_gap_count);
        if (g_chunk_seq_gap_count >= 3) {
            ssdk_printf(SSDK_ERR, "[ICC] GAP limit reached, ABORT\r\n");
            icc_upgrade_send_abort(ICC_ABORT_REASON_SEQ);
            icc_upgrade_cleanup();
            return;
        }
        icc_upgrade_request_chunk(ICC_CHUNK_REQ_STATUS_GAP, g_chunk_seq_next);
        return;
    }

    {
        uint32_t expected_len = g_upgrade_chunk_size;   // 默认按完整块大小
        uint32_t offset = (uint32_t)seq * g_upgrade_chunk_size;

        // 最后一块可能不满一个块大小(文件大小非块大小整数倍)
        if (seq == g_upgrade_chunk_count - 1) {
            uint32_t remainder = g_upgrade_file_size % g_upgrade_chunk_size;
            if (remainder != 0)
                expected_len = remainder;
        }

        // 长度异常：实际长度与期望不符，或写入会越界，累计防抖
        if (chunk_data_len != expected_len ||
            offset + chunk_data_len > g_upgrade_file_size) {
            g_chunk_bad_len_count++;
            ssdk_printf(SSDK_ERR, "[ICC] CHUNK_ACK seq=%u bad len=%lu expected=%lu count=%d\r\n",
                        seq, (unsigned long)chunk_data_len, (unsigned long)expected_len,
                        g_chunk_bad_len_count);
            if (g_chunk_bad_len_count >= 3) {
                ssdk_printf(SSDK_ERR, "[ICC] bad len limit reached, ABORT\r\n");
                icc_upgrade_send_abort(ICC_ABORT_REASON_SEQ);
                icc_upgrade_cleanup();
                return;
            }
            icc_upgrade_request_chunk(ICC_CHUNK_REQ_STATUS_OK, g_chunk_seq_next);
            return;
        }

        memcpy(g_upgrade_file_buf + offset, data + 2, chunk_data_len);   // 数据写入对应偏移
    }

    g_chunk_received++;
    g_chunk_timeout_count = 0;   // 收到数据即清零超时计数

    ssdk_printf(SSDK_DEBUG, "[ICC]   progress: %u/%u chunks\r\n",
                g_chunk_received, g_upgrade_chunk_count);

    // 全部块收齐：计算 CRC 并与主控下发值比对，决定进入升级或中止
    if (g_chunk_received >= g_upgrade_chunk_count) {
        uint32_t calc_crc = icc_crc32_calc(g_upgrade_file_buf, g_upgrade_file_size);
        ssdk_printf(SSDK_INFO, "[ICC] All %u chunks done. CRC calc=0x%08lx expect=0x%08lx\r\n",
                    g_upgrade_chunk_count, (unsigned long)calc_crc,
                    (unsigned long)g_upgrade_file_crc32);
        if (calc_crc == g_upgrade_file_crc32) {
            ssdk_printf(SSDK_INFO, "[ICC] CRC OK -> State UPGRADING\r\n");
            g_upgrade_state = STATE_UPGRADING;   // CRC 通过，交给 manager 启动 CAN 升级
        } else {
            ssdk_printf(SSDK_ERR, "[ICC] CRC MISMATCH! ABORT reason=5\r\n");
            icc_upgrade_send_abort(ICC_ABORT_REASON_CRC);
            icc_upgrade_cleanup();
        }
        return;
    }

    g_chunk_seq_next = seq + 1;   // 期望序号推进到下一块
    icc_upgrade_request_chunk(ICC_CHUNK_REQ_STATUS_OK, g_chunk_seq_next);
}

/*
 * 处理主控下发的 ABORT 指令：仅在收文件状态下执行清理，
 * 其余状态忽略(升级已进入 CAN 流程，由 manager 负责收尾)。
 */
static void icc_handle_abort(const uint8_t *data, uint32_t len)
{
    if (len < 2)
        return;

    uint8_t reason = data[1];

    if (g_upgrade_state == STATE_FILE_RECEIVING) {
        ssdk_printf(SSDK_WARNING, "[ICC] ABORT received reason=%d, cleaning up\r\n", reason);
        icc_upgrade_cleanup();
    } else {
        ssdk_printf(SSDK_INFO, "[ICC] ABORT ignored (state=%d, not FILE_RECEIVING)\r\n",
                    (int)g_upgrade_state);
    }
}

/*
 * ICC 升级消息分发入口：解析首字节命令码，分派给对应的处理函数。
 */
void icc_upgrade_handle_msg(const uint8_t *data, uint32_t len)
{
    if (data == NULL || len < 1)
        return;

    uint8_t cmd = data[0];

    switch (cmd) {
    case ICC_CMD_FILE_START:      // 开始升级指令(含文件/设备参数)
        ssdk_printf(SSDK_DEBUG, "[ICC] RX FILE_START len=%lu\r\n", (unsigned long)len);
        icc_handle_file_start(data, len);
        break;
    case ICC_CMD_FILE_CHUNK_ACK:  // 数据块应答
        icc_handle_chunk_ack(data, len);
        break;
    case ICC_CMD_ABORT:           // 中止指令
        ssdk_printf(SSDK_DEBUG, "[ICC] RX ABORT len=%lu\r\n", (unsigned long)len);
        icc_handle_abort(data, len);
        break;
    default:                      // 未知命令，忽略
        ssdk_printf(SSDK_DEBUG, "[ICC] RX unknown cmd=0x%02x len=%lu\r\n",
                    cmd, (unsigned long)len);
        break;
    }
}

/*
 * ICC 周期处理(由 RTOS 任务循环调用)：在收文件状态下检测数据块应答超时，
 * 超过 CHUNK_TIMEOUT_MS 未收到则累计，超限中止，否则重传请求。
 */
void icc_upgrade_process(void)
{
    uint32_t now;

    // 仅收文件阶段且已发出过请求时才做超时检测
    if (g_upgrade_state != STATE_FILE_RECEIVING)
        return;

    if (g_chunk_req_tick == 0)
        return;

    now = (uint32_t)osKernelGetTickCount();
    if (now - g_chunk_req_tick < CHUNK_TIMEOUT_MS)
        return;   // 未到超时时间，继续等待

    g_chunk_timeout_count++;
    ssdk_printf(SSDK_WARNING, "[ICC] CHUNK_ACK timeout seq=%u count=%d/%d\r\n",
                g_chunk_seq_next, g_chunk_timeout_count, CHUNK_TIMEOUT_MAX);

    if (g_chunk_timeout_count > CHUNK_TIMEOUT_MAX) {
        ssdk_printf(SSDK_ERR, "[ICC] CHUNK timeout limit, ABORT\r\n");
        icc_upgrade_send_abort(ICC_ABORT_REASON_TIMEOUT);
        icc_upgrade_cleanup();
        return;
    }

    icc_upgrade_request_chunk(ICC_CHUNK_REQ_STATUS_OK, g_chunk_seq_next);   // 超时重传同一块
}
