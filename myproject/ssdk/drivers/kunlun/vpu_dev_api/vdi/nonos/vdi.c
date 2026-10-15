//------------------------------------------------------------------------------
// File: vdi.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#include "../vdi.h"
#include "../vdi_osal.h"
#include "coda9/coda9_regdefine.h"
#include "vpuapi.h"
#include "wave/common/common_regdefine.h"
#include "wave/wave4/wave4_regdefine.h"
#include "wave/wave5/wave5_regdefine.h"
#include <arch/cache.h>
#include <cmsis_os2.h>
#include <driver/irq.h>
#include <driver/memmap.h>
#include <irq_num.h>
#include <malloc.h>
#include <reg.h>
#include <regs_base.h>
#include <stdio.h>
#include <stdlib.h>

#define VDI_SYSTEM_ENDIAN VDI_LITTLE_ENDIAN
#define VDI_128BIT_BUS_SYSTEM_ENDIAN VDI_128BIT_LITTLE_ENDIAN

#ifdef CONFIG_BOARD_X9SP
#define VPU_CODA988_INTRPT_NUM VPU2_O_HOST_INTRPT_NUM
#define VPU_WAVE412_INTRPT_NUM VPU1_O_VPU_INTRPT_NUM
#define VPU_WAVE521_INTRPT_NUM VPU4_O_HOST_INTRPT_NUM

#define VPU_CODA988_INTERNAL_LINE_BUFFER_ADDR 0x2500000
#define VPU_CODA988_INTERNAL_LINE_BUFFER_SIZE 0x2A000
#define VPU_CODA988_SRAM_ADDR 0x00500000
#define VPU_CODA988_SRAM_SIZE 0x00040000
#define VPU_WAVE412_SRAM_ADDR 0x00540000
#define VPU_WAVE412_SRAM_SIZE 0x00040000
#define VPU_WAVE521_SRAM_ADDR 0x00580000
#define VPU_WAVE521_SRAM_SIZE 0x00080000
#else /* !CONFIG_BOARD_X9SP */
#define VPU_CODA988_INTRPT_NUM VPU2_O_HOST_INTR_NUM
#define VPU_WAVE412_INTRPT_NUM VPU1_O_VPU_INTR_NUM
#define VPU_WAVE521_INTRPT_NUM 0x0

#define VPU_CODA988_INTERNAL_LINE_BUFFER_ADDR 0x00D00000
#define VPU_CODA988_INTERNAL_LINE_BUFFER_SIZE 0x2A000
#define VPU_CODA988_SRAM_ADDR 0x00180000
#define VPU_CODA988_SRAM_SIZE 0x40000
#define VPU_WAVE412_SRAM_ADDR 0x0
#define VPU_WAVE412_SRAM_SIZE 0x0
#define VPU_WAVE521_SRAM_ADDR 0x0
#define VPU_WAVE521_SRAM_SIZE 0x0
#endif /* end CONFIG_BOARD_X9SP */

#define VPU_INTRPT_FLAG 0x01
#define VPU_HW_INTERRUPT

typedef struct vpu_buffer_t vpudrv_buffer_t;
typedef struct vpu_buffer_pool_t {
    vpudrv_buffer_t vdb;
    int inuse;
} vpu_buffer_pool_t;

typedef struct {
    unsigned long core_idx;
    int vpu_fd;
    vpu_instance_pool_t *pvip;
    int task_num;
    int clock_state;
    vpu_buffer_t vdb_register;
    vpu_buffer_t vpu_common_memory;
    vpu_buffer_pool_t vpu_buffer_pool[MAX_VPU_BUFFER_POOL];
    int product_code;
} vdi_info_t;

static vdi_info_t s_vdi_info[MAX_NUM_VPU_CORE];
vpu_instance_pool_t s_vip[MAX_VPU_CORE_NUM];

osEventFlagsId_t vpu_coda988_interrupt_signal = NULL;
osEventFlagsId_t vpu_wave412_interrupt_signal = NULL;
osEventFlagsId_t vpu_wave521_interrupt_signal = NULL;
unsigned int vpu_coda988_interrupt_status;
unsigned int vpu_wave412_interrupt_status;
unsigned int vpu_wave521_interrupt_status;

static int swap_endian(unsigned long core_idx, unsigned char *data, int len, int endian);
static int allocate_common_memory(unsigned long core_idx);

static int32_t vpu_coda988_interrupt_handler(uint32_t irq_num, void *arg)
{
    vdi_info_t *vdi = (vdi_info_t *)arg;

    if ((vdi->product_code == CODA980_CODE) && vpu_coda988_interrupt_signal) {
        if (vdi_read_register(vdi->core_idx, BIT_INT_STS))
            vpu_coda988_interrupt_status = vdi_read_register(vdi->core_idx, BIT_INT_REASON);
        vdi_write_register(vdi->core_idx, BIT_INT_CLEAR, 0x1);
        osEventFlagsSet(vpu_coda988_interrupt_signal, VPU_INTRPT_FLAG);
    }

    return 0;
}

static int32_t vpu_wave412_interrupt_handler(uint32_t irq_num, void *arg)
{
    vdi_info_t *vdi = (vdi_info_t *)arg;

    if ((vdi->product_code == WAVE412_CODE) && vpu_wave412_interrupt_signal) {
        if (vdi_read_register(vdi->core_idx, W4_VPU_VPU_INT_STS))
            vpu_wave412_interrupt_status = vdi_read_register(vdi->core_idx, W4_VPU_VINT_REASON);
        vdi_write_register(vdi->core_idx, W4_VPU_VINT_REASON_CLR, vpu_wave412_interrupt_status);
        vdi_write_register(vdi->core_idx, W4_VPU_VINT_CLEAR, 0x1);
        osEventFlagsSet(vpu_wave412_interrupt_signal, VPU_INTRPT_FLAG);
    }

    return 0;
}

static int32_t vpu_wave521_interrupt_handler(uint32_t irq_num, void *arg)
{
    vdi_info_t *vdi = (vdi_info_t *)arg;

    if ((vdi->product_code == WAVE521C_CODE) && vpu_wave521_interrupt_signal) {
        if (vdi_read_register(vdi->core_idx, W5_VPU_VPU_INT_STS))
            vpu_wave521_interrupt_status = vdi_read_register(vdi->core_idx, W5_VPU_VINT_REASON);

        vdi_write_register(vdi->core_idx, W5_VPU_VINT_REASON_CLR, vpu_wave521_interrupt_status);
        vdi_write_register(vdi->core_idx, W5_VPU_VINT_CLEAR, 0x1);
        osEventFlagsSet(vpu_wave521_interrupt_signal, VPU_INTRPT_FLAG);
    }

    return 0;
}

int vdi_init(unsigned long phy_base, unsigned int reg_range, unsigned long core_idx)
{
    vdi_info_t *vdi;
    int i;
    Uint32 product_code;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return 0;

    vdi = &s_vdi_info[core_idx];

    if (vdi->vpu_fd != -1 && vdi->vpu_fd != 0x00) {
        vdi->task_num++;
        return 0;
    }

    vdi->vpu_fd = 1;

    {
        vdi->vdb_register.size = reg_range;
        vdi->vdb_register.virt_addr = phy_base;
        vdi->vdb_register.phys_addr = (unsigned long)ap2p((paddr_t)phy_base);
        vdi->vdb_register.base = vdi->vdb_register.phys_addr;
    }

    osal_memset(&vdi->vpu_buffer_pool, 0x00, sizeof(vpu_buffer_pool_t) * MAX_VPU_BUFFER_POOL);

    if (!vdi_get_instance_pool(core_idx)) {
        VLOG(ERR, "[VDI] fail to create shared info for saving context \n");
        goto ERR_VDI_INIT;
    }

    if (allocate_common_memory(core_idx) < 0) {
        VLOG(ERR, "[VDI] fail to get vpu common buffer from driver\n");
        goto ERR_VDI_INIT;
    }

    // VLOG(INFO, "[VDI] vpu reg paddr:%lx - vaddr:%lx \n",vdi->vdb_register->phys_addr,
    // vdi->vdb_register->virt_addr);

    if (vdi_lock(core_idx) < 0) {
        VLOG(ERR, "[VDI] fail to handle lock function\n");
        goto ERR_VDI_INIT;
    }
    vdi_set_clock_gate(core_idx, TRUE);
    vdi->product_code = vdi_read_register(core_idx, VPU_PRODUCT_CODE_REGISTER);
    product_code = vdi->product_code;
    VLOG(INFO, "[VDI] Product code: %x \n", vdi->product_code);

    if (product_code == WAVE412_CODE) {
        if (vdi_read_register(core_idx, W4_VCPU_CUR_PC) == 0) // if BIT processor is not running.
        {
            for (i = 0; i < 64; i++)
                vdi_write_register(core_idx, (i * 4) + 0x100, 0x0);
        }

        irq_attach(VPU_WAVE412_INTRPT_NUM, vpu_wave412_interrupt_handler, vdi);
        irq_enable(VPU_WAVE412_INTRPT_NUM);
        vpu_wave412_interrupt_signal = osEventFlagsNew(NULL);
    } else if (product_code == CODA980_CODE) {
        if (vdi_read_register(core_idx, BIT_CUR_PC) == 0) // if BIT processor is not running.
        {
            for (i = 0; i < 64; i++)
                vdi_write_register(core_idx, (i * 4) + 0x100, 0x0);
        }

        irq_attach(VPU_CODA988_INTRPT_NUM, vpu_coda988_interrupt_handler, vdi);
        irq_enable(VPU_CODA988_INTRPT_NUM);
        vpu_coda988_interrupt_signal = osEventFlagsNew(NULL);
    } else if (product_code == WAVE521C_CODE) {
        irq_attach(VPU_WAVE521_INTRPT_NUM, vpu_wave521_interrupt_handler, vdi);
        irq_enable(VPU_WAVE521_INTRPT_NUM);
        vpu_wave521_interrupt_signal = osEventFlagsNew(NULL);
    } else {
        VLOG(ERR, "[VDI] Unknown product id : %08x\n", vdi->product_code);
        vdi_set_clock_gate(core_idx, FALSE);
        vdi_unlock(core_idx);
        goto ERR_VDI_INIT;
    }

    vdi_set_clock_gate(core_idx, FALSE);
    vdi_unlock(core_idx);

    vdi->core_idx = core_idx;
    vdi->task_num++;

    VLOG(INFO, "[VDI] success to init driver \n");

    return 0;

ERR_VDI_INIT:
    vdi_release(core_idx);
    return -1;
}

int vdi_set_bit_firmware_to_pm(unsigned long core_idx, const unsigned short *code) { return 0; }

int vdi_release(unsigned long core_idx)
{
    int i;
    vpudrv_buffer_t vdb;
    vdi_info_t *vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return 0;
    vdb.size = 0;

    if (vdi_lock(core_idx) < 0) {
        VLOG(ERR, "[VDI] fail to handle lock function\n");
        return -1;
    }

    if (vdi->task_num > 1) // means that the opened instance remains
    {
        vdi->task_num--;
        vdi_unlock(core_idx);
        return 0;
    }

    osal_memset(&vdi->vdb_register, 0x00, sizeof(vpudrv_buffer_t));

    // get common memory information to free virtual address
    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 1 &&
            vdi->vpu_common_memory.phys_addr >= vdi->vpu_buffer_pool[i].vdb.phys_addr &&
            vdi->vpu_common_memory.phys_addr <
                (vdi->vpu_buffer_pool[i].vdb.phys_addr + vdi->vpu_buffer_pool[i].vdb.size)) {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    if (vdb.size > 0) {
        osal_free((void *)vdi->vpu_common_memory.virt_addr);
        osal_memset(&vdi->vpu_common_memory, 0x00, sizeof(vpu_buffer_t));
    }

    if (vdi->product_code == WAVE412_CODE) {
        irq_disable(VPU_WAVE412_INTRPT_NUM);
        osEventFlagsDelete(vpu_wave412_interrupt_signal);
        vpu_wave412_interrupt_signal = NULL;
    } else if (vdi->product_code == WAVE521C_CODE) {
        irq_disable(VPU_WAVE521_INTRPT_NUM);
        osEventFlagsDelete(vpu_wave521_interrupt_signal);
        vpu_wave521_interrupt_signal = NULL;
    } else if (vdi->product_code == CODA980_CODE) {
        irq_disable(VPU_CODA988_INTRPT_NUM);
        osEventFlagsDelete(vpu_coda988_interrupt_signal);
        vpu_coda988_interrupt_signal = NULL;
    } else {
        VLOG(ERR, "[VDI][%s] Unknown product id : %08x\n", __func__, vdi->product_code);
    }

    vdi->core_idx = -1;
    vdi->task_num = 0;
    vdi->vpu_fd = -1;

    vdi_unlock(core_idx);

    osal_memset(vdi, 0x00, sizeof(vdi_info_t));

    VLOG(INFO, "[VDI] vdi ctrl blk%d released \n", core_idx);
    return 0;
}

int vdi_get_common_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memcpy(vb, &vdi->vpu_common_memory, sizeof(vpu_buffer_t));

    return 0;
}

/**
*@Brief: vdi_device_memory_map   Map device memory
        getting dma_addr should mmap memory,
        component can use function to get dma adrress according
        dma_handle.
        dma address attach one master device
*@param: core_idx coda988 or wave412
*@param: vdb description of memory info
*@return 0 success  -1 failure
*/
int vdi_device_memory_map(unsigned long core_idx, vpu_buffer_t *vdb) { return 0; }

/**
 *@Brief: vdi_device_memory_unmap  unmap device memory
 *@param: core_idx coda988 or wave412
 *@param: vdb description of memory info
 *@return void
 */
void vdi_device_memory_unmap(unsigned long core_idx, vpu_buffer_t *vdb) {}

int allocate_common_memory(unsigned long core_idx)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];
    vpudrv_buffer_t vdb = {0};
    int i;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    vdb.size = SIZE_COMMON;
    vdb.virt_addr = (unsigned long)memalign(0x1000, SIZE_COMMON);

    if (!vdb.virt_addr) {
        VLOG(ERR, "[VDI] fail to allocate vdb mem! \n");
        return -1;
    }

    vdb.phys_addr = (unsigned long)p2ap((paddr_t)vdb.virt_addr);
    vdb.base = vdb.phys_addr;

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 0) {
            vdi->vpu_buffer_pool[i].vdb = vdb;
            vdi->vpu_buffer_pool[i].inuse = 1;
            break;
        }
    }

    if (i >= MAX_VPU_BUFFER_POOL) {
        VLOG(ERR, "[%s][%d] vpu buffer pool is full\n", __func__, __LINE__);
        osal_free((void *)vdb.virt_addr);
        return -1;
    }

    // convert os driver buffer type to vpu buffer type
    vdi->pvip->vpu_common_buffer.size = SIZE_COMMON;
    vdi->pvip->vpu_common_buffer.phys_addr = (unsigned int)(vdb.phys_addr);
    vdi->pvip->vpu_common_buffer.base = (unsigned long)(vdb.base);
    vdi->pvip->vpu_common_buffer.virt_addr = (unsigned long)(vdb.virt_addr);

    osal_memcpy(&vdi->vpu_common_memory, &vdi->pvip->vpu_common_buffer, sizeof(vpudrv_buffer_t));

    VLOG(TRACE, "[VDI] vdi_get_common_memory physaddr=0x%llx, size=%x, virtaddr=0x%llx\n",
         vdi->vpu_common_memory.phys_addr, vdi->vpu_common_memory.size,
         vdi->vpu_common_memory.virt_addr);

    return 0;
}

vpu_instance_pool_t *vdi_get_instance_pool(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_VPU_CORE_NUM)
        return NULL;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return NULL;

    if (!vdi->pvip) {

        vdi->pvip = &s_vip[core_idx];

        osal_memset(vdi->pvip, 0, sizeof(vpu_instance_pool_t));
    }

    return (vpu_instance_pool_t *)vdi->pvip;
}

int vdi_open_instance(unsigned long core_idx, unsigned long instIdx)
{
    vdi_info_t *vdi = NULL;

    if (core_idx >= MAX_VPU_CORE_NUM)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    vdi->pvip->vpu_instance_num++;

    return 0;
}

int vdi_close_instance(unsigned long core_idx, unsigned long instIdx)
{
    vdi_info_t *vdi = NULL;

    if (core_idx >= MAX_VPU_CORE_NUM)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    vdi->pvip->vpu_instance_num--;

    return 0;
}

int vdi_get_instance_num(unsigned long core_idx)
{
    vdi_info_t *vdi = NULL;

    if (core_idx >= MAX_VPU_CORE_NUM)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    return vdi->pvip->vpu_instance_num;
}

int vdi_hw_reset(unsigned long core_idx) // DEVICE_ADDR_SW_RESET
{
    vdi_info_t *vdi = NULL;

    if (core_idx >= MAX_VPU_CORE_NUM)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    // to do any action for hw reset

    return 0;
}

int vdi_lock(unsigned long core_idx) { return 0; }

int vdi_lock_check(unsigned long core_idx)
{
    /* need to implement */
    return 0;
}

void vdi_unlock(unsigned long core_idx) { return; }

int vdi_disp_lock(unsigned long core_idx)
{
    /* need to implement */
    return 0;
}

void vdi_disp_unlock(unsigned long core_idx)
{
    /* need to implement */
    return;
}

void vdi_write_register(unsigned long core_idx, unsigned int addr, unsigned int data)
{
    vdi_info_t *vdi;
    unsigned long *reg_addr;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);

    writel(data, reg_addr);
}

unsigned int vdi_read_register(unsigned long core_idx, unsigned int addr)
{
    vdi_info_t *vdi;
    unsigned long *reg_addr;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return (unsigned int)-1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return (unsigned int)-1;

    reg_addr = (unsigned long *)(addr + (unsigned long)vdi->vdb_register.virt_addr);
    return readl(reg_addr);
}

#define FIO_TIMEOUT 100

unsigned int vdi_fio_read_register(unsigned long core_idx, unsigned int addr)
{
    unsigned int ctrl;
    vdi_info_t *vdi = NULL;
    unsigned int count = 0;
    unsigned int data = 0xffffffff;
    unsigned int fio_data = 0;
    unsigned int fio_ctrl_addr = 0;

    ctrl = (addr & 0xffff);
    ctrl |= (0 << 16); /* read operation */

    vdi = &s_vdi_info[core_idx];

    switch (vdi->product_code) {
    case WAVE412_CODE:
        fio_data = W4_VPU_FIO_DATA;
        fio_ctrl_addr = W4_VPU_FIO_CTRL_ADDR;
        break;
    case WAVE521_CODE:
    case WAVE521C_CODE:
        fio_data = W5_VPU_FIO_DATA;
        fio_ctrl_addr = W5_VPU_FIO_CTRL_ADDR;
        break;
    default:
        VLOG(ERR, "%s unknow product id %x \n", __func__, vdi->product_code);
        return 0;
    }

    vdi_write_register(core_idx, fio_ctrl_addr, ctrl);
    count = FIO_TIMEOUT;

    while (count--) {
        ctrl = vdi_read_register(core_idx, fio_ctrl_addr);

        if (ctrl & 0x80000000) {
            data = vdi_read_register(core_idx, fio_data);
            break;
        }
    }

    return data;
}

void vdi_fio_write_register(unsigned long core_idx, unsigned int addr, unsigned int data)
{
    unsigned int ctrl;
    vdi_info_t *vdi = NULL;
    unsigned int count = 0;
    unsigned int fio_data = 0;
    unsigned int fio_ctrl_addr = 0;

    vdi = &s_vdi_info[core_idx];

    switch (vdi->product_code) {
    case WAVE412_CODE:
        fio_data = W4_VPU_FIO_DATA;
        fio_ctrl_addr = W4_VPU_FIO_CTRL_ADDR;
        break;
    case WAVE521_CODE:
    case WAVE521C_CODE:
        fio_data = W5_VPU_FIO_DATA;
        fio_ctrl_addr = W5_VPU_FIO_CTRL_ADDR;
        break;
    default:
        VLOG(ERR, "%s unknow product id %x \n", __func__, vdi->product_code);
        return;
    }

    vdi_write_register(core_idx, fio_data, data);
    ctrl = (addr & 0xffff);
    ctrl |= (1 << 16); /* write operation */
    vdi_write_register(core_idx, fio_ctrl_addr, ctrl);

    if (WAVE521C_CODE == vdi->product_code) {
        count = FIO_TIMEOUT;
        while (count--) {
            ctrl = vdi_read_register(core_idx, fio_ctrl_addr);
            if (ctrl & 0x80000000) {
                break;
            }
        }
    }
}

int vdi_clear_memory(unsigned long core_idx, unsigned int addr, int len, int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
    unsigned long offset;

    int i;
    Uint8 *zero;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 1 && addr >= vdi->vpu_buffer_pool[i].vdb.phys_addr &&
            addr < (vdi->vpu_buffer_pool[i].vdb.phys_addr + vdi->vpu_buffer_pool[i].vdb.size)) {

            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address, endian %d !!!\n", (int)addr, endian);
        return -1;
    }

    zero = (Uint8 *)osal_malloc(len);
    osal_memset((void *)zero, 0x00, len);

    offset = addr - (unsigned long)vdb.phys_addr;
    osal_memcpy_form_cpu_to_device((void *)((unsigned long)vdb.virt_addr + offset), zero, len);

    osal_free(zero);

    return len;
}

void vdi_set_sdram(unsigned long core_idx, unsigned int addr, int len, unsigned char data,
                   int endian)
{
    VLOG(INFO, "[VDI] sdram common in ..coredIdx(%d), len(%d), endian(%d). data %d \n", core_idx,
         len, endian, data);
}

int vdi_write_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len,
                     int endian)
{
    vdi_info_t *vdi;
    vpudrv_buffer_t vdb;
    unsigned long offset;
    int i;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 1 && addr >= vdi->vpu_buffer_pool[i].vdb.phys_addr &&
            addr < (vdi->vpu_buffer_pool[i].vdb.phys_addr + vdi->vpu_buffer_pool[i].vdb.size)) {

            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
        return -1;
    }

    offset = addr - (unsigned long)vdb.phys_addr;
    swap_endian(core_idx, data, len, endian);

    osal_memcpy_form_cpu_to_device((void *)((unsigned long)vdb.virt_addr + offset), data, len);
    return len;
}

int vdi_read_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len,
                    int endian)
{
    vdi_info_t *vdi = NULL;
    vpudrv_buffer_t vdb = {0};
    unsigned long offset = 0;
    int i = 0;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 1 && addr >= vdi->vpu_buffer_pool[i].vdb.phys_addr &&
            addr < (vdi->vpu_buffer_pool[i].vdb.phys_addr + vdi->vpu_buffer_pool[i].vdb.size)) {

            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "address 0x%08x is not mapped address!!!\n", (int)addr);
        return -1;
    }

    offset = addr - (unsigned long)vdb.phys_addr;
    osal_memcpy_form_device_to_cpu(data, (const void *)((unsigned long)vdb.virt_addr + offset),
                                   len);
    swap_endian(core_idx, data, len, endian);
    return len;
}

/**
 *@brief alloc no cache memory
 *@para  core_idx  vpu core id
 *@para  vb  contains the size allocated
 *@return 0  success
 *        -1 failure
 */
int vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi = NULL;
    int i;
    vpudrv_buffer_t vdb = {0};

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    vdb.size = vb->size;
    vdb.virt_addr = (unsigned long)memalign(1024, vdb.size);
    if (!vdb.virt_addr) {
        VLOG(ERR, "[VDI][%s][%d] fail to allocate vdb mem! \n", __func__, __LINE__);
        return -1;
    }

    vdb.phys_addr = (unsigned long)p2ap((paddr_t)vdb.virt_addr);
    vdb.base = vdb.phys_addr;

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 0) {
            vdi->vpu_buffer_pool[i].vdb = vdb;
            vdi->vpu_buffer_pool[i].inuse = 1;
            break;
        }
    }

    if (i >= MAX_VPU_BUFFER_POOL) {
        VLOG(ERR, "[%s][%d] vpu buffer pool is full\n", __func__, __LINE__);
        osal_free((void *)vdb.virt_addr);
        return -1;
    }

    vb->phys_addr = (unsigned long)vdb.phys_addr;
    vb->base = (unsigned long)vdb.base;
    vb->virt_addr = (unsigned long)vdb.virt_addr;

    VLOG(INFO, "[%s][%d] base %llx phy addr=%llx, virt addr=%llx, size=%d, index:%d \n", __func__,
         __LINE__, vb->base, vb->phys_addr, vb->virt_addr, vb->size, i);

    return 0;
}

int vdi_attach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    vpudrv_buffer_t vdb;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    vdb.size = vb->size;
    /* vb phys_addr in kernel */
    vdb.phys_addr = vb->phys_addr;
    /* vb  virt addr in kernel */
    vdb.base = vb->base;
    /* vb virt addr in user space */
    if (!vb->virt_addr)
        vdb.virt_addr = (uint64_t)ap2p((paddr_t)vb->phys_addr);
    else
        vdb.virt_addr = vb->virt_addr;

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 0) {
            vdi->vpu_buffer_pool[i].vdb = vdb;
            vdi->vpu_buffer_pool[i].inuse = 1;
            break;
        }
    }

    if (i >= MAX_VPU_BUFFER_POOL) {
        VLOG(ERR, "[%s][%d] vpu buffer pool is full\n", __func__, __LINE__);
        return -1;
    }

    VLOG(INFO, "[%s][%d] base %llx phy addr=%llx, virt addr=%llx, size=%d index:%d\n", __func__,
         __LINE__, vb->base, vb->phys_addr, vb->virt_addr, vb->size, i);
    return 0;
}

int vdi_dettach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vb || !vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (vb->size == 0)
        return -1;

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 1 &&
            vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr) {
            vdi->vpu_buffer_pool[i].inuse = 0;
            break;
        }
    }

    // vbTemp is from common buffer, but will dettach when dec close, so prompt a warning
    if (i >= MAX_VPU_BUFFER_POOL) {
        VLOG(WARN, "[%s][%d] not found buffer in pool, vb:%p, phys_addr:%llx\n", __func__, __LINE__,
             vb, vb->phys_addr);
        return -1;
    }

    VLOG(INFO, "[%s][%d] base %llx phy addr=%llx, virt addr=%llx, size=%d, index:%d \n", __func__,
         __LINE__, vb->base, vb->phys_addr, vb->virt_addr, vb->size, i);

    return 0;
}

void vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi;
    int i;
    vpudrv_buffer_t vdb;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if (!vb || !vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return;

    if (vb->size == 0)
        return;

    osal_memset(&vdb, 0x00, sizeof(vpudrv_buffer_t));

    for (i = 0; i < MAX_VPU_BUFFER_POOL; i++) {
        if (vdi->vpu_buffer_pool[i].inuse == 1 &&
            vdi->vpu_buffer_pool[i].vdb.phys_addr == vb->phys_addr) {
            vdi->vpu_buffer_pool[i].inuse = 0;
            vdb = vdi->vpu_buffer_pool[i].vdb;
            break;
        }
    }

    if (!vdb.size) {
        VLOG(ERR, "[VDI][%s][%d] invalid buffer to free address = %llx\n", __func__, __LINE__,
             vdb.virt_addr);
        return;
    }

    VLOG(INFO, "[%s][%d] base %llx phy addr=%llx, virt addr=%llx, size=%d, index:%d \n", __func__,
         __LINE__, vb->base, vb->phys_addr, vb->virt_addr, vb->size, i);

    osal_free((void *)vdb.virt_addr);

    osal_memset(vb, 0x0, sizeof(vpu_buffer_t));
}

int vdi_get_sram_memory(unsigned long core_idx, vpu_buffer_t *vb)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    if (!vb || !vdi)
        return -1;

    if (vdi->product_code == CODA980_CODE) {
        vb->phys_addr = VPU_CODA988_SRAM_ADDR;
        vb->size = VPU_CODA988_SRAM_SIZE;
    } else if (vdi->product_code == WAVE412_CODE) {
        vb->phys_addr = VPU_WAVE412_SRAM_ADDR;
        vb->size = VPU_WAVE412_SRAM_SIZE;
    } else if (vdi->product_code == WAVE521C_CODE) {
        vb->phys_addr = VPU_WAVE521_SRAM_ADDR;
        vb->size = VPU_WAVE521_SRAM_SIZE;
    } else {
        vb->phys_addr = 0;
        vb->size = 0;
    }

    VLOG(INFO, "get sram base addr:%llx, total size:%x\n", vb->phys_addr, vb->size);

    return 0;
}

int vdi_set_clock_gate(unsigned long core_idx, int enable)
{
    vdi_info_t *vdi = NULL;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (vdi->product_code == WAVE510_CODE || vdi->product_code == WAVE512_CODE) {
        return 0;
    }

    vdi->clock_state = enable;

    return 0;
}

int vdi_get_clock_gate(unsigned long core_idx)
{
    vdi_info_t *vdi = NULL;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    return vdi->clock_state;
}

int vdi_wait_bus_busy(unsigned long core_idx, int timeout, unsigned int gdi_busy_flag)
{
    vdi_info_t *vdi;
    uint64_t elapse, cur;

    vdi = &s_vdi_info[core_idx];
    elapse = osal_gettime();

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    while (1) {
        if (vdi->product_code == WAVE521C_CODE) {
            if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0x00ff1f3f)
                break;
        } else if (vdi->product_code == WAVE412_CODE) {
            if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0x738)
                break;
        } else if (vdi->product_code == CODA980_CODE) {
            if (vdi_read_register(core_idx, gdi_busy_flag) == 0x77)
                break;
        } else {
            VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
            return -1;
        }

        cur = osal_gettime();
        if ((cur - elapse) > timeout) {
            VLOG(INFO, "[%s] ERROR, read timeout:%d reg offset:%x\n", __func__, timeout,
                 gdi_busy_flag);
            return -1;
        }
    }

    return 0;
}

static Uint32 vdi_get_pc_addr(Uint32 product_code)
{
    Uint32 cur_pc = 0;

    switch (product_code) {
    case CODA980_CODE:
        cur_pc = BIT_CUR_PC;
        break;
    case WAVE412_CODE:
        cur_pc = W4_VCPU_CUR_PC;
        break;
    case WAVE521_CODE:
    case WAVE521C_CODE:
        cur_pc = W5_VCPU_CUR_PC;
        break;
    default:
        VLOG(ERR, "%s unknow product id %x \n", __func__, product_code);
        break;
    }

    return cur_pc;
}

int vdi_wait_vpu_busy(unsigned long core_idx, int timeout, unsigned int addr_bit_busy_flag)
{
    Uint32 cur_pc = 0;
    vdi_info_t *vdi = {0};
    uint64_t elapse, cur;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    elapse = osal_gettime();
    vdi = &s_vdi_info[core_idx];

    cur_pc = vdi_get_pc_addr(vdi->product_code);
    if (cur_pc == 0) {
        VLOG(ERR, "%s unknow product id %x \n", __func__, vdi->product_code);
        return -1;
    }

    while (1) {
        if (vdi->product_code == WAVE521C_CODE) {
            if (vdi_read_register(core_idx, addr_bit_busy_flag) == 0)
                break;
        } else if (vdi->product_code == WAVE412_CODE) {
            Uint32 reg_value = 0;

            if (addr_bit_busy_flag & 0x8000)
                reg_value = vdi_fio_read_register(core_idx, addr_bit_busy_flag);
            else
                reg_value = vdi_read_register(core_idx, addr_bit_busy_flag);

            if (reg_value == 0)
                break;
        } else if (vdi->product_code == CODA980_CODE) {
            if (vdi_read_register(core_idx, addr_bit_busy_flag) == 0)
                break;
        }

        cur = osal_gettime();
        if ((cur - elapse) > timeout) {
            VLOG(ERR, "[%s] ERROR, read timeout:%d reg offset:%x\n", __func__, timeout,
                 addr_bit_busy_flag);

            for (int i = 0; i < 50; i++)
                VLOG(ERR, "[%s] ERROR, read timeout, PC=0x%lx\n", __func__,
                     vdi_read_register(core_idx, cur_pc));

            return -1;
        }
    }

    return 0;
}

int vdi_wait_vcpu_bus_busy(unsigned long core_idx, int timeout, unsigned int gdi_busy_flag)
{
    Uint32 cur_pc;
    vdi_info_t *vdi;
    uint64_t elapse, cur;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];
    elapse = osal_gettime();
    cur_pc = vdi_get_pc_addr(vdi->product_code);

    if (cur_pc == 0) {
        VLOG(ERR, "%s unknow product id %x \n", __func__, vdi->product_code);
        return -1;
    }

    while (1) {
        if (vdi_fio_read_register(core_idx, gdi_busy_flag) == 0x00)
            break;

        cur = osal_gettime();
        if ((cur - elapse) > timeout) {
            VLOG(ERR, "[%s] ERROR, read timeout:%d reg offset:%x PC=0x%lx\n", __func__, timeout,
                 gdi_busy_flag, vdi_read_register(core_idx, cur_pc));
            return -1;
        }
    }
    return 0;
}

int vdi_wait_interrupt_internal(vdi_info_t *vdi, int timeout)
{
    int ret = 0;

    if (vdi->product_code == CODA980_CODE && vpu_coda988_interrupt_signal) {
        if (osErrorTimeout == osEventFlagsWait(vpu_coda988_interrupt_signal, VPU_INTRPT_FLAG,
                                               osFlagsWaitAny, timeout)) {
            VLOG(ERR, "[%s][%d] vpu waiting interrupt timedout\n", __func__, __LINE__);
            return -1;
        }

        ret = vpu_coda988_interrupt_status;
        vpu_coda988_interrupt_status = 0;
    } else if (vdi->product_code == WAVE412_CODE && vpu_wave412_interrupt_signal) {
        if (osErrorTimeout == osEventFlagsWait(vpu_wave412_interrupt_signal, VPU_INTRPT_FLAG,
                                               osFlagsWaitAny, timeout)) {
            VLOG(ERR, "[%s][%d] vpu waiting interrupt timedout\n", __func__, __LINE__);
            return -1;
        }

        ret = vpu_wave412_interrupt_status;
        vpu_wave412_interrupt_status = 0;
    } else if (vdi->product_code == WAVE521C_CODE && vpu_wave521_interrupt_signal) {
        if (osErrorTimeout == osEventFlagsWait(vpu_wave521_interrupt_signal, VPU_INTRPT_FLAG,
                                               osFlagsWaitAny, timeout)) {
            VLOG(ERR, "[%s][%d] vpu waiting interrupt timedout\n", __func__, __LINE__);
            return -1;
        }

        ret = vpu_wave521_interrupt_status;
        vpu_wave521_interrupt_status = 0;
    } else {
        VLOG(ERR,
             "[%s][%d] invalid product code:%d, coda irq signal:%p, wave412 irq signal:%p, "
             "wave521 irq signal:%p\n",
             __func__, __LINE__, vdi->product_code, vpu_coda988_interrupt_signal,
             vpu_wave412_interrupt_signal, vpu_wave521_interrupt_signal);
        return -1;
    }

    return ret;
}

int vdi_wait_interrupt(unsigned long core_idx, int timeout)
{
    vdi_info_t *vdi = &s_vdi_info[core_idx];
    int intr_reason;
    unsigned int addr_bit_int_reason = 0;

    // VDI must implement timeout action in this function for multi-vpu core scheduling efficiency.
    // the setting small value as timeout gives a chance to wait the other vpu core.
#ifdef VPU_HW_INTERRUPT

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    intr_reason = vdi_wait_interrupt_internal(vdi, timeout);

#else
    int int_sts_reg = 0;
    uint64_t elapse, cur;

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (vdi->product_code == WAVE412_CODE) {
        int_sts_reg = W4_VPU_VPU_INT_STS;
        addr_bit_int_reason = W4_VPU_VINT_REASON;
    } else if (vdi->product_code == WAVE521C_CODE) {
        int_sts_reg = W5_VPU_VPU_INT_STS;
        addr_bit_int_reason = W5_VPU_VINT_REASON;
    } else if (vdi->product_code == CODA980_CODE) {
        int_sts_reg = BIT_INT_STS;
        addr_bit_int_reason = BIT_INT_REASON;
    }

    elapse = osal_gettime();

    while (1) {
        if (vdi_read_register(core_idx, int_sts_reg)) {
            if ((intr_reason = vdi_read_register(core_idx, addr_bit_int_reason))) {
                VLOG(INFO, "[VDI-INT] - interrupt:%x \n", intr_reason);
                break;
            }
        }

        cur = osal_gettime();
        if ((endTime - startTime) >= timeout) {
            VLOG(INFO, "[VDI-INT] - NO interrupt, TIMEOUT:%d \n", timeout);
            VLOG(INFO, "int_sts_reg:%x vdi_read_register(core_idx, int_sts_reg):%x\n", int_sts_reg,
                 vdi_read_register(core_idx, int_sts_reg));
            vdi_log(core_idx, 0xff, 1);
            return -1;
        }
    }
#endif
    return intr_reason;
}

static int read_pinfo_buffer(int core_idx, int addr)
{
    int ack;
    int rdata;
#define VDI_LOG_GDI_PINFO_ADDR (0x1068)
#define VDI_LOG_GDI_PINFO_REQ (0x1060)
#define VDI_LOG_GDI_PINFO_ACK (0x1064)
#define VDI_LOG_GDI_PINFO_DATA (0x106c)
    //------------------------------------------
    // read pinfo - indirect read
    // 1. set read addr     (GDI_PINFO_ADDR)
    // 2. send req          (GDI_PINFO_REQ)
    // 3. wait until ack==1 (GDI_PINFO_ACK)
    // 4. read data         (GDI_PINFO_DATA)
    //------------------------------------------
    vdi_write_register(core_idx, VDI_LOG_GDI_PINFO_ADDR, addr);
    vdi_write_register(core_idx, VDI_LOG_GDI_PINFO_REQ, 1);

    ack = 0;

    while (ack == 0) {
        ack = vdi_read_register(core_idx, VDI_LOG_GDI_PINFO_ACK);
    }

    rdata = vdi_read_register(core_idx, VDI_LOG_GDI_PINFO_DATA);

    // VLOG(INFO,"[READ PINFO] ADDR[%x], DATA[%x]", addr, rdata);
    return rdata;
}

enum { VDI_PRODUCT_ID_980, VDI_PRODUCT_ID_960 };

static void printf_gdi_info(int core_idx, int num, int reset)
{
    int i;
    int bus_info_addr;
    int tmp;
    int val;
    int productId = 0;

    val = vdi_read_register(core_idx, VPU_PRODUCT_CODE_REGISTER);

    if ((val & 0xff00) == 0x3200)
        val = 0x3200;

    if (PRODUCT_CODE_W_SERIES(val)) {
        return;
    } else if (PRODUCT_CODE_NOT_W_SERIES(val)) {
        if (val == CODA960_CODE || val == BODA950_CODE)
            productId = VDI_PRODUCT_ID_960;
        else if (val == CODA980_CODE || val == WAVE320_CODE)
            productId = VDI_PRODUCT_ID_980;
    } else {
        VLOG(ERR, "Unknown product id : %08x\n", val);
        return;
    }

    if (productId == VDI_PRODUCT_ID_980)
        VLOG(INFO, "\n**GDI information for GDI_20\n");
    else
        VLOG(INFO, "\n**GDI information for GDI_10\n");

    for (i = 0; i < num; i++) {

#define VDI_LOG_GDI_INFO_CONTROL 0x1400

        if (productId == VDI_PRODUCT_ID_980)
            bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i * (0x20);
        else
            bus_info_addr = VDI_LOG_GDI_INFO_CONTROL + i * 0x14;

        if (reset) {
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);
            bus_info_addr += 4;
            vdi_write_register(core_idx, bus_info_addr, 0x00);

            if (productId == VDI_PRODUCT_ID_980) {
                bus_info_addr += 4;
                vdi_write_register(core_idx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(core_idx, bus_info_addr, 0x00);

                bus_info_addr += 4;
                vdi_write_register(core_idx, bus_info_addr, 0x00);
            }

        } else {
            VLOG(INFO, "index = %02d", i);

            tmp = read_pinfo_buffer(
                core_idx,
                bus_info_addr); // TiledEn<<20 ,GdiFormat<<17,IntlvCbCr,<<16 GdiYuvBufStride
            VLOG(INFO, " control = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " pic_size = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " y-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " cb-top = 0x%08x", tmp);

            bus_info_addr += 4;
            tmp = read_pinfo_buffer(core_idx, bus_info_addr);
            VLOG(INFO, " cr-top = 0x%08x", tmp);

            if (productId == VDI_PRODUCT_ID_980) {
                bus_info_addr += 4;
                tmp = read_pinfo_buffer(core_idx, bus_info_addr);
                VLOG(INFO, " y-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(core_idx, bus_info_addr);
                VLOG(INFO, " cb-bot = 0x%08x", tmp);

                bus_info_addr += 4;
                tmp = read_pinfo_buffer(core_idx, bus_info_addr);
                VLOG(INFO, " cr-bot = 0x%08x", tmp);
            }

            VLOG(INFO, "\n");
        }
    }
}

void vdi_print_vpu_status(unsigned long core_idx) {}

// TODO:adapt to wave412 wave521 coda988
void vdi_make_log(unsigned long core_idx, const char *str, int step)
{
    int val;

    val = VpuReadReg(core_idx, W4_INST_INDEX);
    val &= 0xffff;

    if (step == 1)
        VLOG(INFO, "\n**%s start(%d)\n", str, val);
    else if (step == 2) //
        VLOG(INFO, "\n**%s timeout(%d)\n", str, val);
    else
        VLOG(INFO, "\n**%s end(%d)\n", str, val);
}

// TODO:adapt to wave412 wave521 coda988
void vdi_log(unsigned long core_idx, int cmd, int step)
{
    vdi_info_t *vdi;
    int i;

    // BIT_RUN command
    enum {
        SEQ_INIT = 1,
        SEQ_END = 2,
        PIC_RUN = 3,
        SET_FRAME_BUF = 4,
        ENCODE_HEADER = 5,
        ENC_PARA_SET = 6,
        DEC_PARA_SET = 7,
        DEC_BUF_FLUSH = 8,
        RC_CHANGE_PARAMETER = 9,
        VPU_SLEEP = 10,
        VPU_WAKE = 11,
        ENC_ROI_INIT = 12,
        FIRMWARE_GET = 0xf,
        VPU_RESET = 0x10,
    };

    if (core_idx >= MAX_NUM_VPU_CORE)
        return;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0)
        return;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        switch (cmd) {
        case INIT_VPU:
            vdi_make_log(core_idx, "INIT_VPU", step);
            break;

        case DEC_PIC_HDR: // SET_PARAM for ENC
            vdi_make_log(core_idx, "SET_PARAM(ENC), DEC_PIC_HDR(DEC)", step);
            break;

        case FINI_SEQ:
            vdi_make_log(core_idx, "FINI_SEQ", step);
            break;

        case DEC_PIC: // ENC_PIC for ENC
            vdi_make_log(core_idx, "DEC_PIC, ENC_PIC", step);
            break;

        case SET_FRAMEBUF:
            vdi_make_log(core_idx, "SET_FRAMEBUF", step);
            break;

        case FLUSH_DECODER:
            vdi_make_log(core_idx, "FLUSH_DECODER", step);
            break;

        case GET_FW_VERSION:
            vdi_make_log(core_idx, "GET_FW_VERSION", step);
            break;

        case QUERY_DECODER:
            vdi_make_log(core_idx, "QUERY_DECODER", step);
            break;

        case SLEEP_VPU:
            vdi_make_log(core_idx, "SLEEP_VPU", step);
            break;

        case CREATE_INSTANCE:
            vdi_make_log(core_idx, "CREATE_INSTANCE", step);
            break;

        case RESET_VPU:
            vdi_make_log(core_idx, "RESET_VPU", step);
            break;

        default:
            vdi_make_log(core_idx, "ANY_CMD", step);
            break;
        }
    } else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        switch (cmd) {
        case SEQ_INIT:
            vdi_make_log(core_idx, "SEQ_INIT", step);
            break;

        case SEQ_END:
            vdi_make_log(core_idx, "SEQ_END", step);
            break;

        case PIC_RUN:
            vdi_make_log(core_idx, "PIC_RUN", step);
            break;

        case SET_FRAME_BUF:
            vdi_make_log(core_idx, "SET_FRAME_BUF", step);
            break;

        case ENCODE_HEADER:
            vdi_make_log(core_idx, "ENCODE_HEADER", step);
            break;

        case RC_CHANGE_PARAMETER:
            vdi_make_log(core_idx, "RC_CHANGE_PARAMETER", step);
            break;

        case DEC_BUF_FLUSH:
            vdi_make_log(core_idx, "DEC_BUF_FLUSH", step);
            break;

        case FIRMWARE_GET:
            vdi_make_log(core_idx, "FIRMWARE_GET", step);
            break;

        case VPU_RESET:
            vdi_make_log(core_idx, "VPU_RESET", step);
            break;

        case ENC_PARA_SET:
            vdi_make_log(core_idx, "ENC_PARA_SET", step);
            break;

        case DEC_PARA_SET:
            vdi_make_log(core_idx, "DEC_PARA_SET", step);
            break;

        default:
            vdi_make_log(core_idx, "ANY_CMD", step);
            break;
        }
    } else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return;
    }

    for (i = 0; i < 0x200; i = i + 16) {
        VLOG(INFO, "0x%04xh: 0x%08x 0x%08x 0x%08x 0x%08x\n", i, vdi_read_register(core_idx, i),
             vdi_read_register(core_idx, i + 4), vdi_read_register(core_idx, i + 8),
             vdi_read_register(core_idx, i + 0xc));
    }

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) { // WAVE4xx
        if (cmd == INIT_VPU || cmd == VPU_RESET || cmd == CREATE_INSTANCE) {
            vdi_print_vpu_status(core_idx);
        }

    } else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        // if ((cmd == PIC_RUN && step== 0) || cmd == VPU_RESET)
        if (cmd == VPU_RESET) {
            printf_gdi_info(core_idx, 32, 0);

#define VDI_LOG_MBC_BUSY 0x0440
#define VDI_LOG_MC_BASE 0x0C00
#define VDI_LOG_MC_BUSY 0x0C04
#define VDI_LOG_GDI_BUS_STATUS (0x10F4)
#define VDI_LOG_ROT_SRC_IDX (0x400 + 0x10C)
#define VDI_LOG_ROT_DST_IDX (0x400 + 0x110)

            VLOG(INFO, "MBC_BUSY = %x\n", vdi_read_register(core_idx, VDI_LOG_MBC_BUSY));
            VLOG(INFO, "MC_BUSY = %x\n", vdi_read_register(core_idx, VDI_LOG_MC_BUSY));
            VLOG(INFO, "MC_MB_XY_DONE=(y:%d, x:%d)\n",
                 (vdi_read_register(core_idx, VDI_LOG_MC_BASE) >> 20) & 0x3F,
                 (vdi_read_register(core_idx, VDI_LOG_MC_BASE) >> 26) & 0x3F);
            VLOG(INFO, "GDI_BUS_STATUS = %x\n",
                 vdi_read_register(core_idx, VDI_LOG_GDI_BUS_STATUS));

            VLOG(INFO, "ROT_SRC_IDX = %x\n", vdi_read_register(core_idx, VDI_LOG_ROT_SRC_IDX));
            VLOG(INFO, "ROT_DST_IDX = %x\n", vdi_read_register(core_idx, VDI_LOG_ROT_DST_IDX));

            VLOG(INFO, "P_MC_PIC_INDEX_0 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x200));
            VLOG(INFO, "P_MC_PIC_INDEX_1 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x20c));
            VLOG(INFO, "P_MC_PIC_INDEX_2 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x218));
            VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x230));
            VLOG(INFO, "P_MC_PIC_INDEX_3 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x23C));
            VLOG(INFO, "P_MC_PIC_INDEX_4 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x248));
            VLOG(INFO, "P_MC_PIC_INDEX_5 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x254));
            VLOG(INFO, "P_MC_PIC_INDEX_6 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x260));
            VLOG(INFO, "P_MC_PIC_INDEX_7 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x26C));
            VLOG(INFO, "P_MC_PIC_INDEX_8 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x278));
            VLOG(INFO, "P_MC_PIC_INDEX_9 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x284));
            VLOG(INFO, "P_MC_PIC_INDEX_a = %x\n", vdi_read_register(core_idx, MC_BASE + 0x290));
            VLOG(INFO, "P_MC_PIC_INDEX_b = %x\n", vdi_read_register(core_idx, MC_BASE + 0x29C));
            VLOG(INFO, "P_MC_PIC_INDEX_c = %x\n", vdi_read_register(core_idx, MC_BASE + 0x2A8));
            VLOG(INFO, "P_MC_PIC_INDEX_d = %x\n", vdi_read_register(core_idx, MC_BASE + 0x2B4));

            VLOG(INFO, "P_MC_PICIDX_0 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x028));
            VLOG(INFO, "P_MC_PICIDX_1 = %x\n", vdi_read_register(core_idx, MC_BASE + 0x02C));
        }
    } else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return;
    }
}

static void byte_swap(unsigned char *data, int len)
{
    register Uint8 temp;
    Int32 i;

    for (i = 0; i < len; i += 2) {
        if (i + 1 < len) {
            temp = data[i];
            data[i] = data[i + 1];
            data[i + 1] = temp;
        } else {
            VLOG(ERR, "Error %s : %d : len = %d i = %d\n", __FUNCTION__, __LINE__, len, i);
        }
    }
}

static void word_swap(unsigned char *data, int len)
{
    register Uint16 temp;
    Uint16 *ptr = (Uint16 *)data;
    Int32 i, size = len / sizeof(Uint16);

    for (i = 0; i < size; i += 2) {
        if (i + 1 < size) {
            temp = ptr[i];
            ptr[i] = ptr[i + 1];
            ptr[i + 1] = temp;
        } else {
            VLOG(ERR, "Error %s : %d : len = %d , size =%d i = %d\n", __FUNCTION__, __LINE__, len,
                 size, i);
        }
    }
}

static void dword_swap(unsigned char *data, int len)
{
    register Uint32 temp;
    Uint32 *ptr = (Uint32 *)data;
    Int32 i, size = len / sizeof(Uint32);

    for (i = 0; i < size; i += 2) {
        if (i + 1 < size) {
            temp = ptr[i];
            ptr[i] = ptr[i + 1];
            ptr[i + 1] = temp;
        } else {
            VLOG(ERR, "Error %s : %d : len = %d , size = %d i = %d\n", __FUNCTION__, __LINE__, len,
                 size, i);
        }
    }
}

static void lword_swap(unsigned char *data, int len)
{
    register Uint64 temp;
    Uint64 *ptr = (Uint64 *)data;
    Int32 i, size = len / sizeof(Uint64);

    for (i = 0; i < size; i += 2) {
        if (i + 1 < size) {
            temp = ptr[i];
            ptr[i] = ptr[i + 1];
            ptr[i + 1] = temp;
        } else {
            VLOG(ERR, "Error  %s : %d : len = %d , size =%d i = %d\n", __FUNCTION__, __LINE__, len,
                 size, i);
        }
    }
}

int vdi_get_system_endian(unsigned long core_idx)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        return VDI_128BIT_BUS_SYSTEM_ENDIAN;
    } else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        return VDI_SYSTEM_ENDIAN;
    } else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }
}

int vdi_convert_endian(unsigned long core_idx, unsigned int endian)
{
    vdi_info_t *vdi;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        switch (endian) {
        case VDI_LITTLE_ENDIAN:
            endian = 0x00;
            break;

        case VDI_BIG_ENDIAN:
            endian = 0x0f;
            break;

        case VDI_32BIT_LITTLE_ENDIAN:
            endian = 0x04;
            break;

        case VDI_32BIT_BIG_ENDIAN:
            endian = 0x03;
            break;
        }

        if (endian > 0x0f && vdi->product_code == WAVE521C_CODE) {
            /* WAVE521C PRODUCTS endian, so flip it
            VDI_128BIT_LITTLE_ENDIAN    = 16,
            VDI_128BIT_LE_BYTE_SWAP,
            VDI_128BIT_LE_WORD_SWAP,
            VDI_128BIT_LE_WORD_BYTE_SWAP,
            VDI_128BIT_LE_DWORD_SWAP,
            VDI_128BIT_LE_DWORD_BYTE_SWAP,
            VDI_128BIT_LE_DWORD_WORD_SWAP,
            VDI_128BIT_LE_DWORD_WORD_BYTE_SWAP,
            VDI_128BIT_BE_DWORD_WORD_BYTE_SWAP,
            VDI_128BIT_BE_DWORD_WORD_SWAP,
            VDI_128BIT_BE_DWORD_BYTE_SWAP,
            VDI_128BIT_BE_DWORD_SWAP,
            VDI_128BIT_BE_WORD_BYTE_SWAP,
            VDI_128BIT_BE_WORD_SWAP,
            VDI_128BIT_BE_BYTE_SWAP,
            VDI_128BIT_BIG_ENDIAN        = 31,
            */

            endian = ((endian & 0x0f) - 0x0f) * -1 + 0x10;
        }
    } else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
    } else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    return (endian & 0x0f);
}

static Uint32 convert_endian_coda9_to_wave4(Uint32 endian)
{
    Uint32 converted_endian = endian;

    switch (endian) {
    case VDI_LITTLE_ENDIAN:
        converted_endian = 0;
        break;

    case VDI_BIG_ENDIAN:
        converted_endian = 7;
        break;

    case VDI_32BIT_LITTLE_ENDIAN:
        converted_endian = 4;
        break;

    case VDI_32BIT_BIG_ENDIAN:
        converted_endian = 3;
        break;
    }

    return converted_endian;
}

int swap_endian(unsigned long core_idx, unsigned char *data, int len, int endian)
{
    vdi_info_t *vdi;
    int changes;
    int sys_endian;
    BOOL byteChange, wordChange, dwordChange, lwordChange;

    if (core_idx >= MAX_NUM_VPU_CORE)
        return -1;

    vdi = &s_vdi_info[core_idx];

    if (!vdi || vdi->vpu_fd == -1 || vdi->vpu_fd == 0x00)
        return -1;

    //   VLOG(ERR, "ctm>>> %s:%d: product code (0x%x)\n", __FUNCTION__, __LINE__,
    //   vdi->product_code);
    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
        sys_endian = VDI_128BIT_BUS_SYSTEM_ENDIAN;
    } else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        sys_endian = VDI_SYSTEM_ENDIAN;
    } else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    endian = vdi_convert_endian(core_idx, endian);
    sys_endian = vdi_convert_endian(core_idx, sys_endian);

    if (endian == sys_endian)
        return 0;

    if (PRODUCT_CODE_W_SERIES(vdi->product_code)) {
    } else if (PRODUCT_CODE_NOT_W_SERIES(vdi->product_code)) {
        endian = convert_endian_coda9_to_wave4(endian);
        sys_endian = convert_endian_coda9_to_wave4(sys_endian);
    } else {
        VLOG(ERR, "Unknown product id : %08x\n", vdi->product_code);
        return -1;
    }

    changes = endian ^ sys_endian;
    byteChange = changes & 0x01;
    wordChange = ((changes & 0x02) == 0x02);
    dwordChange = ((changes & 0x04) == 0x04);
    lwordChange = ((changes & 0x08) == 0x08);

    if (byteChange)
        byte_swap(data, len);

    if (wordChange)
        word_swap(data, len);

    if (dwordChange)
        dword_swap(data, len);

    if (lwordChange)
        lword_swap(data, len);

    return 1;
}
