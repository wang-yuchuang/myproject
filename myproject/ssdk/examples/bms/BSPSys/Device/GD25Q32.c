/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : GD25Q32.c
** @Founder      : ZQD808
** @Date         : 2025.11.25
** @Function     : 外部NOR FLash GD25Q32驱动模块
** @Instructions : 独立硬件OSPI通讯
**===========================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <string.h>
#include <arch.h>
#include <board.h>
#include <cmsis_os2.h>
#include <compiler.h>
#include <driver/clk.h>
#include "driver/spi_nor.h"

#include "CLI.h"
#include "irq_num.h"
#include "regs_base.h"
#include "driver/kunlun/spi_nor/cdns_ospi_drv.h"
#include "board.h"
#include "GD25Q32.h"

/* 外部Flash驱动仅在非XIP(emmc/OCRAM)形态编译:OSPI2为XIP执行介质时,重配OSPI2/擦写Flash会与R5取指互斥 */
#if !defined(CONFIG_XIP_MODE)
static u8 sExtFlashReady = 0;   					//外部Flash就绪状态 1就绪

static struct spi_nor flash_fd;
extern clk_node_t g_ckgen_ip_ospi2;
static struct cospi_pdata cospi2;
static struct spi_nor_host spi_nor_host2;
/* 前置声明：初始化路径在定义前调用该静态校验函数。 */
static u8 EXTFlashCheckIcReadyState(void);
/*=============================================================================================
** @Name      : u8 NORFlashGD25Init(void)
** @Input     : void
** @Output    : void
** @Function  : 外部Flash初始化
** @The notes : 检测外部Flash芯片是否就绪
**===========================================================================================*/
u8 NORFlashGD25Init(void)
{
    u8 state = 0;
    u8 data[4] = {0};
    struct cospi_config host_config = {
        .id = 2,
        .apb_base = DEVICE_BASE(OSPI2),
        .ahb_base = OSPI2_BASE,
    };

    host_config.ref_clk = clk_get_rate(&g_ckgen_ip_ospi2);

    cospi_host_init(&spi_nor_host2, &cospi2, &host_config);

    struct spi_nor_config config = {
        .id = 2,
        .cs = 0,
        .baudrate = 50000000,
        .xfer_mode = SPI_NOR_XFER_POLLING_MODE,
        .async_mode = false,
        .sw_rst = 1,
    };
    if(spi_nor_init(&flash_fd, &spi_nor_host2, &config)){
        printf("spi_nor_init error\n");
        return FALSE;
    }
    //检测外部Flash状态
    state = EXTFlashCheckIcReadyState();
    //外部Flash校验未就绪则重写校验码
    if(0 == state)
    {
        //擦除校验扇区(末扇区);擦除失败则放弃本轮重写(未擦除区编程无法可靠恢复标志,2026-09-18审查修复)
        if(FALSE == DEVEXTFLASHEraseSector(EXT_FLA_CRC_ADDR))
        {
            printf("EXTFlash erase crc sector fail\n");
            sExtFlashReady = 0;
            return sExtFlashReady;
        }
        //取得标志
        data[0] = EXT_FLA_CRC_FLAG & 0xff;
        data[1] = (EXT_FLA_CRC_FLAG >> 8) & 0xff;
        data[2] = (EXT_FLA_CRC_FLAG >> 16) & 0xff;
        data[3] = (EXT_FLA_CRC_FLAG >> 24) & 0xff;

        //重写校验标志
        DEVEXTFLASHWriteByte(EXT_FLA_CRC_ADDR, data, 4);

        //重新校验正确则就绪
        if(1 == EXTFlashCheckIcReadyState())
        {
            //外部Flash初始化就绪
            sExtFlashReady = 1;
        }
    }
    //外部Flash校验正确则就绪
    else if(1 == state)
    {
        //外部Flash初始化就绪
        sExtFlashReady = 1;
    }
    //外部Flash校验错误
    else
    {
        //外部Flash初始化故障
        sExtFlashReady = 0;
    }
    return sExtFlashReady;
}
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHWriteByte(u32 addr, u8 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:写入Flash数据块 size:数据长度[1,256]
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 将8位字节数据写入到外部Flash中
** @The notes :NOR FLSH至少必须写入4字节，此函数写入非4的整数倍大小数据会有问题
**===========================================================================================*/
u8 DEVEXTFLASHWriteByte(u32 addr, u8 *pData, u32 size)
{
    u8 result = 0;
    //地址超过最大地址容量或写超过允许长度
    if(((addr + size) > EXT_FLA_ADDR_SIZE)
        || (size > EXT_FLA_WR_MAX_SIZE)
        || (size == 0))
    {
        return(FALSE);
    }
    result = spi_nor_write(&flash_fd, addr, (const uint8_t *)pData, size);
    if(result == 0)
        return (TRUE);
    else
        return (FALSE);
}
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHWriteWord(u32 addr, u16 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:写入Flash数据块 size:数据长度[1,128]
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 将16位单字数据写入到外部Flash中
** @The notes : FLSH至少必须写入4字节，此函数写入非2的整数倍大小数据会有问题
**===========================================================================================*/
u8 DEVEXTFLASHWriteWord(u32 addr, u16 *pData, u32 size)
{
    u8 result = 0;
    u8 buffer[256] = {0x00}; //写数据缓冲区
    //地址超过最大地址容量或写超过允许长度
    if(((addr + (size * 2)) > EXT_FLA_ADDR_SIZE)
        || ((size * 2) > EXT_FLA_WR_MAX_SIZE)
        || (size == 0))
    {
        return(FALSE);
    }
    for(int i = 0; i < size; i++)
    {
        buffer[2 * i] = (pData[i] & 0xff);
        buffer[2 * i + 1] = ((pData[i] >> 8) & 0xff);
    }
    result = spi_nor_write(&flash_fd, addr, (const uint8_t *)buffer, size * 2);
    if(result == 0)
        return (TRUE);
    else
        return (FALSE);
}
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHWriteLong(u32 addr, u32 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:写入Flash数据块 size:数据长度[1,64]
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 将32位双字数据写入到外部Flash中
** @The notes :
**===========================================================================================*/
u8 DEVEXTFLASHWriteLong(u32 addr, u32 *pData, u32 size)
{
    u8 result = 0;
    u8 buffer[256] = {0x00}; //写数据缓冲区
    //地址超过最大地址容量或写超过允许长度
    if(((addr + (size * 4)) > EXT_FLA_ADDR_SIZE)
        || ((size * 4) > EXT_FLA_WR_MAX_SIZE)
        || (size == 0))
    {
        return(FALSE);
    }
    for(int i = 0; i < size; i++)
    {
        buffer[4 * i] = (pData[i] & 0xff);
        buffer[4 * i + 1] = ((pData[i] >> 8) & 0xff);
        buffer[4 * i + 2] = ((pData[i] >> 16) & 0xff);
        buffer[4 * i + 3] = ((pData[i] >> 24) & 0xff);
    }
    result = spi_nor_write(&flash_fd, addr, (const uint8_t *)buffer, size * 4);
    if(result == 0)
        return (TRUE);
    else
        return (FALSE);
}
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHReadByte(u32 addr, u8 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:读出的数据块指针 size:数据长度(最大一块32k)
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从外部Flash指定地址读取任意长度的8位字节数据
** @The notes : 读出的数据是若干个8位数
**===========================================================================================*/
u8 DEVEXTFLASHReadByte(u32 addr, u8 *pData, u32 size)
{
    u8 result = 0;
    //地址超过最大地址容量或读超过一块长度(防止耗时过长)
    if(((addr + size) > EXT_FLA_ADDR_SIZE)
       || (size > EXT_FLA_RO_MAX_SIZE)
       || (size == 0))
    {
       return(FALSE);
    } 
    result = spi_nor_read(&flash_fd, addr, pData, size);
    if(result == 0)
      return (TRUE);
    else
      return (FALSE);
}
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHReadWord(u32 addr, u16 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:读出的数据块指针 size:数据长度(最大一块32k)
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从外部Flash指定地址读取任意长度的16位单字数据
** @The notes : 读出的数据是若干个16位数
**===========================================================================================*/
u8 DEVEXTFLASHReadWord(u32 addr, u16 *pData, u32 size)
{
    u8 result = 0;
    u8 buffer[1024] = {0x00};  //读缓冲区
    //地址超过最大地址容量或读超过一块长度(防止耗时过长)
    if(((addr + (size * 2)) > EXT_FLA_ADDR_SIZE)
    || ((size * 2) > EXT_FLA_RO_MAX_SIZE)
    || (size == 0))
    {
        return(FALSE);
    }
    result = spi_nor_read(&flash_fd, addr, buffer, size * 2);
    if(result == 0)
    {
        for(int i = 0; i < size; i++)
        {
            pData[i] = buffer[2 * i] | (buffer[2 * i + 1] << 8);
        }
        return (TRUE);
    }
    else
        return (FALSE);
}
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHReadLong(u32 addr, u32 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:读出的数据块指针 size:数据长度(最大一块32k)
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从外部Flash指定地址读取任意长度的16位单字数据
** @The notes : 读出的数据是若干个16位数
**===========================================================================================*/
u8 DEVEXTFLASHReadLong(u32 addr, u32 *pData, u32 size)
{
    u8 result = 0;
    u8 buffer[1024] = {0x00};  //读缓冲区
    //地址超过最大地址容量或读超过一块长度(防止耗时过长)
    if(((addr + (size * 4)) > EXT_FLA_ADDR_SIZE)
    || ((size * 4) > EXT_FLA_RO_MAX_SIZE)
    || (size == 0))
    {
        return(FALSE);
    }
    result = spi_nor_read(&flash_fd, addr, buffer, size * 4);
    if(result == 0)
    {
        for(int i = 0; i < size; i++)
        {
            pData[i] = (u32)(buffer[4 * i] | (buffer[4 * i + 1] << 8) | (buffer[4 * i + 2] << 16) | (buffer[4 * i + 3] << 24));
        }
        return (TRUE);
    }
    else
        return (FALSE);
}
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHEraseSector(u32 addr)
** @Input     : addr:Flash地址(0-0x3FFFFFF,必须是4k倍率)
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 外部Flash擦除1扇区Flash(1页)
** @The notes : 擦除指定地址起4K
**===========================================================================================*/
u8 DEVEXTFLASHEraseSector(u32 addr)
{
    u8 result = 0;
    //地址超过最大地址容量或不是1块地址倍率
    if(((addr + EXT_FLA_PAGE_SIZE) > EXT_FLA_ADDR_SIZE)
    || (0 != (addr % EXT_FLA_PAGE_SIZE)))
    {
        return(FALSE);
    }
    result = spi_nor_erase(&flash_fd, addr, 0x1000);
    if(result == 0)
        return (TRUE);
    else
        return (FALSE);
}
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHGetReadyFlag(void)
** @Input     : void
** @Output    : 0:未就绪 1:就绪
** @Function  : 获取外部Flash就绪状态标志
** @The notes : 与外部Flash芯片通讯状态
**===========================================================================================*/
u8 DEVEXTFLASHGetReadyFlag(void)
{
    return(sExtFlashReady);
}
/*=============================================================================================
** @Name      : static u8 EXTFlashCheckIcReadyState(void)
** @Input     : void
** @Output    : 就绪状态 0:未就绪 1:就绪 2:故障
** @Function  : 检测外部Flash就绪状态
** @The notes : 通过读写校验标志判断
**===========================================================================================*/
static u8 EXTFlashCheckIcReadyState(void)
{
    u32 data = 0;
    //nor flash只要初始化成功说明读IDok,这里无需再读ID
    /*读取Flash用户参数校验是否正常*/
    if(TRUE == DEVEXTFLASHReadLong(EXT_FLA_CRC_ADDR, &data,1))
    {
        /*检测用户参数校验是否正常*/
    	if(EXT_FLA_CRC_FLAG == data)
    	{
            return(1);
    	}
    	else
    	{
            return(0);
    	}
    }
    else
    {
        return(2);
    }
}
/*=============================================================================================
** @Name      : void EXTFlashCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 外部Flash检测任务
** @The notes : 1s
**===========================================================================================*/
void EXTFlashCheckTask(void)
{
    //上电8s后检测:未就绪则重新初始化
	if(osKernelGetTickCount() >= 8000)
	{
		if(0 == sExtFlashReady)
		{
			//重新初始化
			NORFlashGD25Init();
		}
	}
}

#if(0)//如下为了验证读写功能是否正常，调试使用
static int norflash_test(int argc, char *argv[])
{
    u8 Buffer1[2] = {0x00,0x00};
    u8 Buffer2[2] = {0x00,0x00};
    u16 Buffer3[2] = {0x00,0x00};
    u16 Buffer4[2] = {0x00,0x00};
    u32 Buffer5[2] = {0x00,0x00};
    u32 Buffer6[2] = {0x00,0x00};
    if (argc == 0) {
        printf("norflash test para error\n");
        return CLI_FALSE;
    }
    if (!strcmp(argv[0], "WB")) {
        Buffer1[0] = strtoul(argv[1], NULL, 0);
        DEVEXTFLASHWriteByte(0, Buffer1, 1);
    }
    else if (!strcmp(argv[0], "RB")) {
        DEVEXTFLASHReadByte(0,Buffer2,1);
        printf("read byte = %x\r\n",Buffer2[0]);
    }
    else if (!strcmp(argv[0], "WW")) {
        Buffer3[0] = strtoul(argv[1], NULL, 0);
        DEVEXTFLASHWriteWord(10, Buffer3, 1);
    }
    else if (!strcmp(argv[0], "RW")) {
        DEVEXTFLASHReadWord(10,Buffer4,1);
        printf("read byte = %x\r\n",Buffer4[0]);
    }
    else if (!strcmp(argv[0], "WL")) {
        Buffer5[0] = strtoul(argv[1], NULL, 0);
        DEVEXTFLASHWriteLong(20, Buffer5, 1);
    }
    else if (!strcmp(argv[0], "RL")) {
        DEVEXTFLASHReadLong(20,Buffer6,1);
        printf("read byte = %x\r\n",Buffer6[0]);
    }
    else if (!strcmp(argv[0], "INIT")) {
        NORFlashGD25Init();
    }
    else if (!strcmp(argv[0], "ERASE")) {
        DEVEXTFLASHEraseSector(0);
    }
    
    return CLI_FALSE;
}
CLI_CMD("norflash_test", "norflash", norflash_test);
#endif

#endif





