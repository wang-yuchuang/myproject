/*
* __regs_ap_dc.h
*
* Copyright (c) 2018-2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description: dc register defs file.
*
* Revision History:
* -----------------
* 011, 12/25/2018 BI create this file
*/
#ifndef __DC_REG_H__
#define __DC_REG_H__

/* DC registers (RMW mode) definition */

#define REG(x)                           (x)
/* CTRL */
#define DC_RA000                          REG(0x0)
#define DC_SHIFT31_RA000             31
#define DC_MASK31_RA000              1UL << DC_SHIFT31_RA000
#define DC_SHIFT03_RA000  3
#define DC_MASK03_RA000   1UL << DC_SHIFT03_RA000
#define DC_SHIFT02_RA000   2
#define DC_MASK02_RA000    1UL << DC_SHIFT02_RA000
#define DC_SHIFT01_RA000            1
#define DC_MASK01_RA000             0x1 << DC_SHIFT01_RA000
#define DC_SHIFT00_RA000            0
#define DC_MASK00_RA000             1UL << DC_SHIFT00_RA000


/* FLC */
#define DC_RA004                      REG(0x4)
#define DC_SHIFT03_RA004                 3
#define DC_MASK03_RA004                  1UL << DC_SHIFT03_RA004
#define DC_SHIFT02_RA004                  2
#define DC_MASK02_RA004                   1UL << DC_SHIFT02_RA004
#define DC_SHIFT01_RA004                    1
#define DC_MASK01_RA004                     1UL << DC_SHIFT01_RA004
#define DC_SHIFT00_RA004                   0
#define DC_MASK00_RA004                    1 << DC_SHIFT00_RA004

#define DC_RA008                    REG(0x8)
#define DC_SHIFT00_RA008            0
#define DC_MASK00_RA008             1UL << DC_SHIFT00_RA008

#define DC_RA010                     REG(0x10)
#define DC_SHIFT04_RA010         4
#define DC_MASK04_RA010          0xF << DC_SHIFT04_RA010
#define DC_SHIFT00_RA010          0
#define DC_MASK00_RA010           0x1 << DC_SHIFT00_RA010

#define DC_RA020                      REG(0x20)
#define DC_RA024                    REG(0x24)
#define DC_INIT_DEF_MASK                 0xFFFFFFF

#define DC_SHIFT00_RA02X                       0
#define DC_MASK00_RA02X                        1UL << DC_SHIFT00_RA02X

#define DC_SHIFT01_RA02X                        1
#define DC_MASK01_RA02X                         1UL << DC_SHIFT01_RA02X

#define DC_SHIFT02_RA02X                        2
#define DC_MASK02_RA02X                         1UL << DC_SHIFT02_RA02X

#define DC_SHIFT03_RA02X                   3
#define DC_MASK03_RA02X                    1UL << DC_SHIFT03_RA02X

#define DC_SHIFT04_RA02X                   4
#define DC_MASK04_RA02X                    1UL << DC_SHIFT04_RA02X

#define DC_SHIFT05_RA02X              5
#define DC_MASK05_RA02X               1UL << DC_SHIFT05_RA02X

#define DC_SHIFT06_RA02X                6
#define DC_MASK06_RA02X                 1UL << DC_SHIFT06_RA02X

#define DC_SHIFT07_RA02X                  7
#define DC_MASK07_RA02X                   1UL << DC_SHIFT07_RA02X

#define DC_SHIFT08_RA02X            8
#define DC_MASK08_RA02X             0xFFFFF << DC_SHIFT08_RA02X

#define DC_RA100                   REG(0x100)
#define DC_RA120                   REG(0x120)
#define DC_RA124                 REG(0x124)
//offset definition see DC_FLC_xxx

/* RDMA */
#define RDMA_CHN_JMP 0x20
#define RDMA_CHN_COUNT 4
#define DC_RB000(i)             (REG(0x1000) + RDMA_CHN_JMP * (i))
#define DC_SHIFT00_RB000              0
#define DC_MASK00_RB000               (0xFFFF << DC_SHIFT00_RB000)

#define DC_RB004(i)           (REG(0x1004) + RDMA_CHN_JMP * (i))
#define DC_SHIFT00_RB004            0
#define DC_MASK00_RB004             (0xFFFF << DC_SHIFT00_RB004)


#define DC_RB008(i)           (REG(0x1008) + RDMA_CHN_JMP * (i))
#define DC_SHIFT00_RB008            0
#define DC_MASK00_RB008             (0xFFFF << DC_SHIFT00_RB004)

#define DC_RB00C(i)               (REG(0x100c) + RDMA_CHN_JMP * (i))
#define DC_SHIFT16_RB00C           16
#define DC_MASK16_RB00C            0x3f << DC_SHIFT16_RB00C
#define DC_SHIFT08_RB00C             8
#define DC_MASK08_RB00C              0x3f << DC_SHIFT08_RB00C
#define DC_SHIFT00_RB00C             0
#define DC_MASK00_RB00C              0x3f << DC_SHIFT00_RB00C


#define DC_RB010(i)                 (REG(0x1010) + RDMA_CHN_JMP * (i))
#define DC_SHIFT03_RB010             3
#define DC_MASK03_RB010              0x1UL << DC_SHIFT03_RB010
#define DC_SHIFT00_RB010              0
#define DC_MASK00_RB010               0x7UL << DC_SHIFT00_RB010

#define DC_RB014(i)              (REG(0x1014) + RDMA_CHN_JMP * (i))
#define DC_SHIFT00_RB014               0
#define DC_MASK00_RB014                0xFFFFF << DC_SHIFT00_RB014

#define DC_RB018(i)              (REG(0x1018) + RDMA_CHN_JMP * (i))
#define DC_SHIFT04_RB018          4
#define DC_MASK04_RB018           0x3 << DC_SHIFT04_RB018
#define DC_SHIFT00_RB018         0
#define DC_MASK00_RB018          0xF << DC_SHIFT00_RB018

#define DC_RB01C(i)              (REG(0x101c) + RDMA_CHN_JMP * (i))
#define DC_SHIFT16_RB01C      16
#define DC_MASK16_RB01C       0x3FF << DC_SHIFT16_RB01C
#define DC_SHIFT04_RB01C          4
#define DC_MASK04_RB01C           0x7 << DC_SHIFT04_RB01C
#define DC_SHIFT00_RB01C            0
#define DC_MASK00_RB01C             0x7 << DC_SHIFT00_RB01C

#define DC_RB100                     REG(0x1100)
#define DC_SHIFT01_RB100         1
#define DC_MASK01_RB100          0x1UL << DC_SHIFT01_RB100
#define DC_SHIFT00_RB100          0
#define DC_MASK00_RB100           0x1UL << DC_SHIFT00_RB100

#define DC_RB200               REG(0x1200)
#define DC_RB204              REG(0x1204)
#define DC_RB208               REG(0x1208)
#define DC_RB20C              REG(0x120c)
#define DC_RB210                  REG(0x1210)
#define DC_RB220                 REG(0x1220)
#define DC_RB224               REG(0x1224)
#define RDMA_INT_DEF_MASK                0x7F
#define DC_SHIFT06_RB224                  6
#define DC_MASK06_RB224                   1UL << DC_SHIFT06_RB224
#define DC_SHIFT05_RB224                  5
#define DC_MASK05_RB224                   1UL << DC_SHIFT05_RB224
#define DC_SHIFT04_RB224                  4
#define DC_MASK04_RB224                   1UL << DC_SHIFT04_RB224
#define DC_SHIFT03_RB224                  3
#define DC_MASK03_RB224                   1UL << DC_SHIFT03_RB224
#define DC_SHIFT02_RB224                  2
#define DC_MASK02_RB224                   1UL << DC_SHIFT02_RB224
#define DC_SHIFT01_RB224                  1
#define DC_MASK01_RB224                   1UL << DC_SHIFT01_RB224
#define DC_SHIFT00_RB224                  0
#define DC_MASK00_RB224                   1UL << DC_SHIFT00_RB224

#define DC_RB240               REG(0x1240)
#define DC_SHIFT00_RB240                   0
#define DC_MASK00_RB240                    0xF << DC_SHIFT00_RB240
#define DC_RB244                REG(0x1244)
#define DC_SHIFT16_RB244             16
#define DC_MASK16_RB244              0xFFFF << DC_SHIFT16_RB244
#define DC_SHIFT00_RB244             0
#define DC_MASK00_RB244              0xFFFF << DC_SHIFT00_RB244

#define DC_RB400(i)               (REG(0x1400) + RDMA_CHN_JMP * (i))
#define DC_RB404(i)             (REG(0x1404) + RDMA_CHN_JMP * (i))
#define DC_RB408(i)             (REG(0x1408) + RDMA_CHN_JMP * (i))
#define DC_RB40C(i)                 (REG(0x140c) + RDMA_CHN_JMP * (i))
#define DC_RB410(i)                   (REG(0x1410) + RDMA_CHN_JMP * (i))
#define DC_RB414(i)                (REG(0x1414) + RDMA_CHN_JMP * (i))
#define DC_RB418(i)                (REG(0x1418) + RDMA_CHN_JMP * (i))
#define DC_RB41C(i)                (REG(0x141c) + RDMA_CHN_JMP * (i))
#define DC_RB500                        REG(0x1500)
#define DC_RB600                  REG(0x1600)
#define DC_RB604                 REG(0x1604)
#define DC_RB608                  REG(0x1608)
#define DC_RB60C                 REG(0x160c)
#define DC_RB610                     REG(0x1610)
#define DC_RB620                    REG(0x1620)
#define DC_RB624                  REG(0x1624)
#define DC_RB640                  REG(0x1640)
#define DC_RB644                   REG(0x1644)

/* GP */
#define DC_RC000                  REG(0x2000)
#define DC_SHIFT24_RC000                       24
#define DC_MASK24_RC000                        0xF << DC_SHIFT24_RC000
#define DC_SHIFT16_RC000                       16
#define DC_MASK16_RC000                        0xF << DC_SHIFT16_RC000
#define DC_SHIFT08_RC000                       8
#define DC_MASK08_RC000                        0x1F << DC_SHIFT08_RC000
#define DC_SHIFT00_RC000                       0
#define DC_MASK00_RC000                        0xF << DC_SHIFT00_RC000

#define DC_RC004                  REG(0x2004)
#define DC_SHIFT16_RC004               16
#define DC_MASK16_RC004                0x7 << DC_SHIFT16_RC004
#define DC_SHIFT12_RC004                 12
#define DC_MASK12_RC004                  0xF << DC_SHIFT12_RC004
#define DC_SHIFT08_RC004                       8
#define DC_MASK08_RC004                        0x7 << DC_SHIFT08_RC004
#define DC_SHIFT07_RC004                   7
#define DC_MASK07_RC004                    1UL << DC_SHIFT07_RC004
#define DC_SHIFT06_RC004                   6
#define DC_MASK06_RC004                    1 << DC_SHIFT06_RC004
#define DC_SHIFT04_RC004                   4
#define DC_MASK04_RC004                    0x3 << DC_SHIFT04_RC004
#define DC_SHIFT02_RC004                 2
#define DC_MASK02_RC004                  0x3 << DC_SHIFT02_RC004
#define DC_SHIFT00_RC004                       0
#define DC_MASK00_RC004                        0x3 << DC_SHIFT00_RC004

#define DC_RC008                  REG(0x2008)
#define DC_SHIFT16_RC008                16
#define DC_MASK16_RC008                 0xFFFF << DC_SHIFT16_RC008
#define DC_SHIFT00_RC008                 0
#define DC_MASK00_RC008                  0xFFFF << DC_SHIFT00_RC008

#define DC_RC00C                 REG(0x200c)
#define DC_SHIFT00_RC00C                 0
#define DC_MASK00_RC00C                  0xFFFFFFFF << DC_SHIFT00_RC00C

#define DC_RC010                 REG(0x2010)
#define DC_SHIFT00_RC010                 0
#define DC_MASK00_RC010                  0xFF << DC_SHIFT00_RC010

#define DC_RC014                 REG(0x2014)
#define DC_SHIFT00_RC014                 0
#define DC_MASK00_RC014                  0xFFFFFFFF << DC_SHIFT00_RC014

#define DC_RC018                 REG(0x2018)
#define DC_SHIFT00_RC018                 0
#define DC_MASK00_RC018                  0xFF << DC_SHIFT00_RC018

#define DC_RC01C                 REG(0x201c)
#define DC_SHIFT00_RC01C                 0
#define DC_MASK00_RC01C                  0xFFFFFFFF << DC_SHIFT00_RC01C

#define DC_RC020                 REG(0x2020)
#define DC_SHIFT00_RC020                 0
#define DC_MASK00_RC020                  0xFF << DC_SHIFT00_RC020

#define DC_RC02C                  REG(0x202c)
#define DC_SHIFT00_RC02C                  0
#define DC_MASK00_RC02C                   0x3FFFFUL << DC_SHIFT00_RC02C

#define DC_RC030                  REG(0x2030)
#define DC_SHIFT00_RC030                  0
#define DC_MASK00_RC030                   0x3FFFFUL << DC_SHIFT00_RC030

#define DC_RC034                  REG(0x2034)
#define DC_SHIFT00_RC034                  0
#define DC_MASK00_RC034                   0x3FFFFUL << DC_SHIFT00_RC034

#define DC_RC040                REG(0x2040)
#define DC_SHIFT16_RC040                     16
#define DC_MASK16_RC040                      0xFFFFUL << DC_SHIFT16_RC040
#define DC_SHIFT00_RC040                     0
#define DC_MASK00_RC040                      0xFFFFUL << DC_SHIFT00_RC040

#define DC_RC044                REG(0x2044)
#define DC_SHIFT31_RC044            31
#define DC_MASK31_RC044             0x1 << DC_SHIFT31_RC044
#define DC_SHIFT06_RC044        6
#define DC_MASK06_RC044         0x3 << DC_SHIFT06_RC044
#define DC_SHIFT04_RC044        4
#define DC_MASK04_RC044         0x3 << DC_SHIFT04_RC044
#define DC_SHIFT03_RC044   3
#define DC_MASK03_RC044    0x1 << DC_SHIFT03_RC044
#define DC_SHIFT02_RC044    2
#define DC_MASK02_RC044     0x1 << DC_SHIFT02_RC044
#define DC_SHIFT01_RC044    1
#define DC_MASK01_RC044     0x1 << DC_SHIFT01_RC044
#define DC_SHIFT00_RC044        0
#define DC_MASK00_RC044         0x1 << DC_SHIFT00_RC044

/*GP CSC*/
#define DC_RC200                     REG(0x2200)
#define DC_SHIFT02_RC200              2
#define DC_MASK02_RC200               0x1 << DC_SHIFT02_RC200
#define DC_SHIFT01_RC200          1
#define DC_MASK01_RC200           0x1 << DC_SHIFT01_RC200
#define DC_SHIFT00_RC200             0
#define DC_MASK00_RC200              0x1 << DC_SHIFT00_RC200

#define DC_RC204                    REG(0x2204)
#define DC_SHIFT16_RC204          16
#define DC_MASK16_RC204           0x3FFF << DC_SHIFT16_RC204
#define DC_SHIFT00_RC204          0
#define DC_MASK00_RC204           0x3FFF << DC_SHIFT00_RC204


#define DC_RC208                 REG(0x2208)
#define DC_SHIFT16_RC208          16
#define DC_MASK16_RC208           0x3FFF << DC_SHIFT16_RC208
#define DC_SHIFT00_RC208          0
#define DC_MASK00_RC208           0x3FFF << DC_SHIFT00_RC208


#define DC_RC20C                 REG(0x220c)
#define DC_SHIFT16_RC20C          16
#define DC_MASK16_RC20C           0x3FFF << DC_SHIFT16_RC20C
#define DC_SHIFT00_RC20C          0
#define DC_MASK00_RC204C           0x3FFF << DC_SHIFT00_RC20C


#define DC_RC210                 REG(0x2210)
#define DC_SHIFT16_RC210          16
#define DC_MASK16_RC210           0x3FFF << DC_SHIFT16_RC210
#define DC_SHIFT00_RC210          0
#define DC_MASK00_RC210           0x3FFF << DC_SHIFT00_RC210


#define DC_RC214                 REG(0x2214)
#define DC_SHIFT16_RC214           16
#define DC_MASK16_RC214            0x3FFF << DC_SHIFT16_RC214
#define DC_SHIFT00_RC214          0
#define DC_MASK00_RC214           0x3FFF << DC_SHIFT00_RC214

#define DC_RC218                 REG(0x2218)
#define DC_SHIFT16_RC218           16
#define DC_MASK16_RC218            0x3FFF << DC_SHIFT16_RC218
#define DC_SHIFT00_RC218           0
#define DC_MASK00_RC218            0x3FFF << DC_SHIFT00_RC218

#define DC_RC21C                 REG(0x221c)
#define DC_SHIFT16_RC21C           16
#define DC_MASK16_RC21C            0x3FF << DC_SHIFT16_RC21C
#define DC_SHIFT00_RC21C           0
#define DC_MASK00_RC21C            0x3FF << DC_SHIFT00_RC21C

#define DC_RC220                 REG(0x2220)
#define DC_SHIFT00_RC220           0
#define DC_MASK00_RC220            0x3FF << DC_SHIFT00_RC220

/*GP RST*/
#define DC_RCE00                    REG(0x2e00)
#define DC_SHIFT00_RCE00                 0
#define DC_MASK00_RCE00                  1 << DC_SHIFT00_RCE00

/*GP SDW*/
#define DC_RCF00                  REG(0x2f00)
#define DC_SHIFT00_RCF00          0
#define DC_MASK00_RCF00           1 << DC_SHIFT00_RCF00

/* SP */
#define SP_NORMAL      0
#define SP_SAFETY      1

#define SP_JMP        0x1000
#define SP_COUNT      2

/* SHIFT MASK define see DC_GP_XX area  */
#define DC_RD000(i)              (REG(0x5000) + SP_JMP * i)
#define DC_RD004(i)              (REG(0x5004) + SP_JMP * i)
#define DC_RD008(i)              (REG(0x5008) + SP_JMP * i)
#define DC_RD00C(i)             (REG(0x500c) + SP_JMP * i)
#define DC_RD010(i)             (REG(0x5010) + SP_JMP * i)
#define DC_RD02C(i)              (REG(0x502c) + SP_JMP * i)

#define DC_RD040(i)            (REG(0x5040) + SP_JMP * i)

/* RLE */
#define DC_RD100(i)                       REG(0x5100 + SP_JMP * i)
#define DC_SHIFT00_RD100               0
#define DC_MASK00_RD100                0xFFFFFF << DC_SHIFT00_RD100

#define DC_RD110(i)                 REG(0x5110 + SP_JMP * i)
#define DC_SHIFT00_RD110         0
#define DC_MASK00_RD110          0xFFFFFFFF << DC_SHIFT00_RD110

#define DC_RD120(i)                        REG(0x5120 + SP_JMP * i)
#define DC_SHIFT01_RD120             1
#define DC_MASK01_RD120              0x3 << DC_SHIFT01_RD120
#define DC_SHIFT00_RD120                    0
#define DC_MASK00_RD120                     0x1 << DC_SHIFT00_RD120

#define DC_RD130(i)              REG(0x5130 + SP_JMP * i)
#define DC_RD134(i)              REG(0x5134 + SP_JMP * i)
#define DC_RD138(i)              REG(0x5138 + SP_JMP * i)
#define DC_RD13C(i)              REG(0x513c + SP_JMP * i)

#define DC_RD140(i)                    REG(0x5140 + SP_JMP * i)
#define DC_RD144(i)                  REG(0x5144 + SP_JMP * i)
#define DC_SHIFT03_RD144             3
#define DC_MASK03_RD144              0x1 << DC_SHIFT03_RD144
#define DC_SHIFT02_RD144             2
#define DC_MASK02_RD144              0x1 << DC_SHIFT02_RD144
#define DC_SHIFT01_RD144             1
#define DC_MASK01_RD144              0x1 << DC_SHIFT01_RD144
#define DC_SHIFT00_RD144             0
#define DC_MASK00_RD144              0x1 << DC_SHIFT00_RD144

/* CLUT */
#define DC_RD200(i)              (REG(0x5200) + SP_JMP * i)
#define DC_SHIFT18_RD200            18
#define DC_MASK18_RD200             0x1 << DC_SHIFT18_RD200
#define DC_SHIFT17_RD200              17
#define DC_MASK17_RD200               0x1 << DC_SHIFT17_RD200
#define DC_SHIFT16_RD200             16
#define DC_MASK16_RD200              0x1 << DC_SHIFT16_RD200
#define DC_SHIFT08_RD200             8
#define DC_MASK08_RD200              0xFF << DC_SHIFT08_RD200
#define DC_SHIFT00_RD200              0
#define DC_MASK00_RD200               0xF << DC_SHIFT00_RD200

#define DC_RD204(i)              (REG(0x5204) + SP_JMP * i)
#define DC_SHIFT16_RD204             16
#define DC_MASK16_RD204              0x1 << DC_SHIFT16_RD204
#define DC_SHIFT08_RD204             8
#define DC_MASK08_RD204              0xFF << DC_SHIFT08_RD204
#define DC_SHIFT00_RD204              0
#define DC_MASK00_RD204               0xF << DC_SHIFT00_RD204

#define DC_RD208(i)              (REG(0x5208) + SP_JMP * i)
#define DC_SHIFT17_RD208              17
#define DC_MASK17_RD208               0x1 << DC_SHIFT17_RD208
#define DC_SHIFT16_RD208             16
#define DC_MASK16_RD208              0x1 << DC_SHIFT16_RD208
#define DC_SHIFT08_RD208             8
#define DC_MASK08_RD208              0xFF << DC_SHIFT08_RD208
#define DC_SHIFT00_RD208              0
#define DC_MASK00_RD208               0xF << DC_SHIFT00_RD208

#define DC_RD20C(i)              (REG(0x520c) + SP_JMP * i)
#define DC_SHIFT17_RD20C              17
#define DC_MASK17_RD20C               0x1 << DC_SHIFT17_RD20C
#define DC_SHIFT16_RD20C             16
#define DC_MASK16_RD20C              0x1 << DC_SHIFT16_RD20C
#define DC_SHIFT08_RD20C             8
#define DC_MASK08_RD20C              0xFF << DC_SHIFT08_RD20C
#define DC_SHIFT00_RD20C              0
#define DC_MASK00_RD20C               0xF << DC_SHIFT00_RD20C

#define DC_RD210                  REG(0x5210)
#define DC_SHIFT00_RD210              0
#define DC_MASK00_RD210               0x1 << DC_SHIFT00_RD210

#define DC_RD214                     REG(0x5214)
#define DC_SHIFT00_RD214               0
#define DC_MASK00_RD214                0xFFFFFFFF << DC_SHIFT00_RD214

#define DC_RD218                     REG(0x5218)
#define DC_SHIFT00_RD218               0
#define DC_MASK00_RD218                0xFF << DC_SHIFT00_RD218

#define DC_RD21C                  REG(0x521c)
#define DC_SHIFT00_RD21C         0
#define DC_MASK00_RD21C          1 << DC_SHIFT00_RD21C

/*SP RST*/
#define DC_RDE00(i)                (REG(0x5e00) + SP_JMP * i)
#define DC_SHIFT00_RDE00                 0
#define DC_MASK00_RDE00                  1 << DC_SHIFT00_RDE00

/* SP_SDW_CTRL */
#define DC_RDF00(i)              (REG(0x5f00) + SP_JMP * i)
#define DC_SHIFT00_RDF00          0
#define DC_MASK00_RDF00           0x1 << DC_SHIFT00_RDF00

/* MLC */
#define MLC_LAYER_JMP              0x30
#define MLC_LAYER_COUNT            4
#define MLC_PATH_JMP               0x4
#define MLC_PATH_COUNT             5

#define DC_RE000(i)         (REG(0x7000) + MLC_LAYER_JMP * (i))
#define DC_SHIFT08_RE000      8
#define DC_MASK08_RE000       0x3F << DC_SHIFT08_RE000
#define DC_SHIFT07_RE000  7
#define DC_MASK07_RE000   0x1 << DC_SHIFT07_RE000
#define DC_SHIFT06_RE000   6
#define DC_MASK06_RE000    0x1 << DC_SHIFT06_RE000
#define DC_SHIFT05_RE000     5
#define DC_MASK05_RE000      0x1 << DC_SHIFT05_RE000
#define DC_SHIFT04_RE000       4
#define DC_MASK04_RE000        0x1 << DC_SHIFT04_RE000
#define DC_SHIFT03_RE000       3
#define DC_MASK03_RE000        0x1 << DC_SHIFT03_RE000
#define DC_SHIFT02_RE000    2
#define DC_MASK02_RE000     0x1 << DC_SHIFT02_RE000
#define DC_SHIFT01_RE000       1
#define DC_MASK01_RE000        0x1 << DC_SHIFT01_RE000
#define DC_SHIFT00_RE000            0
#define DC_MASK00_RE000             0x1 << DC_SHIFT00_RE000

#define DC_RE004(i)       (REG(0x7004) + MLC_LAYER_JMP * (i))
#define DC_SHIFT00_RE004      0
#define DC_MASK00_RE004       0x1FFFF << DC_SHIFT00_RE004

#define DC_RE008(i)       (REG(0x7008) + MLC_LAYER_JMP * (i))
#define DC_SHIFT00_RE008      0
#define DC_MASK00_RE008       0x1FFFF << DC_SHIFT00_RE008

#define DC_RE00C(i)         (REG(0x700c) + MLC_LAYER_JMP * (i))
#define DC_SHIFT16_RE00C        16
#define DC_MASK16_RE00C         0xFFFF << DC_SHIFT16_RE00C
#define DC_SHIFT00_RE00C        0
#define DC_MASK00_RE00C         0xFFFF << DC_SHIFT00_RE00C

#define DC_RE010(i)   (REG(0x7010) + MLC_LAYER_JMP * (i))
#define DC_RE014(i)   (REG(0x7014) + MLC_LAYER_JMP * (i))
#define DC_SHIFT16_RE010      16
#define DC_MASK16_RE010       0xFFFF << DC_SHIFT16_RE010
#define DC_SHIFT00_RE010    0
#define DC_MASK00_RE010     0xFFFF << DC_SHIFT00_RE010

#define DC_RE018(i)     (REG(0x7018) + MLC_LAYER_JMP * (i))
#define DC_SHIFT00_RE018     0
#define DC_MASK00_RE018      0xFF << DC_SHIFT00_RE018

#define DC_RE01C(i)   (REG(0x701c) + MLC_LAYER_JMP * (i))
#define DC_SHIFT00_RE01C  0
#define DC_MASK00_RE01C   0xFF << DC_SHIFT00_RE01C

#define DC_RE020(i)    (REG(0x7020) + MLC_LAYER_JMP * (i))
#define DC_RE024(i)    (REG(0x7024) + MLC_LAYER_JMP * (i))
#define DC_RE028(i)    (REG(0x7028) + MLC_LAYER_JMP * (i))
#define DC_SHIFT16_RE020    16
#define DC_MASK16_RE020     0x3FF << DC_SHIFT16_RE020
#define DC_SHIFT00_RE020    0
#define DC_MASK00_RE020     0x3FF << DC_SHIFT00_RE020

#define DC_RE02C(i)    (REG(0x702c) + MLC_LAYER_JMP * (i))
#define DC_SHIFT00_RE02C    0
#define DC_MASK00_RE02C     0xFFFFFFFF << DC_SHIFT00_RE02C

#define DC_RE200(i)       (REG(0x7200) + MLC_PATH_JMP * (i))
#define DC_SHIFT16_RE200        16
#define DC_MASK16_RE200         0xF << DC_SHIFT16_RE200
#define DC_SHIFT00_RE200        0
#define DC_MASK00_RE200         0xF << DC_SHIFT00_RE200

#define DC_RE220               REG(0x7220)
#define DC_SHIFT08_RE220                8
#define DC_MASK08_RE220                 0xFF << DC_SHIFT08_RE220
#define DC_SHIFT07_RE220             7
#define DC_MASK07_RE220              0x1 << DC_SHIFT07_RE220
#define DC_SHIFT04_RE220          4
#define DC_MASK04_RE220           0x7 << DC_SHIFT04_RE220
#define DC_SHIFT02_RE220            2
#define DC_MASK02_RE220             0x1 << DC_SHIFT02_RE220
#define DC_SHIFT01_RE220               1
#define DC_MASK01_RE220                0x1 << DC_SHIFT01_RE220
#define DC_SHIFT00_RE220      0
#define DC_MASK00_RE220       0x1 << DC_SHIFT00_RE220

#define DC_RE224              REG(0x7224)
#define DC_SHIFT20_RE224          20
#define DC_MASK20_RE224           0x3FF << DC_SHIFT20_RE224
#define DC_SHIFT10_RE224          10
#define DC_MASK10_RE224           0x3FF << DC_SHIFT10_RE224
#define DC_SHIFT00_RE224          0
#define DC_MASK00_RE224           0x3FF << DC_SHIFT00_RE224

#define DC_RE228          REG(0x7228)
#define DC_SHIFT00_RE228   0
#define DC_MASK00_RE228    0xFFFFFFFF << DC_SHIFT00_RE228

#define DC_RE230          REG(0x7230)
#define DC_SHIFT20_RE230      20
#define DC_MASK20_RE230       0x3FF << DC_SHIFT20_RE230
#define DC_SHIFT10_RE230      10
#define DC_MASK10_RE230       0x3FF << DC_SHIFT10_RE230
#define DC_SHIFT00_RE230      0
#define DC_MASK00_RE230       0x3FF << DC_SHIFT00_RE230

#define DC_RE234             REG(0x7234)
#define DC_SHIFT00_RE234       0
#define DC_MASK00_RE234        0xFFFF << DC_SHIFT00_RE234

#define DC_RE238      REG(0x7238)
#define DC_SHIFT00_RE238  0
#define DC_MASK00_RE238   0xFFFFFFFF << DC_SHIFT00_RE238

#define DC_RE240              REG(0x7240)
#define DC_SHIFT12_RE240    12
#define DC_MASK12_RE240     0x1 << DC_SHIFT12_RE240
#define DC_SHIFT11_RE240    11
#define DC_MASK11_RE240     0x1 << DC_SHIFT11_RE240
#define DC_SHIFT10_RE240    10
#define DC_MASK10_RE240     0x1 << DC_SHIFT10_RE240
#define DC_SHIFT09_RE240    9
#define DC_MASK09_RE240     0x1 << DC_SHIFT09_RE240
#define DC_SHIFT08_RE240    8
#define DC_MASK08_RE240     0x1 << DC_SHIFT08_RE240
#define DC_SHIFT07_RE240    7
#define DC_MASK07_RE240     0x1 << DC_SHIFT07_RE240
#define DC_SHIFT06_RE240    6
#define DC_MASK06_RE240     0x1 << DC_SHIFT06_RE240
#define DC_SHIFT05_RE240    5
#define DC_MASK05_RE240     0x1 << DC_SHIFT05_RE240
#define DC_SHIFT04_RE240    4
#define DC_MASK04_RE240     0x1 << DC_SHIFT04_RE240
#define DC_SHIFT03_RE240    3
#define DC_MASK03_RE240     0x1 << DC_SHIFT03_RE240
#define DC_SHIFT02_RE240    2
#define DC_MASK02_RE240     0x1 << DC_SHIFT02_RE240
#define DC_SHIFT01_RE240    1
#define DC_MASK01_RE240     0x1 << DC_SHIFT01_RE240
#define DC_SHIFT00_RE240    0
#define DC_MASK00_RE240     0x1 << DC_SHIFT00_RE240

#define DC_RE244            REG(0x7244)
#define DC_SHIFT27_RE244       27
#define DC_MASK27_RE244        0x1 << DC_SHIFT27_RE244
#define DC_SHIFT26_RE244       26
#define DC_MASK26_RE244        0x1 << DC_SHIFT26_RE244
#define DC_SHIFT25_RE244       25
#define DC_MASK25_RE244        0x1 << DC_SHIFT25_RE244
#define DC_SHIFT24_RE244       24
#define DC_MASK24_RE244        0x1 << DC_SHIFT24_RE244
#define DC_SHIFT23_RE244       23
#define DC_MASK23_RE244        0x1 << DC_SHIFT23_RE244
#define DC_SHIFT22_RE244       22
#define DC_MASK22_RE244        0x1 << DC_SHIFT22_RE244
#define DC_SHIFT21_RE244      21
#define DC_MASK21_RE244       0x1 << DC_SHIFT21_RE244
#define DC_SHIFT20_RE244      20
#define DC_MASK20_RE244       0x1 << DC_SHIFT20_RE244
#define DC_SHIFT19_RE244      19
#define DC_MASK19_RE244       0x1 << DC_SHIFT19_RE244
#define DC_SHIFT18_RE244      18
#define DC_MASK18_RE244       0x1 << DC_SHIFT18_RE244
#define DC_SHIFT17_RE244      17
#define DC_MASK17_RE244       0x1 << DC_SHIFT17_RE244
#define DC_SHIFT16_RE244      16
#define DC_MASK16_RE244       0x1 << DC_SHIFT16_RE244
#define DC_SHIFT12_RE244           12
#define DC_MASK12_RE244            0x1 << DC_SHIFT12_RE244
#define DC_SHIFT11_RE244           11
#define DC_MASK11_RE244            0x1 << DC_SHIFT11_RE244
#define DC_SHIFT10_RE244           10
#define DC_MASK10_RE244            0x1 << DC_SHIFT10_RE244
#define DC_SHIFT09_RE244           9
#define DC_MASK09_RE244            0x1 << DC_SHIFT09_RE244
#define DC_SHIFT08_RE244           8
#define DC_MASK08_RE244            0x1 << DC_SHIFT08_RE244
#define DC_SHIFT07_RE244           7
#define DC_MASK07_RE244            0x1 << DC_SHIFT07_RE244
#define DC_SHIFT06_RE244         6
#define DC_MASK06_RE244          0x1 << DC_SHIFT06_RE244
#define DC_SHIFT05_RE244         5
#define DC_MASK05_RE244          0x1 << DC_SHIFT05_RE244
#define DC_SHIFT04_RE244         4
#define DC_MASK04_RE244          0x1 << DC_SHIFT04_RE244
#define DC_SHIFT03_RE244         3
#define DC_MASK03_RE244          0x1 << DC_SHIFT03_RE244
#define DC_SHIFT02_RE244         2
#define DC_MASK02_RE244          0x1 << DC_SHIFT02_RE244
#define DC_SHIFT01_RE244         1
#define DC_MASK01_RE244          0x1 << DC_SHIFT01_RE244
#define DC_SHIFT00_RE244         0
#define DC_MASK00_RE244          0x1 << DC_SHIFT00_RE244

#define DC_RF000(i)           (REG(0x8000) + MLC_LAYER_JMP * (i))
#define DC_RF004(i)         (REG(0x8004) + MLC_LAYER_JMP * (i))
#define DC_RF008(i)         (REG(0x8008) + MLC_LAYER_JMP * (i))
#define DC_RF00C(i)           (REG(0x800c) + MLC_LAYER_JMP * (i))
#define DC_RF010(i)     (REG(0x8010) + MLC_LAYER_JMP * (i))
#define DC_RF014(i)     (REG(0x8014) + MLC_LAYER_JMP * (i))
#define DC_RF018(i)        (REG(0x8018) + MLC_LAYER_JMP * (i))
#define DC_RF01C(i)     (REG(0x801c) + MLC_LAYER_JMP * (i))
#define DC_RF020(i)      (REG(0x8020) + MLC_LAYER_JMP * (i))
#define DC_RF024(i)      (REG(0x8024) + MLC_LAYER_JMP * (i))
#define DC_RF028(i)      (REG(0x8028) + MLC_LAYER_JMP * (i))
#define DC_RF02C(i)      (REG(0x802c) + MLC_LAYER_JMP * (i))
#define DC_RF200(i)         (REG(0x8200) + MLC_PATH_JMP * (i))
#define DC_RF220               REG(0x8220)
#define DC_RF224              REG(0x8224)
#define DC_RF228          REG(0x8228)
#define DC_RF230          REG(0x8230)
#define DC_RF234             REG(0x8234)
#define DC_RF240              REG(0x8240)
#define DC_RF244            REG(0x8244)

/* TCON */
#define KICK_LAYER_JMP              0x8
#define KICK_LAYER_COUNT            19

#define DC_RG000               REG(0x9000)
#define DC_SHIFT16_RG000             16
#define DC_MASK16_RG000              0xFFFF << DC_SHIFT16_RG000
#define DC_SHIFT00_RG000             0
#define DC_MASK00_RG000              0xFFFF << DC_SHIFT00_RG000

#define DC_RG004               REG(0x9004)
#define DC_SHIFT16_RG004             16
#define DC_MASK16_RG004              0xFFFF << DC_SHIFT16_RG004
#define DC_SHIFT00_RG004            0
#define DC_MASK00_RG004             0xFFFF << DC_SHIFT00_RG004

#define DC_RG008               REG(0x9008)
#define DC_SHIFT16_RG008             16
#define DC_MASK16_RG008              0xFFFF << DC_SHIFT16_RG008
#define DC_SHIFT00_RG008             0
#define DC_MASK00_RG008              0xFFFF << DC_SHIFT00_RG008

#define DC_RG00C               REG(0x900c)
#define DC_SHIFT16_RG00C             16
#define DC_MASK16_RG00C              0xFFFF << DC_SHIFT16_RG00C
#define DC_SHIFT00_RG00C            0
#define DC_MASK00_RG00C             0xFFFF << DC_SHIFT00_RG00C


#define DC_RG010                   REG(0x9010)
#define DC_SHIFT06_RG010          6
#define DC_MASK06_RG010           0x3 << DC_SHIFT06_RG010
#define DC_SHIFT05_RG010       5
#define DC_MASK05_RG010        0x1 << DC_SHIFT05_RG010
#define DC_SHIFT04_RG010      4
#define DC_MASK04_RG010       0x1 << DC_SHIFT04_RG010
#define DC_SHIFT03_RG010           3
#define DC_MASK03_RG010            0x1 << DC_SHIFT03_RG010
#define DC_SHIFT02_RG010        2
#define DC_MASK02_RG010         0x1 << DC_SHIFT02_RG010
#define DC_SHIFT01_RG010        1
#define DC_MASK01_RG010         0x1 << DC_SHIFT01_RG010
#define DC_SHIFT00_RG010               0
#define DC_MASK00_RG010                0x1 << DC_SHIFT00_RG010

#define DC_RG020(i)    (REG(0x9020) + KICK_LAYER_JMP * (i))
#define DC_SHIFT16_RG020     16
#define DC_MASK16_RG020      0xFFFF << DC_SHIFT16_RG020
#define DC_SHIFT00_RG020     0
#define DC_MASK00_RG020      0xFFFF << DC_SHIFT00_RG020

#define DC_RG024(i)      (REG(0x9024) + KICK_LAYER_JMP * (i))
#define DC_SHIFT00_RG024    0
#define DC_MASK00_RG024     0x1 << DC_SHIFT00_RG024

#define DC_RG100           REG(0x9100)
#define DC_SHIFT00_RG100       0
#define DC_MASK00_RG100        0xFFFFFFFF << DC_SHIFT00_RG100


#define TCON_POST_CH_JMP            0x4
#define TCON_POST_CH_COUNT          4

#define DC_RG400              REG(0x9400)
#define DC_SHIFT_01_RG400          1
#define DC_MASK_01_RG400           0x1 << DC_SHIFT_01_RG400
#define DC_SHIFT_00_RG400    0
#define DC_MASK_00_RG400     0x1 << DC_SHIFT_00_RG400

#define DC_RG410(i)       (REG(0x9410) + TCON_POST_CH_JMP * (i))
#define DC_SHIFT_03_RG410 3
#define DC_MASK_03_RG410  0x1 << DC_SHIFT_03_RG410
#define DC_SHIFT_02_RG410  2
#define DC_MASK_02_RG410   0x1 << DC_SHIFT_02_RG410
#define DC_SHIFT_01_RG410  1
#define DC_MASK_01_RG410   0x1 << DC_SHIFT_01_RG410
#define DC_SHIFT_00_RG410   0
#define DC_MASK_00_RG410    0x1 << DC_SHIFT_00_RG410

#define DC_RG430(i)           (REG(0x9430) + TCON_POST_CH_JMP * (i))
#define DC_SHIFT_16_RG430     16
#define DC_MASK_16_RG430      0xFFFF << DC_SHIFT_16_RG430
#define DC_SHIFT_00_RG430      0
#define DC_MASK_00_RG430       0xFFFF << DC_SHIFT_00_RG430

/* DC_CSC */
#define DC_RH000                 REG(0xa000)
#define DC_SHIFT02_RH000          2
#define DC_MASK02_RH000           0x1 << DC_SHIFT02_RH000
#define DC_SHIFT01_RH000      1
#define DC_MASK01_RH000       0x1 << DC_SHIFT01_RH000
#define DC_SHIFT00_RH000         0
#define DC_MASK00_RH000          0x1 << DC_SHIFT00_RH000

#define DC_RH004                REG(0xa004)
#define DC_SHIFT16_RH004      16
#define DC_MASK16_RH004       0x3FFF << DC_SHIFT16_RH004
#define DC_SHIFT00_RH004      0
#define DC_MASK00_RH004       0x3FFF << DC_SHIFT00_RH004

#define DC_RH008                REG(0xa008)
#define DC_SHIFT16_RH008      16
#define DC_MASK16_RH008       0x3FFF << DC_SHIFT16_RH008
#define DC_SHIFT00_RH008      0
#define DC_MASK00_RH008       0x3FFF << DC_SHIFT00_RH008

#define DC_RH00C                REG(0xa00c)
#define DC_SHIFT16_RH00C      16
#define DC_MASK16_RH00C       0x3FFF << DC_SHIFT16_RH00C
#define DC_SHIFT00_RH00C      0
#define DC_MASK00_RH00C       0x3FFF << DC_SHIFT00_RH00C

#define DC_RH010                REG(0xa010)
#define DC_SHIFT16_RH010      16
#define DC_MASK16_RH010       0x3FFF << DC_SHIFT16_RH010
#define DC_SHIFT00_RH010      0
#define DC_MASK00_RH010       0x3FFF << DC_SHIFT00_RH010

#define DC_RH014                REG(0xa014)
#define DC_SHIFT16_RH014       16
#define DC_MASK16_RH014        0x3FFF << DC_SHIFT16_RH014
#define DC_SHIFT00_RH014      0
#define DC_MASK00_RH014       0x3FFF << DC_SHIFT00_RH014

#define DC_RH018                REG(0xa018)
#define DC_SHIFT16_RH018       16
#define DC_MASK16_RH018        0x3FFF << DC_SHIFT16_RH018
#define DC_SHIFT00_RH018       0
#define DC_MASK00_RH018        0x3FFF << DC_SHIFT00_RH018

#define DC_RH01C                REG(0xa01c)
#define DC_SHIFT16_RH01C       16
#define DC_MASK16_RH01C        0x3FF << DC_SHIFT16_RH01C
#define DC_SHIFT00_RH01C       0
#define DC_MASK00_RH01C        0x3FF << DC_SHIFT00_RH01C

#define DC_RH020                REG(0xa020)
#define DC_SHIFT00_RH020       0
#define DC_MASK00_RH020        0x3FF << DC_SHIFT00_RH020

#define DC_RI000                  REG(0xc000)
#define DC_SHIFT08_RI000       8
#define DC_MASK08_RI000        0xFF << DC_SHIFT08_RI000
#define DC_SHIFT00_RI000           0
#define DC_MASK00_RI000            0x1 << DC_SHIFT00_RI000

#define DC_RI004                 REG(0xc004)
#define DC_SHIFT16_RI004          16
#define DC_MASK16_RI004           0xF << DC_SHIFT16_RI004
#define DC_SHIFT12_RI004          12
#define DC_MASK12_RI004           0xF << DC_SHIFT12_RI004
#define DC_SHIFT08_RI004          8
#define DC_MASK08_RI004           0xF << DC_SHIFT08_RI004
#define DC_SHIFT06_RI004		6
#define DC_MASK06_RI004			0x1 << DC_SHIFT06_RI004
#define DC_SHIFT04_RI004	4
#define DC_MASK04_RI004	0x3 << DC_SHIFT04_RI004
#define DC_SHIFT03_RI004		3
#define DC_MASK03_RI004			0x3 << DC_SHIFT03_RI004
#define DC_SHIFT02_RI004         2
#define DC_MASK02_RI004          0x1 << DC_SHIFT02_RI004
#define DC_SHIFT01_RI004    		1
#define DC_MASK01_RI004    		0x1 << DC_SHIFT01_RI004
#define DC_SHIFT00_RI004   		0
#define DC_MASK00_RI004    		0x1 << DC_SHIFT00_RI004



/* CRC32 */
#define CRC_BLK_JMP                 0x20
#define CRC_BLK_COUNT               32

#define DC_RJ000                  REG(0xe000)
#define DC_SHIFT09_RJ000       9
#define DC_MASK09_RJ000        0x1 << DC_SHIFT09_RJ000
#define DC_SHIFT08_RJ000       8
#define DC_MASK08_RJ000        0x1 << DC_SHIFT08_RJ000
#define DC_SHIFT07_RJ000     7
#define DC_MASK07_RJ000      0x1 << DC_SHIFT07_RJ000
#define DC_SHIFT00_RJ000   0
#define DC_MASK00_RJ000    0x1 << DC_SHIFT00_RJ000

#define DC_RJ004                REG(0xe004)
#define DC_RJ008              REG(0xe008)
#define DC_SHIFT15_RJ004           15
#define DC_MASK15_RJ004            0x1 << DC_SHIFT15_RJ004
#define DC_SHIFT14_RJ004           14
#define DC_MASK14_RJ004            0x1 << DC_SHIFT14_RJ004
#define DC_SHIFT13_RJ004           13
#define DC_MASK13_RJ004            0x1 << DC_SHIFT13_RJ004
#define DC_SHIFT12_RJ004           12
#define DC_MASK12_RJ004            0x1 << DC_SHIFT12_RJ004
#define DC_SHIFT11_RJ004           11
#define DC_MASK11_RJ004            0x1 << DC_SHIFT11_RJ004
#define DC_SHIFT10_RJ004           10
#define DC_MASK10_RJ004            0x1 << DC_SHIFT10_RJ004
#define DC_SHIFT09_RJ004           9
#define DC_MASK09_RJ004            0x1 << DC_SHIFT09_RJ004
#define DC_SHIFT08_RJ004           8
#define DC_MASK08_RJ004            0x1 << DC_SHIFT08_RJ004
#define DC_SHIFT07_RJ004            7
#define DC_MASK07_RJ004             0x1 << DC_SHIFT07_RJ004
#define DC_SHIFT06_RJ004            6
#define DC_MASK06_RJ004             0x1 << DC_SHIFT06_RJ004
#define DC_SHIFT05_RJ004            5
#define DC_MASK05_RJ004             0x1 << DC_SHIFT05_RJ004
#define DC_SHIFT04_RJ004            4
#define DC_MASK04_RJ004             0x1 << DC_SHIFT04_RJ004
#define DC_SHIFT03_RJ004            3
#define DC_MASK03_RJ004             0x1 << DC_SHIFT03_RJ004
#define DC_SHIFT02_RJ004            2
#define DC_MASK02_RJ004             0x1 << DC_SHIFT02_RJ004
#define DC_SHIFT01_RJ004            1
#define DC_MASK01_RJ004             0x1 << DC_SHIFT01_RJ004
#define DC_SHIFT00_RJ004            0
#define DC_MASK00_RJ004             0x1 << DC_SHIFT00_RJ004

#define DC_RJ010(i)       (REG(0xe010) + CRC_BLK_JMP * (i))
#define DC_SHIFT31_RJ010    31
#define DC_MASK31_RJ010     0x1 << DC_SHIFT31_RJ010
#define DC_SHIFT30_RJ010      30
#define DC_MASK30_RJ010       0X1 << DC_SHIFT30_RJ010
#define DC_SHIFT16_RJ010     16
#define DC_MASK16_RJ010      0x3FFF << DC_SHIFT16_RJ010
#define DC_SHIFT00_RJ010     0
#define DC_MASK00_RJ010      0x3FFF << DC_SHIFT00_RJ010

#define DC_RJ014(i)       (REG(0xe014) + CRC_BLK_JMP * (i))
#define DC_SHIFT16_RJ014       16
#define DC_MASK16_RJ014        0x3FFF << DC_SHIFT16_RJ014
#define DC_SHIFT00_RJ014       0
#define DC_MASK00_RJ014        0x3FFF << DC_SHIFT00_RJ014

#define DC_RJ018(i) (REG(0xe018) + CRC_BLK_JMP * (i))
#define DC_SHIFT00_RJ018     0
#define DC_MASK00_RJ018      0xFFFFFFFF << DC_SHIFT00_RJ018

#define DC_RJ01C(i) (REG(0xe01c) + CRC_BLK_JMP * (i))
#define DC_SHIFT00_RJ01C     0
#define DC_MASK00_RJ01C      0xFFFFFFFF << DC_SHIFT00_RJ01C
#endif //__DC_REG_H__

