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
* 011, 01/31/2023 BI create this file
*/

#ifndef __DISP_MUX__
#define __DISP_MUX__

#define DISP_MUX_BASE APB_DISP_MUX_BASE

#define REG(x)  (x)

#define MUX_RA000              (REG(0x0000) + 0x100 * 0)
#define MUX_RA100              (REG(0x0000) + 0x100 * 1)
#define MUX_RA200              (REG(0x0000) + 0x100 * 2)
#define MUX_RA300              (REG(0x0000) + 0x100 * 3)
#define MUX_RA400              (REG(0x0000) + 0x100 * 4)
#define MUX_SHIFT00_RA000            0
#define MUX_MASK00_RA000             0x7 << MUX_SHIFT00_RA000
#define MUX_SHIFT08_RA000              8
#define MUX_MASK08_RA000               0x1 << MUX_SHIFT08_RA000

#define MUX_RA700                 REG(0x0700)
#define MUX_SHIFT00_RA700                0
#define MUX_MASK00_RA700                 0x1 << MUX_SHIFT00_RA700
#define MUX_SHIFT01_RA700     1
#define MUX_MASK01_RA700      1 << MUX_SHIFT01_RA700
#define MUX_SHIFT02_RA700     2
#define MUX_MASK02_RA700      1 << MUX_SHIFT02_RA700

#define MUX_RA800                    REG(0x0800)
#define MUX_RA900                    REG(0x0900)
#define MUX_RAA00                    REG(0x0A00)
#define MUX_RAB00                    REG(0x0B00)
#define MUX_SHIFT00_RA800                0
#define MUX_MASK00_RA800                 0x3 << MUX_SHIFT00_RA800
#define MUX_SHIFT02_RA800                 2
#define MUX_MASK02_RA800                  0x1 << MUX_SHIFT02_RA800
#define MUX_SHIFT04_RA800   4
#define MUX_MASK04_RA800    0x3 << MUX_SHIFT04_RA800
#define MUX_SHIFT07_RA800         7
#define MUX_MASK07_RA800          0x1 << MUX_SHIFT07_RA800
#define MUX_SHIFT08_RA800      8
#define MUX_MASK08_RA800       0x1 << MUX_SHIFT08_RA800
#define MUX_SHIFT09_RA800    9
#define MUX_MASK09_RA800     0x1 << MUX_SHIFT09_RA800
#define MUX_SHIFT10_RA800    10
#define MUX_MASK10_RA800     0x1 << MUX_SHIFT10_RA800
#define MUX_SHIFT11_RA800  11
#define MUX_MASK11_RA800   0x1 << MUX_SHIFT11_RA800
#define MUX_SHIFT12_RA800           12
#define MUX_MASK12_RA800            0x1 << MUX_SHIFT12_RA800
#define MUX_SHIFT16_RA800           16
#define MUX_MASK16_RA800            0x1 << MUX_SHIFT16_RA800

#define MUX_RB000                REG(0x1000)
#define MUX_RB100                REG(0x1100)
#define MUX_SHIFT24_RB000 24
#define MUX_MASK24_RB000  0x3 << MUX_SHIFT24_RB000
#define MUX_SHIFT26_RB000  26
#define MUX_MASK26_RB000   0x3 << MUX_SHIFT26_RB000
#define MUX_SHIFT28_RB000 28
#define MUX_MASK28_RB000  0x1 << MUX_SHIFT28_RB000

#define MUX_RB004              REG(0x1004)
#define MUX_RB104              REG(0x1104)
#define MUX_SHIFT00_RB004              0
#define MUX_MASK00_RB004               0x3 << MUX_SHIFT00_RB004
#define MUX_SHIFT02_RB004             2
#define MUX_MASK02_RB004              0x1 << MUX_SHIFT02_RB004
#define MUX_SHIFT03_RB004             3
#define MUX_MASK03_RB004              0x1 << MUX_SHIFT03_RB004
#define MUX_SHIFT04_RB004          4
#define MUX_MASK04_RB004           0x1 << MUX_SHIFT04_RB004
#define MUX_SHIFT05_RB004           5
#define MUX_MASK05_RB004            0x1 << MUX_SHIFT05_RB004
#define MUX_SHIFT06_RB004             6
#define MUX_MASK06_RB004              0x1 << MUX_SHIFT06_RB004
#define MUX_SHIFT07_RB004            7
#define MUX_MASK07_RB004             0x1 << MUX_SHIFT07_RB004
#define MUX_SHIFT08_RB004                 8
#define MUX_MASK08_RB004                  0x3F << MUX_SHIFT08_RB004
#define MUX_SHIFT14_RB004                 14
#define MUX_MASK14_RB004                  0x3F << MUX_SHIFT14_RB004
#define MUX_SHIFT20_RB004                  20
#define MUX_MASK20_RB004                   0x7 << MUX_SHIFT20_RB004
#define MUX_SHIFT24_RB004        24
#define MUX_MASK24_RB004         0x1C << MUX_SHIFT24_RB004

#define MUX_RB008              REG(0x1008)
#define MUX_RB108              REG(0x1108)
#define MUX_SHIFT00_RB008            0
#define MUX_MASK00_RB008             0xA << MUX_SHIFT00_RB008
#define MUX_SHIFT07_RB008              7
#define MUX_MASK07_RB008               0x1 << MUX_SHIFT07_RB008
#define MUX_SHIFT08_RB008             8
#define MUX_MASK08_RB008              0x1 << MUX_SHIFT08_RB008
#define MUX_SHIFT09_RB008        9
#define MUX_MASK09_RB008         0x3 << MUX_SHIFT09_RB008
#define MUX_SHIFT11_RB008                   11
#define MUX_MASK11_RB008                    0x1 << MUX_SHIFT11_RB008
#define MUX_SHIFT12_RB008                   12
#define MUX_MASK12_RB008                    0x1 << MUX_SHIFT12_RB008
#define MUX_SHIFT16_RB008               16
#define MUX_MASK16_RB008                0xFFFF << MUX_SHIFT16_RB008

#define MUX_RB00C               REG(0x100C)
#define MUX_RB10C               REG(0x110C)
#define MUX_SHIFT00_RB00C            0
#define MUX_MASK00_RB00C             0xFFFF << MUX_SHIFT00_RB00C

#define MUX_RB800                       REG(0x1800)
#define MUX_SHIFT00_RB800             0
#define MUX_MASK00_RB800              0x1 << MUX_SHIFT00_RB800
#define MUX_SHIFT01_RB800             1
#define MUX_MASK01_RB800              0x1 << MUX_SHIFT01_RB800
#define MUX_SHIFT02_RB800             2
#define MUX_MASK02_RB800              0x1 << MUX_SHIFT02_RB800

#define MUX_RB900                 REG(0x1900)

#define MUX_RC000                      REG(0x2000)
#define MUX_RC100                      REG(0x2100)
#define MUX_RC200                      REG(0x2200)
#define MUX_SHIFT00_RC000                 0
#define MUX_MASK00_RC000                  0x1 << MUX_SHIFT00_RC000
#define MUX_SHIFT01_RC000                 1
#define MUX_MASK01_RC000                  0x1 << MUX_SHIFT01_RC000
#define MUX_SHIFT02_RC000           2
#define MUX_MASK02_RC000            0x1 << MUX_SHIFT02_RC000
#define MUX_SHIFT03_RC000           3
#define MUX_MASK03_RC000            0x1 << MUX_SHIFT03_RC000

#define MUX_RC004                   REG(0x2004)
#define MUX_RC104                   REG(0x2104)
#define MUX_RC204                   REG(0x2204)
#define MUX_SHIFT00_RC004              0
#define MUX_MASK00_RC004               0x1 << MUX_SHIFT00_RC004
#define MUX_SHIFT01_RC004             1
#define MUX_MASK01_RC004              0x1 << MUX_SHIFT01_RC004
#define MUX_SHIFT02_RC004         2
#define MUX_MASK02_RC004          0x1 << MUX_SHIFT02_RC004
#define MUX_SHIFT03_RC004           3
#define MUX_MASK03_RC004            0x1 << MUX_SHIFT03_RC004
#define MUX_SHIFT04_RC004           4
#define MUX_MASK04_RC004            0x1 << MUX_SHIFT04_RC004
#define MUX_SHIFT08_RC004          8
#define MUX_MASK08_RC004           0x1 << MUX_SHIFT08_RC004
#define MUX_SHIFT09_RC004          9
#define MUX_MASK09_RC004           0x1 << MUX_SHIFT09_RC004
#define MUX_SHIFT10_RC004             10
#define MUX_MASK10_RC004              0x1 << MUX_SHIFT10_RC004
#define MUX_SHIFT11_RC004        11
#define MUX_MASK11_RC004         0x1 << MUX_SHIFT11_RC004
#define MUX_SHIFT18_RC004         18
#define MUX_MASK18_RC004          0x1 << MUX_SHIFT18_RC004
#define MUX_SHIFT19_RC004           19
#define MUX_MASK19_RC004            0x1 << MUX_SHIFT19_RC004
#define MUX_SHIFT20_RC004           20
#define MUX_MASK20_RC004            0x1 << MUX_SHIFT20_RC004
#define MUX_SHIFT24_RC004          24
#define MUX_MASK24_RC004           0x1 << MUX_SHIFT24_RC004
#define MUX_SHIFT25_RC004          25
#define MUX_MASK25_RC004           0x1 << MUX_SHIFT25_RC004
#define MUX_SHIFT26_RC004             26
#define MUX_MASK26_RC004              0x1 << MUX_SHIFT26_RC004
#define MUX_SHIFT27_RC004        27
#define MUX_MASK27_RC004         0x1 << MUX_SHIFT27_RC004
#define MUX_SHIFT28_RC004       28
#define MUX_MASK28_RC004        0x1 << MUX_SHIFT28_RC004
#define MUX_SHIFT29_RC004       29
#define MUX_MASK29_RC004        0x1 << MUX_SHIFT29_RC004

#define MUX_RC008           REG(0x2008)
#define MUX_RC108           REG(0x2108)
#define MUX_RC208           REG(0x2208)
#define MUX_RC00C           REG(0x200C)
#define MUX_RC10C           REG(0x210C)
#define MUX_RC20C           REG(0x220C)
#define MUX_SHIFT00_RC008                0
#define MUX_MASK00_RC008                 0xFFFF << MUX_SHIFT00_RC008
#define MUX_SHIFT16_RC008                16
#define MUX_MASK16_RC008                 0xFFFF << MUX_SHIFT16_RC008

#define MUX_RC010                  REG(0x2010)
#define MUX_RC110                  REG(0x2110)
#define MUX_RC210                  REG(0x2210)
#define MUX_SHIFT00_RC010        0
#define MUX_MASK00_RC010         0x3 << MUX_SHIFT00_RC010
#define MUX_SHIFT02_RC010        2
#define MUX_MASK02_RC010         0x3 << MUX_SHIFT02_RC010
#define MUX_SHIFT04_RC010        4
#define MUX_MASK04_RC010         0x3 << MUX_SHIFT04_RC010
#define MUX_SHIFT06_RC010        6
#define MUX_MASK06_RC010         0x3 << MUX_SHIFT06_RC010

#define MUX_RF000(i)                (REG(0x5000) + 4 * i)
#define MUX_RF100                    REG(0x5100)

#endif
