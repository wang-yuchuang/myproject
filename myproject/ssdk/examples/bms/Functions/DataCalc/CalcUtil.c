/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CalcUtil.c
** @Founder      : ZQD808
** @Date         : 2026.03.11
** @Function     : 计算工具包模块
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "CalcUtil.h"

/*=============================================================================================
** @Global variable definition
**===========================================================================================*/

/*=============================================================================================
** @Static function declaration
**===========================================================================================*/

/*=============================================================================================
** @Interface function definition
**===========================================================================================*/
/*=============================================================================================
** @Name      : u8 SearchHigFromUpTabs8(s8 data, const s8 *array, u8 len)
** @Input     : data:查找的数据 array:数组基地址 len:数组长度
** @Output    : 查找到数据的下标
** @Function  : 二份法查找s8升序数组
** @The notes : 取高端值,注意len为1时
**===========================================================================================*/
u8 SearchHigFromUpTabs8(s8 data, const s8 *array, u8 len)
{
    u8 left = 0;
    u8 right = len - 1;
    u8 mid = 0;

    if(data <= array[0] || len == 1)
    {
        return(0);
    }

    if(data >= array[len - 1])
    {
        return(len - 1);
    }

    while(left <= right)
    {
        mid = (u8)((u16)left + (u16)right) / 2;

        if(data == array[mid])
        {
            return(mid);
        }
        else if(data < array[mid])
        {
            right = mid - 1;	
        }
        else
        {
            left = mid + 1;	
        }
    }

    return(right);
}

/*=============================================================================================
** @Name      : u8 SearchHigFromUpTabFaru8(u8 data, u8 *array, u8 len)
** @Input     : data:查找的数据 array:数组基地址 len:数组长度
** @Output    : 查找到数据的下标
** @Function  : 二份法查找 u8升序数组
** @The notes : 取高端值,注意len为1时
**===========================================================================================*/
u8 SearchHigFromUpTabFaru8(u8 data, u8 *array, u8 len)
{
    u8 left = 0;
    u8 right = len - 1;
    u8 mid = 0;

    if(data <= array[0] || len == 1)
    {
        return(0);
    }

    if(data >= array[len - 1])
    {
        return(len - 1);
    }

    while(left <= right)
    {
        mid = (u8)((u16)left + (u16)right) / 2;

        if(data == array[mid])
        {
            return(mid);
        }
        else if(data < array[mid])
        {
            right = mid - 1;	
        }
        else
        {
            left = mid + 1;	
        }
    }

    return(right);
}

/*=============================================================================================
** @Name      : u8 SearchHigFromUpTabs16(s16 value, const s16 * array, u8 len)
** @Input     : value:查找的数据 array:数组基地址 len:数组长度
** @Output    : 得到的下标
** @Function  : 从s16升序数组中查找数据对应的下标
** @The notes : 取高端值
**===========================================================================================*/
u8 SearchHigFromUpTabs16(s16 value, const s16 * array, u8 len)
{
    u8 temp = 0;
    
    for(temp = 0; temp < len; temp++)
	{
		if(value <= *(array + temp))
		{
			return(temp);
		}
	}
	
	return(len - 1);   
}

/*=============================================================================================
** @Name      : u16 SearchLowFromUpTabu16(u16 data, const u16* array, u16 len)
** @Input     : data:查找的数据 array:数组基地址 len:数组长度
** @Output    : 查找到数据的下标
** @Function  : 二份法从U16升序数组查找对应下标
** @The notes : 取低端值
**===========================================================================================*/
u16 SearchLowFromUpTabu16(u16 data, const u16 *array, u16 len)
{
    u16 left = 0;
    u16 right = 0;
    u16 mid = 0;
    
    right = len - 1;
    
    while(left <= right)
    {
        mid = (left + right) / 2;
        if(data == array[mid])
        {
            return(mid);
        }
        else if(data > array[mid])
        {
            left = mid + 1;  
        }
        else
        {
            right = mid - 1;
        }
    }
    
    return(right);
}

/*=============================================================================================
** @Name      : void AverageCompose(u16 *array, u8 len, u8 *path)
** @Input     : array:数组基地址 len:数组长度 path：电池组组号基址
** @Output    : void
** @Function  : 对电池组按电流值大小进行排序
** @The notes :
**===========================================================================================*/
void AverageCompose(u16 *array, u16 len, u8 *path)
{
    u16 index0 = 0;
    u16 index1 = 0;
    u16 temp = 0;
    u8 temp1 = 0;
    
    for(index0 = 0; index0 < len; index0++)
    {
        path[index0] = index0;
    }
   
    for(index0 = 0; index0 < len; index0++)
    {
        for(index1 = index0; index1 < len; index1++)
        {
            if(array[index0] < array[index1])
            {
                temp = array[index0];
                array[index0] = array[index1];
                array[index1] = temp; 
                
                temp1 = path[index0];
                path[index0] = path[index1];
                path[index1] = temp1;
            }
        }
    }
}

/*=============================================================================================
** @Name      : u8 CalcDataSetBitNum(u32 value)
** @Input     : value:输入值
** @Output    : 输入值为1的位的个数
** @Function  : 计算输入值为1的位的个数
** @The notes :
**===========================================================================================*/
u8 CalcDataSetBitNum(u32 value)
{
    u8 num = 0;

    while(0 != value)
    {
        if(value & 0x01)
        {
            num++;
        }
        
        value >>= 1;    
    } 
    
    return(num);
}

/*=============================================================================================
** @Static function definition
**===========================================================================================*/



