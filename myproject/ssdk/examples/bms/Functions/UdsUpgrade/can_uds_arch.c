/* *******************
// can uds需要重载实现的接口
 **********************/
#include "can_uds_arch.h"
#include "sid_internal_header.h"
#include <cmsis_os2.h>
#include <string.h>
#include "DEVCAN.h"
#include "icc_upgrade_handler.h"
#include <debug.h>

/* 升级所用的 CAN 通道（tester 端通过它收发诊断报文），默认使用 eCAN1 */
e_CANNUM g_upgrade_can_port = eCAN1;
/* CAN 发送连续失败计数：每发送失败一次自增 1，供状态机判断链路是否异常；
   语义为“连续失败计数”，发送成功时不会自动清零，由上层按需复位 */
uint32_t g_uds_can_tx_fail_count = 0;

/* 安全访问算法内部变换函数（仅文件内部使用，实现见文件末尾） */
static unsigned long transform1(unsigned long seed);
static unsigned long transform2(unsigned long seed);

/* 通过经典 CAN 发送一帧 8 字节数据（UDS 单帧发送的底层承载）
 * @param id         报文的 CAN ID
 * @param type       帧类型：0=标准帧(STDID)，非0=扩展帧(EXTID)
 * @param data0~7    待发送的 8 字节负载
 * @return 返回值 >0 表示发送成功（此处返回数据长度 8）；返回 0 表示发送失败，
 *         失败时内部会对 g_uds_can_tx_fail_count 自增并打印错误日志 */
int uds_send_data8(unsigned long id, unsigned char type,
                    unsigned char data0, unsigned char data1, unsigned char data2, unsigned char data3,
                    unsigned char data4, unsigned char data5, unsigned char data6, unsigned char data7)
{
    can_frame_t frame;

    memset(&frame, 0, sizeof(can_frame_t));
    frame.id = id;
    frame.ide = (type != 0) ? CAN_EXTID : CAN_STDID;//type非0时按扩展帧发送，否则按标准帧
    frame.rtr = CAN_DTR;
    frame.fd = 0;      //fd=0 表示经典 CAN 帧（非 CAN-FD）
    frame.len = 8;
    frame.data[0] = data0;
    frame.data[1] = data1;
    frame.data[2] = data2;
    frame.data[3] = data3;
    frame.data[4] = data4;
    frame.data[5] = data5;
    frame.data[6] = data6;
    frame.data[7] = data7;

    if (BSPCANSendQin(g_upgrade_can_port, &frame) != 0)//入队成功，返回 8 表示发送成功
        return 8;
    g_uds_can_tx_fail_count++;//发送失败：连续失败计数自增
    ssdk_printf(SSDK_ERR, "[ARCH] uds_send_data8 FAIL: id=0x%08lx port=%d\r\n",
                id, (int)g_upgrade_can_port);
    return 0;
}

/* 通过 CAN-FD 发送一帧 64 字节数据（大数据块传输的底层承载）
 * 前 8 字节由 data0~data7 提供，后续 56 字节由 next_datas 指向的缓冲区拷贝
 * @return 返回值 >0 表示发送成功（此处返回 64）；返回 0 表示发送失败并自增失败计数 */
int uds_send_data64(unsigned long id, unsigned char type,
                    unsigned char data0, unsigned char data1, unsigned char data2, unsigned char data3,
                    unsigned char data4, unsigned char data5, unsigned char data6, unsigned char data7,
                    unsigned char *next_datas)
{
    can_frame_t frame;

    memset(&frame, 0, sizeof(can_frame_t));
    frame.id = id;
    frame.ide = (type != 0) ? CAN_EXTID : CAN_STDID;//type非0时按扩展帧发送，否则按标准帧
    frame.rtr = CAN_DTR;
    frame.fd = 1;      //fd=1 表示 CAN-FD 帧，可携带更长负载
    frame.len = 64;
    frame.data[0] = data0;
    frame.data[1] = data1;
    frame.data[2] = data2;
    frame.data[3] = data3;
    frame.data[4] = data4;
    frame.data[5] = data5;
    frame.data[6] = data6;
    frame.data[7] = data7;

    if (next_datas != NULL) {
        memcpy(&frame.data[8], next_datas, 56);//拷贝后续 56 字节到 CAN-FD 帧剩余数据区
    }

    if (BSPCANSendQin(g_upgrade_can_port, &frame) != 0)//入队成功，返回 64 表示发送成功
        return 64;
    g_uds_can_tx_fail_count++;//发送失败：连续失败计数自增
    ssdk_printf(SSDK_ERR, "[ARCH] uds_send_data64 FAIL: id=0x%08lx port=%d\r\n",
                id, (int)g_upgrade_can_port);
    return 0;
}

/* 获取系统时间戳（tick 计数），用作协议超时判断或安全访问 seed 的随机来源 */
unsigned long get_timer_tick(void)
{
    return (unsigned long)osKernelGetTickCount();
}

/* 向指定逻辑地址写入升级数据（平台适配层钩子）
 * 当前示例为占位实现：直接返回写入长度，真实产品需在此对接底层 flash 编程接口 */
int program_data_by_logic_addr(unsigned long addr, unsigned char *msg_buff, unsigned short len)
{
    return len;
}

/* 获取安全访问的 seed：由时间戳与固定常量异或产生，
 * 使每次会话返回的 seed 随 tick 变化而不同 */
unsigned long uds_get_seed(void)
{
    return get_timer_tick() ^ 0xAAff5A75;
}

/* 安全访问算法：由 seed 计算 key
 * 分别经过 transform1、transform2 两次变换后做异或得到最终 key */
unsigned long uds_seed_to_key(unsigned long input_seed)
{
    unsigned long key1 = transform1(input_seed);
    unsigned long key2 = transform2(input_seed);
    return key1 ^ key2;
}

/* 擦除应用区数据（平台适配层钩子），示例占位实现返回 0（成功） */
int uds_erase_app_data(void)
{
    return 0;
}

/* 升级收尾处理（平台适配层钩子），示例占位实现返回 0（成功） */
int uds_program_finish(void)
{
    return 0;
}

/* 查询当前是否使用 CAN-FD 通道：固定返回 1，表示升级走 CAN-FD 大包传输 */
int uds_is_canfd(void)
{
    return 1;
}

/* 从升级文件缓冲区读取指定地址/长度的数据，通过 data_out 返回指针
 * @return 成功返回实际可取长度；缓冲区为空、指针为空或越界时返回 0 */
int uds_get_upgrade_data(unsigned long addr, unsigned long len, unsigned char **data_out)
{
    if (g_upgrade_file_buf == NULL || data_out == NULL)
        return 0;

    if (addr + len > g_upgrade_file_size)//防止越界读取
        return 0;

    *data_out = g_upgrade_file_buf + addr;
    return (int)len;
}

/* 返回升级文件（镜像）总长度，缓冲区未就绪时返回 0 */
uint32_t uds_get_upgrade_bin_len(void)
{
    if (g_upgrade_file_buf == NULL)
        return 0;
    return g_upgrade_file_size;
}

/* CRC 计算（查表法，表为 16 项，按半字节逐步更新 crc）
 * 返回累加后的 CRC 值，支持分块连续计算 */
unsigned long uds_crc(unsigned long crc, const void *buffer, int size)
{
    static const unsigned long rtable[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };

    const uint8_t *data = buffer;

    for (int i = 0; i < size; i++) {
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
    }

    return crc;
}

/* 整包 CRC 计算：与 uds_crc 算法一致，但结果直接写入 wholepackcrc 指向的变量，
 * 用于对完整数据块（如整个传输块）做校验 */
void uds_wholepackcrc(unsigned long *wholepackcrc, const void *buffer, int size)
{
    static const unsigned long rtable[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };

    const uint8_t *data = buffer;

    for (int i = 0; i < size; i++) {
        *wholepackcrc = (*wholepackcrc >> 4) ^ rtable[(*wholepackcrc ^ (data[i] >> 0)) & 0xf];
        *wholepackcrc = (*wholepackcrc >> 4) ^ rtable[(*wholepackcrc ^ (data[i] >> 4)) & 0xf];
    }
}

/* 初始化 CRC 计算上下文：将整包 CRC 初值置为 0xFFFFFFFF，返回 0 */
unsigned long uds_crc_init(unsigned long *wholepackcrc)
{
    *wholepackcrc = 0xFFFFFFFFuL;
    return 0;
}

/* 喂狗钩子：升级过程中周期性调用以防看门狗复位，示例为空实现 */
void uds_feed_wdt(void)
{
}

/* 升级完成后复位目标设备（钩子），示例为空实现，真实产品在此触发系统复位 */
void uds_reset(void)
{
}

/* 安全访问变换1：对 seed 的每个字节做循环左移 1 位后异或 0x1B */
static unsigned long transform1(unsigned long seed)
{
    unsigned char *b = (unsigned char *)&seed;
    for (int i = 0; i < 4; i++) {
        b[i] = (b[i] << 1) | (b[i] >> 7);//循环左移 1 位（最高位回绕到最低位）
        b[i] ^= 0x1B;
    }
    return seed;
}

/* 安全访问变换2：先按字节做端序翻转，再异或固定常量 0xA5A555AA */
static unsigned long transform2(unsigned long seed)
{
    unsigned long temp = (seed >> 24) |
                   ((seed >> 8) & 0xFF00) |
                   ((seed << 8) & 0xFF0000) |
                   (seed << 24);//整体字节序反转
    return temp ^ 0xA5A555AA;
}
