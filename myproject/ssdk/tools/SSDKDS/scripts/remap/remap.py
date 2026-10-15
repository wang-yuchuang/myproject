from file_ops import FileBase
import project

# core_map
core_map = {
    'safety'    : 'SAF',
    'secure'    : 'SEC',
    'MP'        : 'SEC',
}

romc_map = {
    'safety'    : 'ROMC1',
    'secure'    : 'ROMC2',
    'MP'        : 'ROMC2',
}

scr_addr_map = {
    'safety'    : 'SCR_SAFETY__remap_cr5_saf_ar_addr_offset_19_0',
    'secure'    : 'SCR_SEC__remap_cr5_sec_ar_addr_offset_19_0',
    'MP'        : 'SCR_SEC__remap_cr5_mp_ar_addr_offset_19_0',
}

scr_en_map = {
    'safety'    : 'SCR_SAFETY__remap_cr5_saf_ar_remap_ovrd_en',
    'secure'    : 'SCR_SEC__remap_cr5_sec_ar_remap_ovrd_en',
    'MP'        : 'SCR_SEC__remap_cr5_mp_ar_remap_ovrd_en',
}

reset_core_map = {
    'safety'    : 'rstsig_cr5_saf',
    'secure'    : 'rstsig_cr5_sec',
    'mp'        : 'rstsig_cr5_mp',
}

# for remap.h file
class RemapIncFile(FileBase):
    prefix = FileBase.prefix + """
#ifndef BOARDS_REMAP_H
#define BOARDS_REMAP_H

#ifdef CONFIG_BOARD_REMAP
"""

    suffix = """

extern void board_remap(void);

#endif

#endif /* BOARDS_REMAP_H */
"""

# for remap.c file
class RemapSrcFile(FileBase):
    prefix = FileBase.prefix + """
#ifdef CONFIG_BOARD_REMAP

#include <types.h>
#include <compiler.h>
#include <reg.h>

#include <driver/kunlun/reset/sdrv_reset_drv.h>
#include <driver/scr.h>
#include <scr_hw.h>
#include <rstgen_cfg.h>
#include <regs_base.h>

#include "remap.h"
"""

    template = """
#define APB_RSTGEN_BASE         %s
#define APB_SCR_BASE            %s
#define APB_ROMC_BASE           %s

#define SCR_REMAP_ADDR_OFFSET   %s
#define SCR_REMAP_OVRD_EN       %s

#define RSTGEN_CORE             &%s

#define RSTGEN_GENERAL_REG(n)   ((0x50 + (n)*4) << 10)
#define RSTGEN_REMAP_STATUS_REG (APB_RSTGEN_BASE + RSTGEN_GENERAL_REG(1))
#define REMAP_DONE              (0x52454d50) /* 'REMP' */
#define ROMC_STICKY_REG         (APB_ROMC_BASE + 0x34)

static void vector_remap(addr_t vector_base)
{
    /* base address must be 4 KB aligned */
    //writel(REMAP_DONE, RSTGEN_REMAP_STATUS_REG);
    /* Enable R5 remapping to vector base. The remap config doesn't
     * take effect until REMAP module detects R5 core reset.
     */
    scr_signal_t remap_addr_signal = SCR_REMAP_ADDR_OFFSET;
    scr_signal_t remap_en_signal = SCR_REMAP_OVRD_EN;
    writel(REMAP_DONE, RSTGEN_REMAP_STATUS_REG);
    scr_set(APB_SCR_BASE, &remap_addr_signal, vector_base >> 12);
    scr_set(APB_SCR_BASE, &remap_en_signal, 1);
    /* ROMC Stick Reg
     *
     * [0]: ROMC_STICKY_REMAP_EN. This bit is AND'ed with
     *      SCR remap_en bit, to enable remapping.
     */
    RMWREG32(ROMC_STICKY_REG, 0, 1, 1);
}

void board_remap(void)
{
    if (readl((RSTGEN_REMAP_STATUS_REG)) != REMAP_DONE) {
        vector_remap(CONFIG_MEM_BASE);
        reset_ctl_reset(RSTGEN_CORE);
        while (1) {
            __ASM volatile("wfi");
        }
    } else {
        writel(0, (RSTGEN_REMAP_STATUS_REG));
    }
}
"""

    suffix = """
#endif"""


def gen(project):

    with RemapIncFile(project.get_generated_path('remap.h')) as remap_inc:
        remap_inc.write('')

    with RemapSrcFile(project.get_generated_path('remap.c')) as remap_src:
        remap_src.write(remap_src.template % 
        (('APB_RSTGEN_%s_BASE' % (core_map[project.core])),
        ('APB_SCR_%s_BASE' % (core_map[project.core])),
        ('APB_%s_BASE' % (romc_map[project.core])),
        scr_addr_map[project.core], scr_en_map[project.core],
        reset_core_map[project.core]))