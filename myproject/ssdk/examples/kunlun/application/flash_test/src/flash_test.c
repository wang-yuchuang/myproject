/************************************************************************
 * Copyright(c) 2026  Semidrive
 * All rights reserved.
 *
 * flash_test.c — OSPI2 NOR XIP 擦写最小化验证 CLI (kunlun / OSPI2 NOR)
 *
 * Purpose:
 *   在 XIP 模式（R5 从 OSPI2 NOR 取指执行）下，验证从 flash 取指执行
 *   擦写操作是否可安全运行。最小化实验：备份 -> 擦除 -> 写回一个绝对
 *   安全的测试 sector（NOR 尾部空闲区 0x3C9000，220KB），观察是否崩溃。
 *
 *   一次性排除3个中高风险点（Oracle 审核结论）：
 *     Q3  : 中断/异常在 flash，擦写时是否崩溃
 *     自包含: 擦写闭包是否会漏调 flash 里的函数（ssdk_printf 等）
 *     cache: 写回后校验数据是否被 cache 污染
 *
 * 安全测试区（bmu05 OSPI2 NOR 4KB sector，必须避开取指区/GPT/dil/ssystem）:
 *   NOR 尾部空闲区 0x3C9000~0x400000，从未使用、无任何 boot 路径引用。
 *
 * Usage (from R-core FreeRTOS CLI):
 *   flash_test            -> 备份->擦->写回->校验 安全区 sector (默认 4KB)
 *   flash_test erase      -> 只做擦除验证（擦除走 STIG，不改 CONFIG，用于分层定位）
 *   flash_test noraw      -> 不搬 RAM，直接用当前 XIP 地址执行（验证"是否需要搬RAM"）
 ************************************************************************/
#include <CLI.h>
#include <debug.h>

/* 用直接 printf（与 xip main.c Hello 一致），ssdk_printf 需 CONFIG_DEBUG 定义才输出 */
#define DBG(format, args...) printf("[FLASH_TEST] " format, ##args)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <param.h>
#include <arch/cache.h>
#include <arch/irq.h>     /* arch_irq_save/arch_irq_restore */
#include <reg.h>     /* readl/writel */
#include <reg.h>     /* readl/writel */
#include <driver/spi_nor.h>
#include <lib/disk/disk.h>
#include <lib/disk/disk_norflash.h>
#include <lib/cksum.h>   /* crc32 (标准 CRC-32/IEEE) */

#include "flash_test.h"

/************************************************************************
 * OSPI2 寄存器宏（复制自 private 头 cdns_ospi.h，避免 include private 头）
 ************************************************************************/
#define FT_OSPI_CMDCTRL              (0x90u)
#define FT_OSPI_CMDCTRL_EXECUTE      BIT(0)
#define FT_OSPI_CMDCTRL_INPROGRESS   BIT(1)
#define FT_OSPI_CMDCTRL_ADDR_EN      BIT(19)
#define FT_OSPI_CMDCTRL_OPCODE_LSB   (24u)
#define FT_OSPI_CMDCTRL_RD_EN        BIT(23)
#define FT_OSPI_CMDCTRL_RD_BYTES_LSB (20u)
#define FT_OSPI_CMDADDRESS            (0x94u)
#define FT_OSPI_CMDWRITEDATALOWER     (0xA8u)
#define FT_OSPI_CMDREADDATALOWER     (0xA0u)

#define FT_SPINOR_OP_RDSR            (0x05u)  /* Read Status Register */
#define FT_SPINOR_WIP_MASK           (0x1u)    /* Status Reg bit0: Write In Progress */

#define FT_SNOR_OPCODE_PROTO_LSB     (24u)   /* spi_nor.h:32 */

/* cospi_pdata 基址访问（从 flash.host->priv_data 取）*/
struct cospi_pdata_ft {
    uint8_t id;
    addr_t apb_base;
};

/************************************************************************
 * 最小验证闭包入口 — 直接调用 cospi 底层（导出函数，不触发 cospi_lock）
 *
 * 注意: cospi_lld_stig_erase 定义在 cdns_ospi.c，声明在 private 头
 *       driver/kunlun/spi_nor/cdns_ospi.h。此处用 extern 声明避免 include
 *       private 头文件（保持 example 与驱动低耦合）。
 ************************************************************************/
struct spi_nor_cmd;     /* forward decl */
int cospi_lld_stig_erase(struct spi_nor *nor, flash_addr_t offset);
int cospi_lld_command_write(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                            flash_addr_t addr, const uint8_t *buf, flash_size_t len);
int cospi_write(struct spi_nor *nor, flash_addr_t addr,
                const uint8_t *buf, flash_size_t size);



/* bmu05 ospi_cfg.c 全局 spi_nor 实例（cospi_host_init + spi_nor_init 初始化） */
extern struct spi_nor flash;

/* RAM 探针（.data/.bss，放 RAM）：flash_test_erase 记录 RDSR 读到的 WIP 值和 ret，
 * 供 flash_test_cmd（flash XIP）返回后打印。闭包内不能 printf（格式串在 flash）。*/
volatile uint32_t g_probe_wip = 0;      /* RDSR 读到的 flash 状态 */
volatile int      g_probe_ret = 0;      /* ft_wait_flash_ready 返回值 */
volatile uint32_t g_probe_loops = 0;    /* ft_wait_flash_ready 轮询次数 */

#ifndef NORFLASH_TEST_OFFSET
#define NORFLASH_TEST_OFFSET   (0x3C9000)   /* NOR 尾部空闲区, 4KB sector */
#endif
#ifndef NORFLASH_TEST_SIZE
#define NORFLASH_TEST_SIZE     (4096)      /* 1 个 4KB sector */
#endif



/************************************************************************
 * 读测试区（XIP direct）：从 OSPI2_NOR 窗口直接 memcpy 读，绕过 cospi_read
 * (XIP direct 读已确认安全，与 t_loader get_partition_addr->read_ptr 同构)
 ************************************************************************/
static int flash_test_read(uint32_t addr, uint8_t *buf, uint32_t size)
{
    /* 读取走 XIP memory-mapped (OSPI2_BASE + addr)，纯 AHB 读事务 */
    arch_clean_invalidate_cache_range((addr_t)addr, ROUNDUP(size, 32));
    memcpy(buf, (void *)(uintptr_t)addr, size);
    return 0;
}

/************************************************************************
 * RAM 擦除闭包（__RAM_TEXT：链接到 .ram.text 段，VMA=RAM_BASE 0x140000，
 * 由 earlycopy 拷贝到 OCRAM；运行期从 RAM 执行擦除，避开 Q3 取指卡死）
 *
 * 直接发 STIG sector erase（WREN 0x06 + erase opcode），不走 cospi_lld_stig_erase
 * （它在 flash，调用链 static 函数回拉 flash 会卡死）。完全隔离：本闭包及其调用
 * 的寄存器操作都在 .ram.text，从 RAM 执行，擦除完成后跳回 XIP。
 ************************************************************************/
#define FT_OSPI_CONFIG_IDLE_MASK   BIT(31)   /* cdns_ospi.h:94 CONFIG_IDLE = BIT(31) */

static __RAM_TEXT int ft_wait_idle(addr_t apb_base)
{
    volatile uint32_t idle_cnt = 0, poll = 0;
    /* 轮询 CONFIG 寄存器 IDLE 位 (bit31) —— 对照 cospi_wait_idle */
    while (1) {
        if (readl(apb_base + 0x0u) & FT_OSPI_CONFIG_IDLE_MASK) {
            idle_cnt++;
            if (idle_cnt >= 4)
                return 0;
        } else {
            idle_cnt = 0;
        }
        if (++poll > 1000000u)
            return -1;
    }
}

/* 前向声明：ft_wait_flash_ready 定义在后，ft_stig_write_pp 先调用 */
static __RAM_TEXT int ft_wait_flash_ready(addr_t apb_base);

static __RAM_TEXT int ft_stig_cmd(addr_t apb_base, uint32_t opcode,
                                  uint32_t addr, uint8_t addr_bytes)
{
    uint32_t reg;
    /* 发命令前先等控制器空闲（对照 cospi_lld_command_write:687）*/
    if (ft_wait_idle(apb_base)) { return -1; }
    reg = (opcode << FT_OSPI_CMDCTRL_OPCODE_LSB);
    if (addr_bytes) {
        reg |= FT_OSPI_CMDCTRL_ADDR_EN;
        reg |= ((addr_bytes - 1u) & 0x3u) << 16;  /* ADD_BYTES_LSB=16 */
        writel(addr, apb_base + FT_OSPI_CMDADDRESS);
    }
    writel(reg, apb_base + FT_OSPI_CMDCTRL);        /* 写 CMDCTRL 不启动 */
    writel(reg | FT_OSPI_CMDCTRL_EXECUTE, apb_base + FT_OSPI_CMDCTRL);   /* 启动 */
    /* 等 INPROGRESS 位清除 (bit1) */
    {
        uint32_t cnt = 0;
        while (readl(apb_base + FT_OSPI_CMDCTRL) & FT_OSPI_CMDCTRL_INPROGRESS) {
            if (++cnt > 100000u)
                return -1;
        }
    }
    return ft_wait_idle(apb_base);
}

/* STIG 写 PP (page program 0x02)，≤4B/次。每次 PP 前必需 WREN（WEL 在每次 PP 后清）。
 * 不跨 256B page 边界由调用方保证。 */
static __RAM_TEXT int ft_stig_write_pp(addr_t apb_base, uint32_t addr,
                                       const uint8_t *buf, uint32_t size)
{
    uint32_t reg;
    /* ① WREN (0x06) —— 每次 PP 前必需，flash_test_write 块前的 WREN 对块内多次 PP 无效 */
    if (ft_stig_cmd(apb_base, 0x6u, 0, 0)) { return -1; }
    if (ft_wait_idle(apb_base)) { return -1; }
    /* ② PP 0x02, 3字节地址, 写≤4B 数据 */
    reg = (0x02u << FT_OSPI_CMDCTRL_OPCODE_LSB);   /* PP opcode */
    reg |= FT_OSPI_CMDCTRL_ADDR_EN;
    reg |= ((3u - 1u) & 0x3u) << 16;               /* ADDR 3 bytes */
    reg |= (1u << 15);                             /* WR_EN_LSB=15 */
    reg |= ((size - 1u) & 0x7u) << 12;             /* WR_BYTES_LSB=12 */
    writel(addr, apb_base + FT_OSPI_CMDADDRESS);
    writel(*(uint32_t *)buf, apb_base + FT_OSPI_CMDWRITEDATALOWER);
    writel(reg, apb_base + FT_OSPI_CMDCTRL);
    writel(reg | FT_OSPI_CMDCTRL_EXECUTE, apb_base + FT_OSPI_CMDCTRL);
    {
        uint32_t cnt = 0;
        while (readl(apb_base + FT_OSPI_CMDCTRL) & FT_OSPI_CMDCTRL_INPROGRESS) {
            if (++cnt > 100000u)
                return -1;
        }
    }
    return ft_wait_flash_ready(apb_base);   /* 等 WIP 清零 */
}

/* 读 flash 状态寄存器（RDSR 0x05），返回 Status Reg（WIP=bit0）*/
static __RAM_TEXT uint8_t ft_read_status(addr_t apb_base)
{
    uint32_t reg;
    if (ft_wait_idle(apb_base)) { return 0xFFu; }
    reg = (FT_SPINOR_OP_RDSR << FT_OSPI_CMDCTRL_OPCODE_LSB);
    reg |= FT_OSPI_CMDCTRL_RD_EN;
    reg |= ((2u - 1u) & 0x7u) << FT_OSPI_CMDCTRL_RD_BYTES_LSB;  /* 读2字节 */
    writel(reg, apb_base + FT_OSPI_CMDCTRL);
    writel(reg | FT_OSPI_CMDCTRL_EXECUTE, apb_base + FT_OSPI_CMDCTRL);
    {
        uint32_t cnt = 0;
        while (readl(apb_base + FT_OSPI_CMDCTRL) & FT_OSPI_CMDCTRL_INPROGRESS) {
            if (++cnt > 100000u)
                return 0xFFu;
        }
    }
    ft_wait_idle(apb_base);
    return (uint8_t)(readl(apb_base + FT_OSPI_CMDREADDATALOWER) & 0xFFu);
}

/* 等 flash 擦/写真正完成（WIP=bit0 清零）。控制器 IDLE != flash 就绪。*/
static __RAM_TEXT int ft_wait_flash_ready(addr_t apb_base)
{
    uint32_t cnt = 0;
    while (1) {
        uint8_t st = ft_read_status(apb_base);
        g_probe_wip = st;
        if (!(st & FT_SPINOR_WIP_MASK))
            return 0;
        if (++cnt > 10000000u) {
            g_probe_loops = cnt;
            return -1;
        }
    }
}

__RAM_TEXT int flash_test_erase(uint32_t addr)
{
    struct cospi_pdata_ft *cospi = (struct cospi_pdata_ft *)flash.host->priv_data;
    addr_t apb = cospi->apb_base;
    uint32_t erase_opcode = (uint32_t)(flash.info.erase_proto >> FT_SNOR_OPCODE_PROTO_LSB) & 0xFFu;
    irq_state_t old_irq;
    int ret;

    /* 擦除期间禁用中断 + 完全禁止访问 NOR AHB（含 printf 格式串在 flash）
     * 因此本闭包内不用 DBG/printf（其格式串在 flash .rodata），只写 RAM 标志 */
    old_irq = arch_irq_save();
    /* WREN (0x06, 无地址) — 纯 APB 寄存器操作 */
    ret = ft_stig_cmd(apb, 0x6u, 0, 0);
    if (ret) { arch_irq_restore(old_irq); return ret; }
    /* sector erase (地址) — 纯 APB 寄存器操作 */
    ret = ft_stig_cmd(apb, erase_opcode, addr, 3u);
    if (ret) { arch_irq_restore(old_irq); return ret; }
    /* 等 flash 擦除真正完成（WIP 清零），否则返回后 NOR 仍 busy，XIP 读返回垃圾 */
    ret = ft_wait_flash_ready(apb);
    g_probe_ret = ret;
    arch_irq_restore(old_irq);
    return ret;
}

/************************************************************************
 * STIG 分次写回（RAM 闭包）：Oracle 确认 indirect 写在 XIP 下不可行
 * (streaming-vs-XIP 仲裁冲突)。改用 STIG PP 分次写（≤4B/次，从 RAM 执行），
 * 与已验证的擦除 STIG 同构。低频元数据可接受。
 * 闭包内无 printf（格式串 flash .rodata），用探针记录。
 ************************************************************************/
__RAM_TEXT int flash_test_write(uint32_t addr, const uint8_t *buf,
                                       uint32_t size)
{
    struct cospi_pdata_ft *cospi = (struct cospi_pdata_ft *)flash.host->priv_data;
    addr_t apb_write = cospi->apb_base;
    uint32_t page = (uint32_t)flash.info.page_size;
    /* 256 */
    uint32_t off = 0;
    while (off < size) {
        /* 当前页边界内剩余 + 剩余数据，取 min */
        uint32_t cur_page_pos = (addr + off) % page;
        uint32_t room_in_page = page - cur_page_pos;
        uint32_t chunk = MIN(room_in_page, size - off); /* 不跨 256B page */
        uint32_t written_in_chunk = 0;

        /* 每块前发 WREN */
        if (ft_stig_cmd(apb_write, 0x6u, 0, 0)) { return -1; }
        /* 分次 PP 写，每次 ≤4B */
        while (written_in_chunk < chunk) {
            uint32_t sub = MIN((uint32_t)4, chunk - written_in_chunk);
            if (ft_stig_write_pp(apb_write, addr + off + written_in_chunk,
                                buf + off + written_in_chunk, sub)) {
                return -1;
            }
            written_in_chunk += sub;
        }
        off += chunk;
    }
    return 0;
}

/************************************************************************
 * 大闭包: 单次进 RAM + 全程关中断, 完成 整区擦除 + 整区写入。
 *
 * 修复: 多次进出 RAM 闭包 (flash_test_erase/write 每 sector 往返 XIP +
 *       arch_irq_save/restore) 的累积副作用导致连续多 sector 擦写崩溃。
 *
 * 约束: 本函数及其调用的 helper 全为 __RAM_TEXT (RAM 执行); 执行期间
 *       中断全程关闭 (NOR busy 时 XIP 取指不可靠), 系统 tick 停摆,
 *       仅限 OTA/离线场景调用。源数据 buf 可在 RAM/DDR (非 NOR)。
 ************************************************************************/
__RAM_TEXT int flash_test_program_range(uint32_t flash_off, const uint8_t *buf,
                                    uint32_t size)
{
    struct cospi_pdata_ft *cospi = (struct cospi_pdata_ft *)flash.host->priv_data;
    addr_t apb = cospi->apb_base;
    uint32_t erase_opcode = (uint32_t)(flash.info.erase_proto >> FT_SNOR_OPCODE_PROTO_LSB) & 0xFFu;
    irq_state_t old_irq;
    uint32_t off;

    old_irq = arch_irq_save();   /* 全程关中断: 擦写期间禁止任何 NOR XIP 取指 */

    /* ① 整区擦除 (4KB sector, 每 sector: WREN + sector-erase + 等 WIP) */
    for (off = 0; off < size; off += 4096u) {
        if (ft_stig_cmd(apb, 0x6u, 0, 0)) goto fail;              /* WREN */
        if (ft_stig_cmd(apb, erase_opcode, flash_off + off, 3u)) goto fail;
        if (ft_wait_flash_ready(apb)) goto fail;
    }

    /* ② 整区写入 (≤4B/PP; 每 PP 前 WREN (ft_stig_write_pp 内); 不跨 256B 页由调用方保证——
     * 这里按页边界切 chunk 传给 ft_stig_write_pp, 由它逐 ≤4B PP) */
    for (off = 0; off < size; ) {
        uint32_t page = (uint32_t)flash.info.page_size;
        uint32_t cur_page_pos = (flash_off + off) % page;
        uint32_t room = page - cur_page_pos;
        uint32_t chunk = size - off;
        uint32_t sub = 0;
        if (chunk > room) chunk = room;
        /* 页内逐 ≤4B PP (ft_stig_write_pp 内部含 WREN) */
        while (sub < chunk) {
            uint32_t n = chunk - sub;
            if (n > 4u) n = 4u;
            if (ft_stig_write_pp(apb, flash_off + off + sub,
                                 buf + off + sub, n)) goto fail;
            sub += n;
        }
        off += chunk;
    }

    arch_irq_restore(old_irq);
    return 0;
fail:
    arch_irq_restore(old_irq);
    g_probe_ret = -1;   /* RAM 探针: 闭包内不 printf */
    return -1;
}

/************************************************************************
 * 主命令：flash_test [erase|noraw]
 ************************************************************************/
int flash_test_cmd(int argc, char *argv[])
{
    uint32_t test_addr = 0x08000000 + NORFLASH_TEST_OFFSET;  /* XIP CPU 地址 */
    uint8_t backup[NORFLASH_TEST_SIZE];
    uint8_t readback[NORFLASH_TEST_SIZE];
    int ret;
    int do_erase_only = 0;

    /* gpt 子命令：XIP direct 读 SFS 镜像头验证可行性（bmu05 XIP 用 SFS 非 GPT）*/
    if (argc >= 1 && strcmp(argv[0], "gpt") == 0) {
        /* SFS 头 @0x08000000（镜像起始）: tag(0x0) + normal_img_base(0x70) + backup_img_base(0x74) + third_img_base(0x78) */
        uint32_t sfs_addr = 0x08000000u;
        uint32_t sfs[32];   /* 读 0x7C+4 = 128 字节 */
        arch_clean_invalidate_cache_range((addr_t)sfs_addr, 128);
        memcpy(sfs, (void *)(uintptr_t)sfs_addr, 128);
        DBG("[SFS] read SFS image head @0x%x (XIP direct)\n", sfs_addr);
        DBG("[SFS] tag=0x%08x (expect 0x53465301)\n", sfs[0]);
        DBG("[SFS] normal_img_base=0x%08x (A/slot0) @0x70\n", sfs[0x70/4]);
        DBG("[SFS] backup_img_base =0x%08x (B/slot1) @0x74\n", sfs[0x74/4]);
        DBG("[SFS] third_img_base =0x%08x @0x78\n", sfs[0x78/4]);
        DBG("[SFS] crc32=0x%08x @0x7c\n", sfs[0x7c/4]);
        if (sfs[0] == 0x53465301u) {
            DBG("[SFS] VALID SFS header — bmu05 XIP 确认用 SFS 镜像格式\n");
        } else {
            DBG("[SFS] tag MISMATCH (非 SFS)\n");
        }
        return 0;
    }

    /* dump 子命令：XIP direct 读任意 flash offset（对比 ssystem_a/b 内容）
     * 用法: flash_test dump <flash_off_hex> <len_hex>
     *   例:   flash_test dump 1eb000 40   -> 读 ssystem_a 起点(flash 0x1EB000)

     *        flash_test dump 2db000 40   -> 读 ssystem_b 起点(flash 0x2DB000)

     *        flash_test dump 1f5000 40   -> ssystem_a offset 0x5000

     *        flash_test dump 2e5000 40   -> ssystem_b offset 0x5000 */
    if (argc >= 1 && strcmp(argv[0], "dump") == 0) {
        uint32_t off = 0, len = 16, addr, i;
        if (argc >= 2) off = (uint32_t)strtoul(argv[1], NULL, 0);
        if (argc >= 3) len = (uint32_t)strtoul(argv[2], NULL, 0);
        if (len > 64) len = 64;
        addr = 0x08000000u + off;
        DBG("[DUMP] flash_off=0x%x (XIP addr=0x%x) len=0x%x\n", off, addr, len);
        arch_clean_invalidate_cache_range((addr_t)addr, ROUNDUP(len, 32));
        for (i = 0; i < len; i += 16) {
            uint32_t j;
            DBG("  %08x:", addr + i);
            for (j = 0; j < 16 && (i + j) < len; j++)
                DBG(" %02x", *(uint8_t *)(uintptr_t)(addr + i + j));
            DBG("\n");
        }
        return 0;
    }

    /* remap_test 子命令：精确逆向 REMAP_OSPI2 映射公式（裁决性实验）
     * 用法: flash_test remap_test
     * 手动编程 REMAP_OSPI2（直接写 SCR 寄存器，与 BM t_loader nosafety_boot_next 同参数），
     * 读窗口内/外多个地址，逆向映射公式（为何 0x081F0000 映射到 'lpdd' 而非 ssystem_b@0x5000）。
     *
     * 寄存器地址（BM scr_bits_wr + SSDK scr_lld 双公式交叉确认，修正主管 (start/32)*4 公式）：
     *   BM scr_bits_wr(b,L31,start,w,v): off=(start/32)*4+SCR_L31_OFF(0x400);
     *       addr=b+SOC_SCR_REG_MAP(off)=b+(off<<10)   [soc_share.h: SOC_SCR_REG_MAP(a)=(a)<<10]
     *   SSDK scr_lld _scr_reg: reg_off=reg_off[L31](0x400)+(start>>5)*4;
     *       addr=base+(reg_off<<reg_shift(10))        [scr_cfg.c g_scr_sec: reg_off[L31]=0x400,reg_shift=10]
     *   两者一致，APB_SCR_SEC_BASE=0xF8200000，故：
     *   OFFSET (start=608): 0xF8200000+((0x400+0x4C)<<10) = 0xF8313000  bit 0-19
     *   REMAP_EN(start=628): 同寄存器 0xF8313000  bit 20
     *   LOW    (start=640): 0xF8200000+((0x400+0x50)<<10) = 0xF8314000  bit 0-19
     *   UP     (start=672): 0xF8200000+((0x400+0x54)<<10) = 0xF8315000  bit 0-19
     *   ⚠️ 主管给的 (start/32)*4 遗漏了 <<10 移位 + SCR_L31_OFF=0x400，
     *   按 0xF820004C/50/54 直写会写错寄存器（本实验已修正为 0xF8313000/4000/5000）。
     */
    if (argc >= 1 && strcmp(argv[0], "remap_test") == 0) {
        volatile uint32_t *p;
        uint32_t reg;

        DBG("[REMAP-REV] ===== reverse-engineer REMAP_OSPI2 mapping =====\n");
        DBG("[REMAP-REV] LOW=0x81EB UP=0x82DB OFFSET=0x2DB (window 0x081EB000~0x082DB000)\n");

        /* ① 基线（EN=0，应读 ssystem_a 各 flash offset 直接内容） */
        p = (volatile uint32_t *)0x081EB000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=0 0x081EB000: %08x %08x %08x %08x\n", p[0], p[1], p[2], p[3]);
        p = (volatile uint32_t *)0x081EC000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=0 0x081EC000: %08x %08x %08x %08x\n", p[0], p[1], p[2], p[3]);
        p = (volatile uint32_t *)0x081ED000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=0 0x081ED000: %08x %08x %08x %08x\n", p[0], p[1], p[2], p[3]);
        p = (volatile uint32_t *)0x081F0000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=0 0x081F0000: %08x %08x %08x %08x\n", p[0], p[1], p[2], p[3]);
        p = (volatile uint32_t *)0x08000000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=0 0x08000000: %08x %08x %08x %08x (anchor, outside window)\n", p[0], p[1], p[2], p[3]);

        /* ② 编程 OFFSET（先清 EN 再写，避免半编程态） */
        reg = readl(0xF8313000u);
        reg &= ~(1u << 20);              /* EN=0 */
        writel(reg, 0xF8313000u);
        reg = readl(0xF8313000u);
        reg &= ~(0xFFFFFu);              /* 清 bit0-19 OFFSET */
        reg |= (0x2DBu & 0xFFFFFu);
        writel(reg, 0xF8313000u);

        /* ③ 编程 LOW / UP */
        reg = readl(0xF8314000u);
        reg &= ~(0xFFFFFu);
        reg |= (0x81EBu & 0xFFFFFu);
        writel(reg, 0xF8314000u);

        reg = readl(0xF8315000u);
        reg &= ~(0xFFFFFu);
        reg |= (0x82DBu & 0xFFFFFu);
        writel(reg, 0xF8315000u);

        /* ④ 读回验证 SCR 寄存器值 */
        DBG("[REMAP-REV] SCR readback: OFF/EN reg=0x%08x LOW=0x%08x UP=0x%08x\n",
            readl(0xF8313000u), readl(0xF8314000u), readl(0xF8315000u));

        /* ⑤ 使能 REMAP（EN=1） */
        reg = readl(0xF8313000u);
        reg |= (1u << 20);
        writel(reg, 0xF8313000u);
        DBG("[REMAP-REV] REMAP_EN=1, OFF/EN reg=0x%08x\n", readl(0xF8313000u));

        /* ⑥ 读窗口内/外多地址（EN=1，观察映射后的值） */
        p = (volatile uint32_t *)0x081EB000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=1 0x081EB000: %08x %08x %08x %08x\n", p[0], p[1], p[2], p[3]);
        p = (volatile uint32_t *)0x081EC000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=1 0x081EC000: %08x %08x %08x %08x\n", p[0], p[1], p[2], p[3]);
        p = (volatile uint32_t *)0x081ED000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=1 0x081ED000: %08x %08x %08x %08x\n", p[0], p[1], p[2], p[3]);
        p = (volatile uint32_t *)0x081F0000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=1 0x081F0000: %08x %08x %08x %08x\n", p[0], p[1], p[2], p[3]);
        p = (volatile uint32_t *)0x08000000;
        arch_clean_invalidate_cache_range((addr_t)p, 32);
        DBG("[REMAP-REV] EN=1 0x08000000: %08x %08x %08x %08x (anchor)\n", p[0], p[1], p[2], p[3]);

        /* ⑦ 关闭 REMAP（恢复） */
        reg = readl(0xF8313000u);
        reg &= ~(1u << 20);
        writel(reg, 0xF8313000u);
        DBG("[REMAP-REV] REMAP disabled (EN=0), OFF/EN reg=0x%08x\n", readl(0xF8313000u));

        return 0;
    }


    /* ota_sim 子命令：模拟 OTA 更新 ssystem_b（从 ssystem_a 整段拷贝 960KB + CRC32 校验）
     * 用法: flash_test ota_sim
     * 前提: 当前运行在 slot_a（XIP 从 ssystem_a 取指），读源安全
     * 结果: ssystem_b 内容 == ssystem_a 内容（含 padding）→ vbmeta hash 匹配 → FDA verify PASS */
    if (argc >= 1 && strcmp(argv[0], "ota_sim") == 0) {
        uint32_t src_off = 0x1EB000;           /* ssystem_a flash offset */
        uint32_t dst_off = 0x2DB000;           /* ssystem_b flash offset */
        uint32_t total   = 0xF0000;            /* 960KB = SSDK_SEC_MEMSIZE */
        uint32_t chunk   = 4096;               /* 4KB per sector */
        uint32_t done    = 0;
        uint32_t crc_src = 0, crc_dst = 0;
        int ret;
        static uint8_t buf[4096];              /* static 避免 stack overflow */

        DBG("[OTA-SIM] copy ssystem_a(0x%x) -> ssystem_b(0x%x), total=0x%x (%u sectors)\n",
            src_off, dst_off, total, total / chunk);

        while (done < total) {
            uint32_t len = (total - done < chunk) ? (total - done) : chunk;

            /* ① XIP direct 读源（ssystem_a） */
            arch_clean_invalidate_cache_range((addr_t)(0x08000000u + src_off + done),
                                              ROUNDUP(len, 32));
            memcpy(buf, (void *)(uintptr_t)(0x08000000u + src_off + done), len);

            /* ② 累计源 CRC */
            crc_src = crc32(crc_src, buf, len);

            /* ③ STIG 擦+写目标（ssystem_b） */
            ret = flash_test_erase(dst_off + done);
            if (ret) {
                DBG("[OTA-SIM] ERASE FAILED @0x%x ret=%d, abort at %u/%u\n",
                    dst_off + done, ret, done, total);
                return 0;
            }
            ret = flash_test_write(dst_off + done, buf, len);
            if (ret) {
                DBG("[OTA-SIM] WRITE FAILED @0x%x ret=%d, abort at %u/%u\n",
                    dst_off + done, ret, done, total);
                return 0;
            }

            done += len;

            /* 每 40 个 sector（160KB）打印进度 */
            if ((done % (chunk * 40)) == 0 || done == total) {
                DBG("[OTA-SIM] progress: %u/%u bytes (%u%%)\n",
                    done, total, done * 100 / total);
            }
        }

        /* ④ 校验：重读 ssystem_b 全部，累计 CRC */
        DBG("[OTA-SIM] write done, verifying...\n");
        done = 0;
        while (done < total) {
            uint32_t len = (total - done < chunk) ? (total - done) : chunk;
            arch_clean_invalidate_cache_range((addr_t)(0x08000000u + dst_off + done),
                                              ROUNDUP(len, 32));
            memcpy(buf, (void *)(uintptr_t)(0x08000000u + dst_off + done), len);
            crc_dst = crc32(crc_dst, buf, len);
            done += len;
        }

        DBG("[OTA-SIM] CRC32 src=0x%08x dst=0x%08x %s\n",
            crc_src, crc_dst,
            (crc_src == crc_dst) ? "MATCH - OTA SIM SUCCESS" : "MISMATCH - FAILED");

        if (crc_src == crc_dst) {
            DBG("[OTA-SIM] ssystem_b updated. Now run: slot_switch b && reboot\n");
        } else {
            DBG("[OTA-SIM] FAILED. ssystem_b corrupted. Run slot_switch a to stay on slot_a\n");
        }

        /* ⑤ 拷贝 vbmeta-ssdk_a → vbmeta-ssdk_b（PAC 只烧 _a，_b 未写入，FDA verify 需要 _b 有相同 hash）
         * GPT 实测 offset：vbmeta-ssdk_a @0x1CB000, vbmeta-ssdk_b @0x1DB000，各 64KB (0x10000)
         */
        const uint32_t v_src_off = 0x1CB000;        /* vbmeta-ssdk_a flash offset */
        const uint32_t v_dst_off = 0x1DB000;        /* vbmeta-ssdk_b flash offset */
        const uint32_t v_total   = 0x10000;         /* 64KB 分区大小 */
        uint32_t v_done = 0;
        uint32_t v_crc_src = 0, v_crc_dst = 0;

        DBG("[OTA-SIM] copy vbmeta-ssdk_a(0x%x) -> vbmeta-ssdk_b(0x%x), total=0x%x (%u sectors)\n",
            v_src_off, v_dst_off, v_total, v_total / chunk);

        while (v_done < v_total) {
            uint32_t len = (v_total - v_done < chunk) ? (v_total - v_done) : chunk;

            /* XIP direct 读源（vbmeta-ssdk_a） */
            arch_clean_invalidate_cache_range((addr_t)(0x08000000u + v_src_off + v_done),
                                              ROUNDUP(len, 32));
            memcpy(buf, (void *)(uintptr_t)(0x08000000u + v_src_off + v_done), len);
            v_crc_src = crc32(v_crc_src, buf, len);

            /* STIG 擦+写目标（vbmeta-ssdk_b） */
            ret = flash_test_erase(v_dst_off + v_done);
            if (ret) {
                DBG("[OTA-SIM] VBMETA ERASE FAILED @0x%x ret=%d\n",
                    v_dst_off + v_done, ret);
                return 0;
            }
            ret = flash_test_write(v_dst_off + v_done, buf, len);
            if (ret) {
                DBG("[OTA-SIM] VBMETA WRITE FAILED @0x%x ret=%d\n",
                    v_dst_off + v_done, ret);
                return 0;
            }

            v_done += len;
        }

        /* ⑥ 校验：重读 vbmeta-ssdk_b 全部，累计 CRC */
        DBG("[OTA-SIM] vbmeta write done, verifying...\n");
        v_done = 0;
        while (v_done < v_total) {
            uint32_t len = (v_total - v_done < chunk) ? (v_total - v_done) : chunk;
            arch_clean_invalidate_cache_range((addr_t)(0x08000000u + v_dst_off + v_done),
                                              ROUNDUP(len, 32));
            memcpy(buf, (void *)(uintptr_t)(0x08000000u + v_dst_off + v_done), len);
            v_crc_dst = crc32(v_crc_dst, buf, len);
            v_done += len;
        }

        DBG("[OTA-SIM] vbmeta CRC32 src=0x%08x dst=0x%08x %s\n",
            v_crc_src, v_crc_dst,
            (v_crc_src == v_crc_dst) ? "MATCH - VBMETA OTA SIM OK" : "MISMATCH - VBMETA FAILED");

        return 0;
    }

    DBG("[1] enter flash_test_cmd argc=%d\n", argc);
    if (argc >= 1 && strcmp(argv[0], "erase") == 0) {
        do_erase_only = 1;
    }

    /* loop 子命令: 连续多 sector 擦+写+验 (模拟 OTA 长跑) */
    if (argc >= 2 && strcmp(argv[0], "loop") == 0) {
        int n = atoi(argv[1]);
        int i;
        if (n < 1) n = 1;
        if (n > 55) n = 55;   /* 安全区 0x3C9000~0x400000 = 55 sectors */
        DBG("[LOOP] erase+write+verify %d sectors @0x3C9000+ ...\n", n);
        for (i = 0; i < n; i++) {
            uint32_t off = NORFLASH_TEST_OFFSET + (uint32_t)i * 4096u;
            DBG("[LOOP][%d/%d] erase+write+verify @0x%x\n", i + 1, n, off);
            if (flash_test_erase(off) != 0) { DBG("[LOOP] erase FAIL @0x%x\n", off); break; }
            __asm volatile("isb" ::: "memory");
            arch_invalidate_cache_range(0x08000000u + off, 4096);
            memset(backup, 0xAA, sizeof(backup));
            if (flash_test_write(off, backup, sizeof(backup)) != 0) { DBG("[LOOP] write FAIL @0x%x\n", off); break; }
            __asm volatile("isb" ::: "memory");
            arch_invalidate_cache_range(0x08000000u + off, 4096);
            memset(readback, 0, sizeof(readback));
            flash_test_read(0x08000000u + off, readback, sizeof(readback));
            if (memcmp(backup, readback, sizeof(backup)) != 0) {
                DBG("[LOOP] sector %d VERIFY FAIL\n", i + 1);
                break;
            }
        }
        DBG("[LOOP] done\n");
        return 0;
    }


    DBG("[2] test_addr=0x%x (XIP), size=%u, erase_only=%d\n",
        test_addr, NORFLASH_TEST_SIZE, do_erase_only);

    /* ① 准备 0xAA 测试模式（非 0xff，用于验证写入是否真正发生）*/
    if (!do_erase_only) {
        DBG("[3] prepare 0xAA test pattern @0x%x\n", test_addr);
        memset(backup, 0xAA, sizeof(backup));  /* 0xAA 标记：区分于擦除态 0xff */
        DBG("[4] pattern: 0x%02x 0x%02x 0x%02x 0x%02x\n",
            backup[0], backup[1], backup[2], backup[3]);
    }

    /* ② 擦除测试区 */
    DBG("[5] erase @0x%x (flash offset)\n", NORFLASH_TEST_OFFSET);
    ret = flash_test_erase(NORFLASH_TEST_OFFSET);
    if (ret) {
        DBG("[6] erase FAILED ret=%d --- 擦除即崩溃/失败(可能需搬RAM)\n", ret);
        return 0;
    }

    __asm volatile("isb" ::: "memory");
    /* 擦除后 invalidate 被改测试区 D-cache（Oracle Q4）——否则后续从 flash 读该区命中脏 cache 行 */
    arch_invalidate_cache_range(0x08000000u + NORFLASH_TEST_OFFSET, ROUNDUP(NORFLASH_TEST_SIZE, 32));

    if (do_erase_only) {
        DBG("[7] erase-only DONE (no crash). 擦除走 STIG 安全\n");
        return 0;
    }

    /* ③ 写回备份 */
    DBG("[7] write back @0x%x\n", test_addr);
    ret = flash_test_write(NORFLASH_TEST_OFFSET, backup, sizeof(backup));  /* 用 flash offset(0x3c9000) 非 CPU 地址 */
    if (ret) {
        DBG("[8] write FAILED ret=%d --- 写路径崩溃/失败(需搬RAM)\n", ret);
        return 0;
    }
    __asm volatile("isb" ::: "memory");

    /* ④ 写回校验（cache 是否污染） */
    DBG("[9] verify readback @0x%x\n", test_addr);
    memset(readback, 0, sizeof(readback));
    flash_test_read(test_addr, readback, sizeof(readback));

    ret = memcmp(backup, readback, sizeof(backup));
    if (ret != 0) {
        int i;
        /* 定位第一个不一致字节 */
        for (i = 0; i < (int)sizeof(backup); i++) {
            if (backup[i] != readback[i]) break;
        }
        DBG("[10] verify FAIL: first mismatch @offset %d: 0x%02x (want 0x%02x)\n",
            i, readback[i], backup[i]);
    } else {
        DBG("[10] verify PASS-write-verified\n");
    }
    DBG("     head: readback 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
        readback[0], readback[1], readback[2], readback[3],
        readback[4], readback[5], readback[6], readback[7]);

    DBG("[11] done, return 0\n");
    return 0;   /* CLI: MUST return 0 (non-zero = repeat, infinite loop) */
}

CLI_CMD("flash_test", "OSPI2 NOR XIP erase/write min-verify: flash_test [erase|noraw]", flash_test_cmd);
