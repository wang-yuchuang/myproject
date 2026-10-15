/*
* disp_mux_reg.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 10/21/2019 BI create this file
*/

#ifndef __DISP_MUX__
#define __DISP_MUX__

#define DISP_MUX_BASE APB_DISP_MUX_BASE

#define REG(x)  (x)

#define MUX_RA000               REG(0x0)
#define MUX_RA004               REG(0x4)
#define MUX_SHIFT00_RA004          0
#define MUX_MASK00_RA004           0x7 << MUX_SHIFT00_RA004

#define MUX_RA008                REG(0x8)
#define MUX_SHIFT00_RA008          0
#define MUX_MASK00_RA008           0x7 << MUX_SHIFT00_RA008

#define MUX_RA00C           REG(0xc)
#define MUX_RA010           REG(0x10)
#define MUX_RA014           REG(0x14)
#define MUX_RA018           REG(0x18)
#define MUX_SHIFT00_RA00C        0
#define MUX_MASK00_RA00C         0x3 << MUX_SHIFT00_RA00C

#define MUX_RA01C           REG(0x1c)
#define MUX_SHIFT00_RA01C       0
#define MUX_MASK00_RA01C        0x7 << MUX_SHIFT00_RA01C

#define MUX_RA020                 REG(0x20)
#define MUX_SHIFT05_RA020       5
#define MUX_MASK05_RA020        0x1 << MUX_SHIFT05_RA020
#define MUX_SHIFT04_RA020           4
#define MUX_MASK04_RA020            0x1 << MUX_SHIFT04_RA020
#define MUX_SHIFT03_RA020           3
#define MUX_MASK03_RA020            0x1 << MUX_SHIFT03_RA020
#define MUX_SHIFT02_RA020           2
#define MUX_MASK02_RA020            0x1 << MUX_SHIFT02_RA020
#define MUX_SHIFT01_RA020           1
#define MUX_MASK01_RA020            0x1 << MUX_SHIFT01_RA020
#define MUX_SHIFT00_RA020           0
#define MUX_MASK00_RA020            0x1 << MUX_SHIFT00_RA020

#define MUX_RA030             REG(0x30)
#define MUX_RA038             REG(0x38)
#define MUX_SHIFT14_RA030       14
#define MUX_MASK14_RA030        0x3F << MUX_SHIFT14_RA030
#define MUX_SHIFT08_RA030       8
#define MUX_MASK08_RA030        0x3F << MUX_SHIFT08_RA030
#define MUX_SHIFT07_RA030  7
#define MUX_MASK07_RA030   0x1 << MUX_SHIFT07_RA030
#define MUX_SHIFT06_RA030   6
#define MUX_MASK06_RA030    0x1 << MUX_SHIFT06_RA030
#define MUX_SHIFT05_RA030 5
#define MUX_MASK05_RA030  0x1 << MUX_SHIFT05_RA030
#define MUX_SHIFT04_RA030 4
#define MUX_MASK04_RA030  0x1 << MUX_SHIFT04_RA030
#define MUX_SHIFT03_RA030   3
#define MUX_MASK03_RA030    0x1 << MUX_SHIFT03_RA030
#define MUX_SHIFT02_RA030   2
#define MUX_MASK02_RA030    0x1 << MUX_SHIFT02_RA030
#define MUX_SHIFT00_RA030    0
#define MUX_MASK00_RA030     0x3 << MUX_SHIFT00_RA030

#define MUX_RA034             REG(0x34)
#define MUX_RA03C             REG(0x3c)
#define MUX_SHIFT13_RA034     13
#define MUX_MASK13_RA034      0xFFFF << MUX_SHIFT13_RA034
#define MUX_SHIFT12_RA034         12
#define MUX_MASK12_RA034          0x1 << MUX_SHIFT12_RA034
#define MUX_SHIFT11_RA034         11
#define MUX_MASK11_RA034          0x1 << MUX_SHIFT11_RA034
#define MUX_SHIFT09_RA034       9
#define MUX_MASK09_RA034        0x3 << MUX_SHIFT09_RA034
#define MUX_SHIFT08_RA034   8
#define MUX_MASK08_RA034    0x1 << MUX_SHIFT08_RA034
#define MUX_SHIFT07_RA034    7
#define MUX_MASK07_RA034     0x1 << MUX_SHIFT07_RA034
#define MUX_SHIFT00_RA034  0
#define MUX_MASK00_RA034   0x7F << MUX_SHIFT00_RA034

#define MUX_RA040                      REG(0x40)
#define MUX_SHIFT02_RA040            2
#define MUX_MASK02_RA040             0x1 << MUX_SHIFT02_RA040
#define MUX_SHIFT01_RA040            1
#define MUX_MASK01_RA040             0x1 << MUX_SHIFT01_RA040
#define MUX_SHIFT00_RA040            0
#define MUX_MASK00_RA040             0x1 << MUX_SHIFT00_RA040

#define MUX_RA050                REG(0x50)
#define MUX_RA054                REG(0x54)
#define MUX_RA058                REG(0x58)
#define MUX_RA05c                REG(0x5c)
#define MUX_SHIFT00_RA050       0
#define MUX_MASK00_RA050        0x7 << MUX_SHIFT00_RA050

#define MUX_RA060              REG(0x60)
#define MUX_RA068              REG(0x68)
#define MUX_RA070              REG(0x70)
#define MUX_SHIFT00_RA060  0
#define MUX_MASK00_RA060   0x7F << MUX_SHIFT00_RA060

/*This is a special reg.*/
#define MUX_RA088            REG(0x88)
#define MUX_SHIFT02_RA088               2
#define MUX_MASK02_RA088                0x1 << MUX_SHIFT02_RA088
#define MUX_SHIFT01_RA088    1
#define MUX_MASK01_RA088     0x1 << MUX_SHIFT01_RA088
#define MUX_SHIFT00_RA088    0
#define MUX_MASK00_RA088     0x1 << MUX_SHIFT00_RA088

#define LOCK_JMP 0x4
#define LOCK_COUNT 10
#define MUX_RA100(i)       (REG(0x100) + LOCK_JMP * i)
#define MUX_SHIFT00_RA100     0
#define MUX_MASK00_RA100      0x1 << MUX_SHIFT00_RA100

#define MUX_RA200                  REG(0x200)
/*Need ASIC Supply detail information*/
#endif // __DISP_MUX__
