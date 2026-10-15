/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : EepM24512.c
** @Founder      : ZQD808
** @Date         : 2025.11.20
** @Function     : 外部EEPROM驱动模块
** @Instructions : 
**===========================================================================================*/
#include "M2514.h"
#include <compiler.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include "driver/i2c.h"
#include <debug.h>
#include "ParaInterface.h"
#include "DEVTIMER.h"

#include "CLI_type.h"
i2c_adap_dev_t* i2c_handle;
static u8 sExtEepromReady = 0;          //外部EEPROM就绪状态 1就绪
/*=============================================================================================
** @Name      : u8 EXTEep24M512Init(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 外部EEPROM-M24512初始化
** @The notes :
**===========================================================================================*/
u8 EXTEep24M512Init(void)
{
    u32 flag = 0;
    i2c_handle = i2c_get_adap_dev(EepRpmI2C);
    if (NULL == i2c_handle){
        ssdk_printf(SSDK_WARNING, "%s: i2c2_get_adap_dev error!!!\r\n", __func__);
        return FALSE;
    }
	/*读取用户校验标志是否正常*/
	if(TRUE == DEVEXTEEPReadLong(Eep_Ver_Addr, &flag, 1))
	{
		/*校验标志正确*/
		if(Eep_Ver_Flag == flag)
		{
			/*外部EEPROM有效*/
			sExtEepromReady = 1;
		}
		/*校验标志错误*/
		else
		{
			/*重写标志*/
			flag = Eep_Ver_Flag;
			DEVEXTEEPWriteLong(Eep_Ver_Addr, &flag, 1);

			/*外部EEPROM无效*/
			sExtEepromReady = 0;
		}
	}
	else
	{
		/*外部EEPROM无效*/
		sExtEepromReady = 0;
	}
    return TRUE;
}
/*=============================================================================================
** @Name      : int DEVEXTEEPWriteByte(u16 addr, u8 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要写入的数据  length:要写入数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 向外部EEPROM指定地址写入length个字节
** @The notes : 函数已支持跨页写入
**===========================================================================================*/
int DEVEXTEEPWriteByte(u16 addr, u8 *buffer, int length)
{
    int ret = -1;
    int remaining = length;
    u16 current_addr = addr;
    u8 *current_buffer = buffer;
    /*输入参数无效*/
	if((0 == length) || (((u32)addr + length) > Eep_Max_Addr) || (buffer == NULL))
	{
		return(FALSE);
	}
    if(i2c_handle == NULL)
    {
        printf("DEVEXTEEPWriteByte i2c_handle == NULL\r\n");
		return(FALSE);
    }
    /* 分段写入数据，处理I2C限制和页边界 */
    while(remaining > 0)
    {
        u16 page_start = (current_addr / Eep_Page_Size) * Eep_Page_Size;
        u16 page_end = page_start + Eep_Page_Size - 1;

         /* 计算当前页内剩余空间 */
        u16 page_remaining = page_end - current_addr + 1;

        /* 确定本次写入块大小（取三者的最小值） */
        int chunk_size = remaining;

        /* 1. 不能超过I2C驱动限制 */
        if (chunk_size > MAX_I2C_WRITE_SIZE) {
            chunk_size = MAX_I2C_WRITE_SIZE;
        }
            /* 2. 不能超过页边界 */
        if (chunk_size > page_remaining) {
            chunk_size = page_remaining;
        }
         ret = i2c_write_reg16(i2c_handle, M24512_Devic_Addr, current_addr, 
                              current_buffer, chunk_size);
        if(ret < 0){
            printf("\ni2c-%d write byte dev addr=%d, reg addr = %d,error code is %d\r\n", 5, M24512_Devic_Addr, addr, ret);
            return FALSE;
        }
        /* 更新指针和剩余长度 */
        current_addr += chunk_size;
        current_buffer += chunk_size;
        remaining -= chunk_size;
    }
    return TRUE;
}
/*=============================================================================================
** @Name      : int DEVEXTEEPWriteWord(u16 addr, u16 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要写入的数据  length:要写入数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 向外部EEPROM指定地址写入length个字节
** @The notes : 尽量不要跨页写(一页128Byte)
**===========================================================================================*/
int DEVEXTEEPWriteWord(u16 addr, u16 *buffer, int length)
{
    int ret = -1;
    //u8 buffer1[128] = {0x00};
    /*输入参数无效*/
    if((addr % 2) != 0)
    {
        printf("\nDEVEXTEEPWriteWord Invalid addr= %d\r\n" ,addr);
		return(FALSE);
    }
	if((0 == length) || (((u32)addr + length * 2) > Eep_Max_Addr) || (buffer == NULL))
	{
        printf("\nInvalid parameters: addr=0x%04X, length=%d words\r\n", addr, length);
		return(FALSE);
	}
    if(i2c_handle == NULL)
    {
        printf("DEVEXTEEPWriteWord i2c_handle == NULL\r\n");
		return(FALSE);
    }
    /* 逐块处理，每个u16占2字节 */
    u16 current_addr = addr;
    const u16 *current_buffer = buffer;
    int words_remaining = length;

    while (words_remaining > 0)
    {
        /* 计算当前页内剩余空间（字节） */
        u16 page_start = (current_addr / Eep_Page_Size) * Eep_Page_Size;
        u16 page_end = page_start + Eep_Page_Size - 1;
        u16 bytes_to_page_end = page_end - current_addr + 1;

        /* 计算本次可以处理的u16数量 */
        int max_words_by_page = bytes_to_page_end / 2;  // 页内最多能放的u16数量
        int max_words_by_i2c = MAX_I2C_WRITE_SIZE / 2;  // I2C单次最多能写的u16数量

        /* 取最小值作为本次处理的u16数量 */
        int words_to_process = words_remaining;
        if (words_to_process > max_words_by_page) {
                words_to_process = max_words_by_page;
        }
        if (words_to_process > max_words_by_i2c) {
                words_to_process = max_words_by_i2c;
        }

        /* 将u16转换为字节并写入 */
        u8 byte_buffer[64];  // 最大32个u16 = 64字节
        int byte_count = 0;

        for (int i = 0; i < words_to_process; i++) {
            u16 value = current_buffer[i];
            byte_buffer[byte_count++] = (value & 0xFF);      // 低字节
            byte_buffer[byte_count++] = ((value >> 8) & 0xFF); // 高字节
        }

        /* 写入当前块 */
        ret = i2c_write_reg16(i2c_handle, M24512_Devic_Addr, 
                            current_addr, byte_buffer, byte_count);
        if (ret < 0) {
            printf("\ni2c-%d write word error at addr 0x%04X, error=%d\r\n", 
                    EepRpmI2C, current_addr, ret);
            return FALSE;
        }
        /* 更新指针和剩余数量 */
        current_addr += byte_count;
        current_buffer += words_to_process;
        words_remaining -= words_to_process;
    }
    return TRUE;
}
/*=============================================================================================
** @Name      : int DEVEXTEEPWriteLong(u16 addr, u32 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要写入的数据  length:要写入数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 向外部EEPROM指定地址写入length个字节
** @The notes : 尽量不要跨页写(一页128Byte)
**===========================================================================================*/
int DEVEXTEEPWriteLong(u16 addr, u32 *buffer, int length)
{
    int ret = -1;
    /*输入参数无效*/
    if((addr % 4) != 0)
    {
        printf("\nDEVEXTEEPWriteLong Invalid addr= %d\r\n" ,addr);
		return(FALSE);
    }
	if((0 == length) || (((u32)addr + length * 4) > Eep_Max_Addr) || (buffer == NULL))
	{
        printf("\nInvalid parameters: addr=0x%04X, length=%d longs\r\n", addr, length);
		return(FALSE);
	}
    if(i2c_handle == NULL)
    {
        printf("DEVEXTEEPWriteLong i2c_handle == NULL\r\n");
		return(FALSE);
    }
     /* 计算总字节数 */
    int total_bytes = length * 4;
    /* 逐块处理，每块最大16个u32（64字节） */
    u16 current_addr = addr;
    const u32 *current_buffer = buffer;
    int longs_remaining = length;

     while (longs_remaining > 0)
     {
        /* 计算当前页内剩余空间（以字节为单位） */
        u16 page_start = (current_addr / Eep_Page_Size) * Eep_Page_Size;
        u16 page_end = page_start + Eep_Page_Size - 1;
        u16 bytes_to_page_end = page_end - current_addr + 1;

        /* 计算本次可以处理的u32数量（受页剩余空间和I2C限制） */
        int max_longs_by_page = bytes_to_page_end / 4;  // 页内最多能放的u32数量
        int max_longs_by_i2c = MAX_I2C_WRITE_SIZE / 4;  // I2C单次最多能写的u32数量

        /* 取最小值作为本次处理的u32数量 */
        int longs_to_process = longs_remaining;
        if (longs_to_process > max_longs_by_page) {
            longs_to_process = max_longs_by_page;
        }
        if (longs_to_process > max_longs_by_i2c) {
            longs_to_process = max_longs_by_i2c;
        }

        /* 将u32转换为字节 */
        u8 byte_buffer[64];  // 最大16个u32 = 64字节
        int byte_count = 0;

        for (int i = 0; i < longs_to_process; i++) {
            u32 value = current_buffer[i];
            byte_buffer[byte_count++] = (value & 0xFF);
            byte_buffer[byte_count++] = ((value >> 8) & 0xFF);
            byte_buffer[byte_count++] = ((value >> 16) & 0xFF);
            byte_buffer[byte_count++] = ((value >> 24) & 0xFF);
        }

        /* 写入当前块 */
        ret = i2c_write_reg16(i2c_handle, M24512_Devic_Addr, 
                              current_addr, byte_buffer, byte_count);
        if (ret < 0) {
            printf("\ni2c-%d write long error at addr 0x%04X, error=%d\r\n", 
                   EepRpmI2C, current_addr, ret);
            return FALSE;
        }
        /* 更新指针和剩余数量 */
        current_addr += byte_count;
        current_buffer += longs_to_process;
        longs_remaining -= longs_to_process;

     }

    return TRUE;
}

/*=============================================================================================
** @Name      : int DEVEXTEEPReadByte(u16 addr, u8 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要读取的数据  length:读取数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 从外部EEPROM指定地址读出length个字节
** @The notes :
**===========================================================================================*/
int DEVEXTEEPReadByte(u16 addr, u8 *buffer, int length)
{
    int ret = -1;
    /*输入参数无效*/
	if((0 == length) || (((u32)addr + length) > Eep_Max_Addr))
	{
        printf("DEVEXTEEPReadByte para error\r\n");
		return(FALSE);
	}
    if(i2c_handle == NULL)
    {
        printf("DEVEXTEEPReadByte i2c_handle == NULL\r\n");
		return(FALSE);
    }
    /* 如果读取长度不超过64字节，直接读取 */
    if (length <= MAX_I2C_READ_SIZE)
    {
        ret = i2c_read_reg16(i2c_handle, M24512_Devic_Addr, addr, buffer, length);
        if(ret < 0)
        {
            printf("\ni2c-%d read byte reg=0x%04X error, error code=%d\r\n", 
                   EepRpmI2C, addr, ret);
            return FALSE;
        }
        return TRUE;
    }

    /* 超过64字节，分段读取 */
    int remaining = length;
    u16 current_addr = addr;
    u8 *current_buffer = buffer;

    while(remaining > 0)
    {
            /* 计算本次读取的字节数（最大64字节） */
        int chunk_size = (remaining > MAX_I2C_READ_SIZE)  ? MAX_I2C_READ_SIZE : remaining;
         /* 读取当前块 */
        ret = i2c_read_reg16(i2c_handle, M24512_Devic_Addr, 
                             current_addr, current_buffer, chunk_size);
        if(ret < 0)
        {
            printf("\ni2c-%d read byte error at addr 0x%04X, error=%d\r\n", 
                   EepRpmI2C, current_addr, ret);
            return FALSE;
        }
        /* 更新地址和缓冲区指针 */
        current_addr += chunk_size;
        current_buffer += chunk_size;
        remaining -= chunk_size;
    }

    return TRUE;
}

/*=============================================================================================
** @Name      : int DEVEXTEEPReadWord(u16 addr, u16 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要读取的数据  length:读取数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 从外部EEPROM指定地址读出length个字节
** @The notes :
**===========================================================================================*/
int DEVEXTEEPReadWord(u16 addr, u16 *buffer, int length)
{
    int ret = -1;
    /*输入参数无效*/
	if((0 == length) || (((u32)addr + length * 2) > Eep_Max_Addr) || (buffer == NULL))
	{
        printf("DEVEXTEEPReadWord para error\r\n");
		return(FALSE);
	}
    if(i2c_handle == NULL)
    {
        printf("DEVEXTEEPReadWord i2c_handle == NULL\r\n");
		return(FALSE);
    }
    /* 如果数据量小，直接读取 */
    if (length <= 32) {  // 32 words * 2 = 64 bytes
        u8 byte_buffer[64];
        int bytes_to_read = length * 2;

        ret = i2c_read_reg16(i2c_handle, M24512_Devic_Addr, addr, byte_buffer, bytes_to_read);
        if (ret < 0) {
            printf("\ni2c-%d read word error at 0x%04X\r\n", EepRpmI2C, addr);
            return FALSE;
        }
        /* 转换为u16数组 */
        for (int i = 0; i < length; i++) {
            buffer[i] = byte_buffer[2 * i] | (byte_buffer[2 * i + 1] << 8);
        }
        
        return TRUE;
    }

        /* 大数据量，分段读取 */
    int max_words_per_chunk = MAX_I2C_READ_SIZE / 2;  // 32 words
    int words_read = 0;
    while (words_read < length)
    {
        /* 计算本次读取的u16数量 */
        int words_to_read = length - words_read;
        if (words_to_read > max_words_per_chunk) {
            words_to_read = max_words_per_chunk;
        }
        
        /* 计算当前地址 */
        u16 current_addr = addr + words_read * 2;  // 每个u16占2字节
        
        /* 读取当前块 */
        u8 byte_buffer[64];
        int bytes_to_read = words_to_read * 2;
        
        ret = i2c_read_reg16(i2c_handle, M24512_Devic_Addr, 
                             current_addr, byte_buffer, bytes_to_read);
        if (ret < 0) {
            printf("\ni2c-%d read word error at 0x%04X\r\n", EepRpmI2C, current_addr);
            return FALSE;
        }
        /* 转换为u16并存储 */
        for (int i = 0; i < words_to_read; i++) {
            buffer[words_read + i] = byte_buffer[2 * i] | (byte_buffer[2 * i + 1] << 8);
        }

        words_read += words_to_read;
    }

    return TRUE;
}
/*=============================================================================================
** @Name      : int DEVEXTEEPReadLong(u16 addr, u32 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要读取的数据  length:读取数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 从外部EEPROM指定地址读出length个字节
** @The notes :
**===========================================================================================*/
int DEVEXTEEPReadLong(u16 addr, u32 *buffer, int length)
{
    int ret = -1;
    /*输入参数无效*/
	if((0 == length) || (((u32)addr + length * 4) > Eep_Max_Addr) || buffer == NULL)
	{
        printf("DEVEXTEEPReadLong para error\r\n");
		return(FALSE);
	}
    if(i2c_handle == NULL)
    {
        printf("DEVEXTEEPReadWord i2c_handle == NULL\r\n");
		return(FALSE);
    }
    /* 如果数据量小，直接读取 */
    if (length <= 16) {  // 16 longs * 4 = 64 bytes
        u8 byte_buffer[64];
        int bytes_to_read = length * 4;
        
        ret = i2c_read_reg16(i2c_handle, M24512_Devic_Addr, addr, byte_buffer, bytes_to_read);
        if (ret < 0) {
            printf("\ni2c-%d read long error at 0x%04X\r\n", EepRpmI2C, addr);
            return FALSE;
        }
        
        /* 转换为u32数组 */
        for (int i = 0; i < length; i++) {
            buffer[i] = (u32)(byte_buffer[4 * i] | 
                             (byte_buffer[4 * i + 1] << 8) | 
                             (byte_buffer[4 * i + 2] << 16) | 
                             (byte_buffer[4 * i + 3] << 24));
        }
        
        return TRUE;
    }
        /* 大数据量，分段读取 */
    int max_longs_per_chunk = MAX_I2C_READ_SIZE / 4;  // 16 longs
    int longs_read = 0;
    while (longs_read < length)
    {
                /* 计算本次读取的u32数量 */
        int longs_to_read = length - longs_read;
        if (longs_to_read > max_longs_per_chunk) {
            longs_to_read = max_longs_per_chunk;
        }

          /* 计算当前地址 */
        u16 current_addr = addr + longs_read * 4;  // 每个u32占4字节

          /* 读取当前块 */
        u8 byte_buffer[64];
        int bytes_to_read = longs_to_read * 4;
        ret = i2c_read_reg16(i2c_handle, M24512_Devic_Addr, 
                             current_addr, byte_buffer, bytes_to_read);
        if (ret < 0) {
            printf("\ni2c-%d read long error at 0x%04X\r\n", EepRpmI2C, current_addr);
            return FALSE;
        }
        
        /* 转换为u32并存储 */
        for (int i = 0; i < longs_to_read; i++) {
            buffer[longs_read + i] = (u32)(byte_buffer[4 * i] | 
                                          (byte_buffer[4 * i + 1] << 8) | 
                                          (byte_buffer[4 * i + 2] << 16) | 
                                          (byte_buffer[4 * i + 3] << 24));
        }
        longs_read += longs_to_read;
    }

    return TRUE;
}

/*=============================================================================================
** @Name      : u8 DEVEXTEEPGetReadyFlag(void)
** @Input     : void
** @Output    : 0:未就绪 1:就绪
** @Function  : 获取外部EEPROm就绪状态标志
** @The notes : 与外部EEPROM芯片通讯状态
**===========================================================================================*/
u8 DEVEXTEEPGetReadyFlag(void)
{
    return(sExtEepromReady);
}
/*=============================================================================================
** @Name      : void DEVEXTEEPCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 外部EEPROM检测任务
** @The notes : 4s
**===========================================================================================*/
void DEVEXTEEPCheckTask(void)
{
	static u8 sTime = 0;

    //外部EEPROM未就绪
    if((0 == sExtEepromReady) && (osKernelGetTickCount() >= 10000))
    {
    	//只重复50次
    	if(sTime < 50)
    	{
			//重新初始化
			EXTEep24M512Init();
    		sTime++;
    	}
    }
    else
    {
    	sTime = 0;
    }
}
#if(1)
//如下为了验证读写功能是否正常，正式程序可以屏蔽
static int eeprom_test(int argc, char *argv[])
{
    u8 i = 0,j = 0,k = 0;
    int ret = 0;
    u8 Buffer1[128] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
    u8 Buffer2[128] = {0x00,0x00};
    u16 Buffer3[128] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
    u16 Buffer4[128] = {0x00,0x00};
    u32 Buffer5[128] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
    u32 Buffer6[128] = {0x00,0x00};
    if (argc == 0) {
        printf("spi test para error\n");
        return CLI_FALSE;
    }
    if (!strcmp(argv[0], "WB")) {
        j = strtoul(argv[1], NULL, 0);
        k = strtoul(argv[2], NULL, 0);
        for(i = 0; i < k; i++)
        {
            Buffer1[i] = j+i;
        }
        DEVEXTEEPWriteByte((u16)100,Buffer1,k);
    }
    else if (!strcmp(argv[0], "RB")) {
         j = strtoul(argv[1], NULL, 0);
         DEVEXTEEPReadByte(100,Buffer2,j);
         for(i = 0; i < j; i++) 
         {
            printf("buffer2[%d] = %d\r\n",i,Buffer2[i]);
         }

    }
    else if (!strcmp(argv[0], "WW")) {
        j = strtoul(argv[1], NULL, 0);
        k = strtoul(argv[2], NULL, 0);
        for(i = 0; i < k; i++)
        {
            Buffer3[i] = j+i;
        }
        DEVEXTEEPWriteWord(300, Buffer3, k);
    }
    else if (!strcmp(argv[0], "RW")) {
        j = strtoul(argv[1], NULL, 0);
        DEVEXTEEPReadWord(300,Buffer4,j);
        for(i = 0; i < j; i++)
        {
            printf("Buffer4[%d] = %d\r\n",i,Buffer4[i]);
        }
    }
    else if (!strcmp(argv[0], "WL")) {
        j = strtoul(argv[1], NULL, 0);
        k = strtoul(argv[2], NULL, 0);
        for(i = 0; i < k; i++)
        {
            Buffer5[i] = j+i;
        }
        DEVEXTEEPWriteLong(1000, Buffer5, k);
    }
    else if (!strcmp(argv[0], "RL")) {
        j = strtoul(argv[1], NULL, 0);
        DEVEXTEEPReadLong(1000,Buffer6,j);
        for(i = 0; i < j; i++)
        {
            printf("Buffer6[%d] = %d\r\n",i,Buffer6[i]);
        }
        
    }
    else if (!strcmp(argv[0], "INIT")) {
        EXTEep24M512Init();
    }

    else if (!strcmp(argv[0], "PARA")) {
        ParaStoreParaInfoInit();
    }
    return CLI_FALSE;
}
CLI_CMD("eeprom_test", "eeprom", eeprom_test);
#endif



