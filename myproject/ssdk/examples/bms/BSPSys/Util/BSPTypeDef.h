/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : BSPTypeDef.h
** @Founder      : ZQD808
** @Date         : 2025.11.11
** @Function     : BSP通用数据类型定义模块
** @Instructions :
**===========================================================================================*/
#ifndef _BSPTYPEDEF_H
#define _BSPTYPEDEF_H

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include <stdint.h>

/*=============================================================================================
** @Macro definition
**===========================================================================================*/

/*=============================================================================================
** @Data type definition
**===========================================================================================*/
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;
typedef signed char		s8;
typedef short			s16;
typedef int			s32;
typedef long long		s64;

typedef u8  uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;
typedef s8  int8_t;
typedef s16 int16_t;
typedef s32 int32_t;
typedef s64 int64_t;


typedef u8      BOOLEAN;
typedef u8      INT8U;
typedef s8      INT8S;
typedef u16     INT16U;
typedef s16     INT16S;
typedef u32     INT32U;
typedef s32     INT32S;

typedef u8     UINT8;
typedef s8     SINT8;
typedef u16    UINT16;
typedef s16    SINT16;
typedef u32    UINT32;
typedef s32    SINT32;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

/*
#ifndef FAR_NULL
#define FAR_NULL ((void * far)0)
#endif
*/
/*
typedef signed char int8_t;
typedef short int   int16_t;
typedef long int    int32_t;
typedef long long   int64_t;

typedef unsigned char		uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned long int   uint32_t;
typedef unsigned long long  uint64_t;
*/
typedef enum                    /*功能参数常用状态返回*/
{
    ok = 0,                     /*执行正确*/
    failed = 1,                 /*失败*/
    errP0 = 2,                  /*参数0错误*/
    errP1 = 3,                  /*参数1错误*/
    errP2 = 4,                  /*参数2错误*/
    NOUSE = 5                   /*调用无效*/
}e_FunStatus;

#define IsEqual(a,b)            ((a) == (b))                                /*a等于b返回1 否则返回0*/
#define IsNotEqual(a,b)         ((a)!=(b))                                  /*a不等于b返回1 否则返回0*/
#define BitSet(target,index)    ((target) |= (u32)0x01<<(index))            /*将数target的第index位设置为1*/
#define BitClr(target,index)    ((target) &= (~((u32)0x01<<(index))))       /*将数target的第index位清0*/
#define BitGet(target,index)    (((target) & (u32)0x01<<(index))>>(index))  /*获得数据target的第index位的值*/
#define BitSet16(target,index)  ((target) |= (u16)0x01<<(index))
#define BitClr16(target,index)  ((target) &= (~((u16)0x01<<(index))))
#define BitGet16(target,index)  (((target) & (u16)0x01<<(index))>>(index))
#define BitSet8(target,index)   ((target) |= (u8)0x01<<(index))             /*将数target的第index位设置为1*/
#define BitClr8(target,index)   ((target) &= (~((u8)0x01<<(index))))        /*将数target的第index位清0*/
#define BitGet8(target,index)   (((target) & (u8)0x01<<(index))>>(index))   /*获得数据target的第index位的值*/

#define IsInside(x1,x,x2)       (((x1)<=(x))&&((x)<=(x2)))                  /*判断数x是否在x1与x2之间*/
#define ABS(target,index)       ((target) > (index)?((target) - (index)):((index) - (target)))/*两数相减正数差值*/

#define BSPDelay(Count)         {volatile u16 i = Count; while(i--);}       /*BSP延时函数*/

#define UPPER_BYTE(x)           ((x >> 8) & 0xff)
#define LOWER_BYTE(x)       	(x & 0xff)
#define UPPER_WORD(x)       	((x >> 16) & 0xffff)
#define LOWER_WORD(x)       	(x & 0xffff)
#define UPPER_NIBBLE(x)     	((x >> 4) & 0x0f)
#define LOWER_NIBBLE(x)     	(x & 0x0f)


//输出绝对值
#define OAV(a)        (((a) < 0) ? (-(a)) : (a))

#ifdef DEBUG
  #define ALIGN_ASSERT(ptr, align) do { if (((uintptr_t)(ptr) & ((align) - 1)) != 0) { __builtin_trap(); } } while(0)
#else
  #define ALIGN_ASSERT(ptr, align) ((void)0)
#endif

#endif
