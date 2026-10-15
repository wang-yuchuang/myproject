#ifndef _AP_AHB_GAMA_ASW_AB0_REG_H
#define _AP_AHB_GAMA_ASW_AB0_REG_H

//--------------------------------------------------------------------------
// Component          : semidrive.com, d_ip_gama, gama_asw, r0p0
// Memory Map         : AHB
// Address Block Name : AB0
// Description        : AB0
//--------------------------------------------------------------------------
#define AHB_GAMA_ASW_BASE_ADDR 0x0


//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_FCTRL
// Register Offset : 0x0
// Description     : ASW frame control register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_FCTRL (AHB_GAMA_ASW_BASE_ADDR + (0x0<<0))

#define AP_AHB_GAMA_ASW_ASW_FCTRL_ASW_M2M_FS_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_FCTRL_ASW_M2M_FS_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_FCTRL_ASW_M2M_FS
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : ASW frame start when memory-to-memory mode
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_FCTRL_ASW_M2M_FS    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_SOFT_RESET
// Register Offset : 0x4
// Description     : ASW sync reset register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_SOFT_RESET (AHB_GAMA_ASW_BASE_ADDR + (0x4<<0))

#define AP_AHB_GAMA_ASW_ASW_SOFT_RESET_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_SOFT_RESET_VAL_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_SOFT_RESET_VAL
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : ASW sync clear
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_SOFT_RESET_VAL    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_PARA_SET
// Register Offset : 0x8
// Description     : ASW parameters set 0 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_PARA_SET (AHB_GAMA_ASW_BASE_ADDR + (0x8<<0))

#define AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_FIELD_OFFSET 20
#define AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_FIELD_SIZE 12

#define AP_AHB_GAMA_ASW_ASW_PARA_SET_ASW_DC_BUF_SIZE_FIELD_OFFSET 12
#define AP_AHB_GAMA_ASW_ASW_PARA_SET_ASW_DC_BUF_SIZE_FIELD_SIZE 2

#define AP_AHB_GAMA_ASW_ASW_PARA_SET_EXT_LUT_FORMAT_FIELD_OFFSET 11
#define AP_AHB_GAMA_ASW_ASW_PARA_SET_EXT_LUT_FORMAT_FIELD_SIZE 1

#define AP_AHB_GAMA_ASW_ASW_PARA_SET_EXT_CSPLINE_EN_FIELD_OFFSET 10
#define AP_AHB_GAMA_ASW_ASW_PARA_SET_EXT_CSPLINE_EN_FIELD_SIZE 1

#define AP_AHB_GAMA_ASW_ASW_PARA_SET_WORK_MODE_FIELD_OFFSET 8
#define AP_AHB_GAMA_ASW_ASW_PARA_SET_WORK_MODE_FIELD_SIZE 2

#define AP_AHB_GAMA_ASW_ASW_PARA_SET_CACHE_N_WAY_FIELD_OFFSET 4
#define AP_AHB_GAMA_ASW_ASW_PARA_SET_CACHE_N_WAY_FIELD_SIZE 4

#define AP_AHB_GAMA_ASW_ASW_PARA_SET_C_ENTRY_TYPE_FIELD_OFFSET 1
#define AP_AHB_GAMA_ASW_ASW_PARA_SET_C_ENTRY_TYPE_FIELD_SIZE 3

#define AP_AHB_GAMA_ASW_ASW_PARA_SET_LUT_INTERP_MODE_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_PARA_SET_LUT_INTERP_MODE_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW
// Field Width     : 12
// Field Start Bit : 20
// Field End Bit   : 31
// Description     : when CSI lock mode and that line is written out from CSI to RAM, to start ASW
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_11    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_10    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_9    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_8    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_7    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_6    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_5    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_4    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CSI_LN_START_ASW_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET_ASW_DC_BUF_SIZE
// Field Width     : 2
// Field Start Bit : 12
// Field End Bit   : 13
// Description     : ASW-DC handshake ping buffer (or pong buffer) size (in external IRAM)
//                   0=8 lines
//                   1=16 lines
//                   2=32 lines
//                   3=64 lines
//                   (Active when WORK_MODE==2 or 3)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_ASW_DC_BUF_SIZE_1    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_ASW_DC_BUF_SIZE_0    (BIT_(12))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET_EXT_LUT_FORMAT
// Field Width     : 1
// Field Start Bit : 11
// Field End Bit   : 11
// Description     : 0=16bit, sign.11int.4frac
//                   1=32bit, 13sign.11int.8frac
//                   (Active when LUT_INTERP_MODE==1'b0)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_EXT_LUT_FORMAT    (BIT_(11))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET_EXT_CSPLINE_EN
// Field Width     : 1
// Field Start Bit : 10
// Field End Bit   : 10
// Description     : cubic spline mode
//                   0=internal cubic spline assembler
//                   1=external cubic spline assembler
//                   (Active when LUT_INTERP_MODE==1'b1)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_EXT_CSPLINE_EN    (BIT_(10))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET_WORK_MODE
// Field Width     : 2
// Field Start Bit : 8
// Field End Bit   : 9
// Description     : 0=memory to memory
//                   1=CSI handshake only
//                   2=DC handshake only
//                   3=CSI/DC handshake both enable
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_WORK_MODE_1    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_WORK_MODE_0    (BIT_(8))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET_CACHE_N_WAY
// Field Width     : 4
// Field Start Bit : 4
// Field End Bit   : 7
// Description     : n-way associative cache
//                   (1) when (cache entry vsize==8), 10>=way>1
//                   (2) when (cache entry vsize==4), only even number is supported and 10>=way>2
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CACHE_N_WAY_3    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CACHE_N_WAY_2    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CACHE_N_WAY_1    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_CACHE_N_WAY_0    (BIT_(4))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET_C_ENTRY_TYPE
// Field Width     : 3
// Field Start Bit : 1
// Field End Bit   : 3
// Description     : cache entry type (hsize x vsize)
//                   0=8x4
//                   1=8x8
//                   2=16x4
//                   3=16x8
//                   4=32x4
//                   others reserved
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_C_ENTRY_TYPE_2    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_C_ENTRY_TYPE_1    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_C_ENTRY_TYPE_0    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET_LUT_INTERP_MODE
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : 0=bilinear
//                   1=cubic spline
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET_LUT_INTERP_MODE    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_PARA_SET1
// Register Offset : 0xc
// Description     : ASW parameters set 1 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_PARA_SET1 (AHB_GAMA_ASW_BASE_ADDR + (0xc<<0))

#define AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_V_OFFSET_FIELD_OFFSET 12
#define AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_V_OFFSET_FIELD_SIZE 2

#define AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_H_OFFSET_FIELD_OFFSET 10
#define AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_H_OFFSET_FIELD_SIZE 2

#define AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_OFFSET_FIELD_OFFSET 4
#define AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_OFFSET_FIELD_SIZE 6

#define AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_SLOPE_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_SLOPE_FIELD_SIZE 4


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_V_OFFSET
// Field Width     : 2
// Field Start Bit : 12
// Field End Bit   : 13
// Description     : 0=-0.5
//                   1=0
//                   2=+0.5
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_V_OFFSET_1    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_V_OFFSET_0    (BIT_(12))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_H_OFFSET
// Field Width     : 2
// Field Start Bit : 10
// Field End Bit   : 11
// Description     : 0=-0.5
//                   1=0
//                   2=+0.5
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_H_OFFSET_1    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_UV_H_OFFSET_0    (BIT_(10))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_OFFSET
// Field Width     : 6
// Field Start Bit : 4
// Field End Bit   : 9
// Description     : signed setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_OFFSET_5    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_OFFSET_4    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_OFFSET_3    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_OFFSET_2    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_OFFSET_1    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_OFFSET_0    (BIT_(4))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_SLOPE
// Field Width     : 4
// Field Start Bit : 0
// Field End Bit   : 3
// Description     : unsigned setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_SLOPE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_SLOPE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_SLOPE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_PARA_SET1_ADB_SLOPE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_SRC_RES
// Register Offset : 0x10
// Description     : ASW source resolution register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_SRC_RES (AHB_GAMA_ASW_BASE_ADDR + (0x10<<0))

#define AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_FIELD_OFFSET 16
#define AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_FIELD_SIZE 12

#define AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE
// Field Width     : 12
// Field Start Bit : 16
// Field End Bit   : 27
// Description     : =vsize-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_11    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_10    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_9    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_8    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_7    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_VSIZE_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : =hsize-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RES_HSIZE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DES_RES
// Register Offset : 0x14
// Description     : ASW destination resolution register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DES_RES (AHB_GAMA_ASW_BASE_ADDR + (0x14<<0))

#define AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_FIELD_OFFSET 16
#define AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_FIELD_SIZE 12

#define AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE
// Field Width     : 12
// Field Start Bit : 16
// Field End Bit   : 27
// Description     : =vsize-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_11    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_10    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_9    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_8    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_7    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_VSIZE_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : =hsize-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_RES_HSIZE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_IMG_FMT
// Register Offset : 0x18
// Description     : ASW src/des image format register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_IMG_FMT (AHB_GAMA_ASW_BASE_ADDR + (0x18<<0))

#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_VFLIP_FIELD_OFFSET 25
#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_VFLIP_FIELD_SIZE 1

#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_HFLIP_FIELD_OFFSET 24
#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_HFLIP_FIELD_SIZE 1

#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_FMT_FIELD_OFFSET 16
#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_FMT_FIELD_SIZE 4

#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_VFLIP_FIELD_OFFSET 9
#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_VFLIP_FIELD_SIZE 1

#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_HFLIP_FIELD_OFFSET 8
#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_HFLIP_FIELD_SIZE 1

#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_FMT_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_FMT_FIELD_SIZE 4


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_VFLIP
// Field Width     : 1
// Field Start Bit : 25
// Field End Bit   : 25
// Description     : 1=enable
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_VFLIP    (BIT_(25))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_HFLIP
// Field Width     : 1
// Field Start Bit : 24
// Field End Bit   : 24
// Description     : 1=enable
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_HFLIP    (BIT_(24))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_FMT
// Field Width     : 4
// Field Start Bit : 16
// Field End Bit   : 19
// Description     : destination image format(for interleaved, the leftside is LSB)
//                   0=8 bit monotonic
//                   4=16 bit interleaved, RGB565
//                   5=16 bit interleaved, RGBA5551
//                   6=16 bit interleaved, ARGB1555
//                   8=24 bit interleaved, RGB/YUV 888
//                   9=16 bit interleaved YUV422, pattern (U0,Y0,V0,Y1,...)
//                   10=16 bit interleaved YUV422, pattern (V0,Y0,U0,Y1,...)
//                   12=32 bit interleaved, RGBA8888
//                   13=32 bit interleaved, ARGB8888
//                   14=16 bit interleaved YUV422, pattern (Y0,U0,Y1,V0,...)
//                   15=16 bit interleaved YUV422, pattern (Y0,V0,Y1,U0,...)
//                   others reserved
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_FMT_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_FMT_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_FMT_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_DES_FMT_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_VFLIP
// Field Width     : 1
// Field Start Bit : 9
// Field End Bit   : 9
// Description     : 1=enable
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_VFLIP    (BIT_(9))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_HFLIP
// Field Width     : 1
// Field Start Bit : 8
// Field End Bit   : 8
// Description     : 1=enable
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_HFLIP    (BIT_(8))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_FMT
// Field Width     : 4
// Field Start Bit : 0
// Field End Bit   : 3
// Description     : source image format(for interleaved, the leftside is LSB)
//                   0=8 bit monotonic
//                   1=8 bit planar, RGB/YUV 888
//                   2=8 bit planar, YUV422
//                   3=8 bit planar, YUV420
//                   4=16 bit interleaved, RGB565
//                   5=16 bit interleaved, RGBA5551
//                   6=16 bit interleaved, ARGB1555
//                   8=24 bit interleaved, RGB/YUV 888
//                   9=semi planar, YUV422, UV pattern U0V0U1V1...
//                   10=semi planar, YUV422, UV pattern V0U0V1U1...
//                   12=32 bit interleaved, RGBA8888
//                   13=32 bit interleaved, ARGB8888
//                   others reserved
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_FMT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_FMT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_FMT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_IMG_FMT_SRC_FMT_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_CFG_PROT
// Register Offset : 0x1c
// Description     : ASW configuration protection register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_CFG_PROT (AHB_GAMA_ASW_BASE_ADDR + (0x1c<<0))

#define AP_AHB_GAMA_ASW_ASW_CFG_PROT_ADR_PROT_BIT_FIELD_OFFSET 1
#define AP_AHB_GAMA_ASW_ASW_CFG_PROT_ADR_PROT_BIT_FIELD_SIZE 1

#define AP_AHB_GAMA_ASW_ASW_CFG_PROT_CFG_PROT_BIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_CFG_PROT_CFG_PROT_BIT_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_CFG_PROT_ADR_PROT_BIT
// Field Width     : 1
// Field Start Bit : 1
// Field End Bit   : 1
// Description     : source/destination image address protection bit
//                   0=disable shadow load
//                   1=prepare shadow load, wait HW clear after set (also refer to the corresponding interrupt bit)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_CFG_PROT_ADR_PROT_BIT    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_CFG_PROT_CFG_PROT_BIT
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : configuration protection bit(except for source/destination image address)
//                   0=disable shadow load
//                   1=prepare shadow load, wait HW clear after set (also refer to the corresponding interrupt bit)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_CFG_PROT_CFG_PROT_BIT    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_LUT_SIZE
// Register Offset : 0x20
// Description     : ASW LUT size register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_LUT_SIZE (AHB_GAMA_ASW_BASE_ADDR + (0x20<<0))

#define AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_FIELD_OFFSET 16
#define AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_FIELD_SIZE 10

#define AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT
// Field Width     : 10
// Field Start Bit : 16
// Field End Bit   : 25
// Description     : LUT vertical size
//                   =lut_v_size-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_9    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_8    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_7    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_HEIGHT_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : LUT horizontal size
//                   =lut_h_size-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_SIZE_WIDTH_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_LUT_STEP
// Register Offset : 0x24
// Description     : ASW LUT step setting register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_LUT_STEP (AHB_GAMA_ASW_BASE_ADDR + (0x24<<0))

#define AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_FIELD_OFFSET 16
#define AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_FIELD_SIZE 12

#define AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL
// Field Width     : 12
// Field Start Bit : 16
// Field End Bit   : 27
// Description     : LUT step vsize
//                   =lut_step_v-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_11    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_10    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_9    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_8    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_7    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_VVAL_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : LUT step hsize
//                   =lut_step_h-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEP_HVAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP
// Register Offset : 0x28
// Description     : ASW LUT reciprocal steph register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP (AHB_GAMA_ASW_BASE_ADDR + (0x28<<0))

#define AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : setting value
//                   =(int)((1/lut_step_h)*2^20)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPH_RECP_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP
// Register Offset : 0x2c
// Description     : ASW LUT reciprocal stepv register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP (AHB_GAMA_ASW_BASE_ADDR + (0x2c<<0))

#define AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : setting value
//                   =(int)((1/lut_step_v)*2^20)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_LUT_STEPV_RECP_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA
// Register Offset : 0x30
// Description     : ASW position calc parameter file base address register(local buffer)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA (AHB_GAMA_ASW_BASE_ADDR + (0x30<<0))

#define AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_FIELD_SIZE 24


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL
// Field Width     : 24
// Field Start Bit : 0
// Field End Bit   : 23
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_CALC_PFILE_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA
// Register Offset : 0x34
// Description     : ASW position LUT table base address register register(local buffer)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA (AHB_GAMA_ASW_BASE_ADDR + (0x34<<0))

#define AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_FIELD_SIZE 24


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL
// Field Width     : 24
// Field Start Bit : 0
// Field End Bit   : 23
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_LUT_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA
// Register Offset : 0x38
// Description     : ASW spline interpolation M_V table base address register(local buffer)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA (AHB_GAMA_ASW_BASE_ADDR + (0x38<<0))

#define AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_FIELD_SIZE 24


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL
// Field Width     : 24
// Field Start Bit : 0
// Field End Bit   : 23
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_M_V_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA
// Register Offset : 0x40
// Description     : ASW R/Y cache base address register(local buffer)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA (AHB_GAMA_ASW_BASE_ADDR + (0x40<<0))

#define AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_FIELD_SIZE 24


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL
// Field Width     : 24
// Field Start Bit : 0
// Field End Bit   : 23
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_RY_CACHE_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA
// Register Offset : 0x44
// Description     : ASW G/U cache base address register(local buffer)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA (AHB_GAMA_ASW_BASE_ADDR + (0x44<<0))

#define AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_FIELD_SIZE 24


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL
// Field Width     : 24
// Field Start Bit : 0
// Field End Bit   : 23
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_GU_CACHE_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA
// Register Offset : 0x48
// Description     : ASW B/V cache base address register(local buffer)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA (AHB_GAMA_ASW_BASE_ADDR + (0x48<<0))

#define AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_FIELD_SIZE 24


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL
// Field Width     : 24
// Field Start Bit : 0
// Field End Bit   : 23
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_BV_CACHE_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA
// Register Offset : 0x4c
// Description     : ASW alpha cache base address register(local buffer)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA (AHB_GAMA_ASW_BASE_ADDR + (0x4c<<0))

#define AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_FIELD_SIZE 24


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL
// Field Width     : 24
// Field Start Bit : 0
// Field End Bit   : 23
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_A_CACHE_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_PIX_R_BA
// Register Offset : 0x50
// Description     : ASW filter pixel result buffer base address register(local buffer)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_PIX_R_BA (AHB_GAMA_ASW_BASE_ADDR + (0x50<<0))

#define AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_FIELD_SIZE 24


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL
// Field Width     : 24
// Field Start Bit : 0
// Field End Bit   : 23
// Description     : address setting value
//                   (1) 16 byte alignment
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE
// Register Offset : 0x58
// Description     : ASW filter pixel result buffer line stride register(local buffer)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0x58<<0))

#define AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_FIELD_SIZE 16


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL
// Field Width     : 16
// Field Start Bit : 0
// Field End Bit   : 15
// Description     : stride setting value, =stride-1, limit condition:
//                   (1) 4 byte alignment
//                   (2) when des_hflip enable, >=((des_hsize+15)/16)*16
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_PIX_R_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA
// Register Offset : 0x60
// Description     : ASW source image R/Y base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA (AHB_GAMA_ASW_BASE_ADDR + (0x60<<0))

#define AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA
// Register Offset : 0x64
// Description     : ASW source image G/U base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA (AHB_GAMA_ASW_BASE_ADDR + (0x64<<0))

#define AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA
// Register Offset : 0x68
// Description     : ASW source image B/V base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA (AHB_GAMA_ASW_BASE_ADDR + (0x68<<0))

#define AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE
// Register Offset : 0x6c
// Description     : ASW source image R/Y line stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0x6c<<0))

#define AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_RY_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE
// Register Offset : 0x70
// Description     : ASW source image G/U line stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0x70<<0))

#define AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_GU_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE
// Register Offset : 0x74
// Description     : ASW source image B/V line stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0x74<<0))

#define AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_SRC_BV_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DES_BA
// Register Offset : 0x80
// Description     : ASW destination image base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DES_BA (AHB_GAMA_ASW_BASE_ADDR + (0x80<<0))

#define AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DES_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DES_STRIDE
// Register Offset : 0x84
// Description     : ASW destination image  line stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DES_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0x84<<0))

#define AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR
// Register Offset : 0x88
// Description     : ASW destination background color register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR (AHB_GAMA_ASW_BASE_ADDR + (0x88<<0))

#define AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_FIELD_OFFSET 24
#define AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_FIELD_SIZE 8

#define AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_FIELD_OFFSET 16
#define AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_FIELD_SIZE 8

#define AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_FIELD_OFFSET 8
#define AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_FIELD_SIZE 8

#define AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_FIELD_SIZE 8


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL
// Field Width     : 8
// Field Start Bit : 24
// Field End Bit   : 31
// Description     : backgound alpha value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_7    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_6    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_5    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_4    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_3    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_2    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_1    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_A_VAL_0    (BIT_(24))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL
// Field Width     : 8
// Field Start Bit : 16
// Field End Bit   : 23
// Description     : background blue value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_7    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_B_VAL_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL
// Field Width     : 8
// Field Start Bit : 8
// Field End Bit   : 15
// Description     : background green value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_7    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_6    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_5    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_4    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_3    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_2    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_1    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_G_VAL_0    (BIT_(8))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL
// Field Width     : 8
// Field Start Bit : 0
// Field End Bit   : 7
// Description     : backgound red value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DES_BG_COLOR_R_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA
// Register Offset : 0x90
// Description     : ASW external bilinear map LUT R.x(Y.x) base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA (AHB_GAMA_ASW_BASE_ADDR + (0x90<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA
// Register Offset : 0x94
// Description     : ASW external bilinear map LUT R.y(Y.y) base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA (AHB_GAMA_ASW_BASE_ADDR + (0x94<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA
// Register Offset : 0x98
// Description     : ASW external bilinear map LUT G.x(U.x) base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA (AHB_GAMA_ASW_BASE_ADDR + (0x98<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA
// Register Offset : 0x9c
// Description     : ASW external bilinear map LUT G.y(U.y) base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA (AHB_GAMA_ASW_BASE_ADDR + (0x9c<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA
// Register Offset : 0xa0
// Description     : ASW external bilinear map LUT B.x(V.x) base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA (AHB_GAMA_ASW_BASE_ADDR + (0xa0<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA
// Register Offset : 0xa4
// Description     : ASW external bilinear map LUT B.y(V.y) base address register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA (AHB_GAMA_ASW_BASE_ADDR + (0xa4<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : address setting value
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_BA_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE
// Register Offset : 0xa8
// Description     : ASW external bilinear map LUT R.x(Y.x) stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0xa8<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RX_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE
// Register Offset : 0xac
// Description     : ASW external bilinear map LUT R.y(Y.y) stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0xac<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_RY_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE
// Register Offset : 0xb0
// Description     : ASW external bilinear map LUT G.x(U.x) stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0xb0<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GX_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE
// Register Offset : 0xb4
// Description     : ASW external bilinear map LUT G.y(U.y) stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0xb4<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_GY_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE
// Register Offset : 0xb8
// Description     : ASW external bilinear map LUT B.x(V.x) stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0xb8<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BX_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE
// Register Offset : 0xbc
// Description     : ASW external bilinear map LUT B.y(V.y) stride register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE (AHB_GAMA_ASW_BASE_ADDR + (0xbc<<0))

#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : stride setting value(signed), =stride-1
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_EXT_MLUT_BY_STRIDE_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE
// Register Offset : 0xe0
// Description     : ASW abnormal handle register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE (AHB_GAMA_ASW_BASE_ADDR + (0xe0<<0))

#define AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_LB_LUT_TYPE_FIELD_OFFSET 2
#define AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_LB_LUT_TYPE_FIELD_SIZE 2

#define AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_RECOVERY_MODE_FIELD_OFFSET 1
#define AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_RECOVERY_MODE_FIELD_SIZE 1

#define AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_LN_UPDATE_EN_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_LN_UPDATE_EN_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_LB_LUT_TYPE
// Field Width     : 2
// Field Start Bit : 2
// Field End Bit   : 3
// Description     : 0=determined by source format
//                   1=one-component-LUT
//                   2=two-components-LUT
//                   3=three-components-LUT
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_LB_LUT_TYPE_1    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_LB_LUT_TYPE_0    (BIT_(2))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_RECOVERY_MODE
// Field Width     : 1
// Field Start Bit : 1
// Field End Bit   : 1
// Description     : when new frame-start detected and ASW is still busy processing the previous frame,
//                   0=ASW wait until processing the previous frame finished, then start new frame
//                   1=ASW is immediately reset and start new frame
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_RECOVERY_MODE    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_LN_UPDATE_EN
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : now as reserved
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_ABNORM_HANDLE_LN_UPDATE_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG
// Register Offset : 0xe4
// Description     : ASW JUMP to local buffer's program register(when external cubic spline mode)
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG (AHB_GAMA_ASW_BASE_ADDR + (0xe4<<0))

#define AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_FIELD_SIZE 24


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL
// Field Width     : 24
// Field Start Bit : 0
// Field End Bit   : 23
// Description     : assembler program's base address in local buffer
//                   (active when ASW_PARA_SET.LUT_INTERP_MODE=1 and ASW_PARA_SET.EXT_CSPLINE_EN=1)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_JP_LB_PROG_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW
// Register Offset : 0xf0
// Description     : ASW cache load peak bandwidth register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW (AHB_GAMA_ASW_BASE_ADDR + (0xf0<<0))

#define AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : max(cache load block number per line)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_PEAK_BW_RVAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW
// Register Offset : 0xf4
// Description     : ASW cache load total bandwidth register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW (AHB_GAMA_ASW_BASE_ADDR + (0xf4<<0))

#define AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : cache load block number per frame
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_CLOAD_TOTAL_BW_RVAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_POS_ST
// Register Offset : 0xf8
// Description     : ASW position calc status register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_POS_ST (AHB_GAMA_ASW_BASE_ADDR + (0xf8<<0))

#define AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_POS_ST_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : [31:24] = stepv_cnt (active when ASW_PARA_SET.LUT_INTERP_MODE=1)
//                   [23:16] = segv_cnt (active when ASW_PARA_SET.LUT_INTERP_MODE=1)
//                   [15:12] = comp_cnt
//                   [11:0] = line_cnt
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_POS_ST_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST
// Register Offset : 0xfc
// Description     : ASW filter result output status register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST (AHB_GAMA_ASW_BASE_ADDR + (0xfc<<0))

#define AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : [31:16] = pix_cnt (before pack)
//                   [15:0] = line_cnt
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_FR_OUT_ST_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0
// Register Offset : 0x100
// Description     : ASW POS debug status 0 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0 (AHB_GAMA_ASW_BASE_ADDR + (0x100<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {2'b00,cr_comp_cnt, //31-28
//                    1'b0,cr_pos_buf_line_cnt, //27-24
//                    1'b0,cs_pos_buf_line_cnt, //23-20
//                    1'b0,pos_buf_line_cnt, //19-16
//                    3'b000,code_cnt, //15-4
//                    wpos_fsm_cur} //3-0(IDLE-0,LD_GP-1,LN_WAIT-2,LUT_H-3,M_H-4,H_ABCD-5,EXT_REQ-9,LN_DONE-15)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1
// Register Offset : 0x104
// Description     : ASW POS debug status 1 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1 (AHB_GAMA_ASW_BASE_ADDR + (0x104<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {asw_rst_flg,rdma_accept_flg,1'b0,i_ext_mlut_svalid, //31-28
//                    1'b0,lut_pop_cur, //27-24(POP_IDLE-0,POP_BL_HEAD-1,POP_NORM-2,INTERP-3,LN_FLUSH-4)
//                    draw_buf_rpt,draw_buf_wpt,draw_buf_slot_cnt, //23-20
//                    2'b00,pos_buf_cr_rp, //19-16
//                    2'b00,pos_buf_cs_rp, //15-12
//                    2'b00,prev_y_slot_rpt, //11-8
//                    2'b00,pos_r_slot_wpt, //7-4
//                    2'b00,i_asw_iq_rdy,o_asw_iq_vld} //3-0
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST1_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2
// Register Offset : 0x108
// Description     : ASW POS debug status 2 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2 (AHB_GAMA_ASW_BASE_ADDR + (0x108<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : o_asw_iq
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST2_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3
// Register Offset : 0x10c
// Description     : ASW POS debug status 3 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3 (AHB_GAMA_ASW_BASE_ADDR + (0x10c<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {2'b00,ext_mlut_req_vl_cnt[9:0],
//                    ext_mlut_req_vs_cnt[11:0],
//                    1'b0,ext_mlut_req_hcnt[6:0]}
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST3_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4
// Register Offset : 0x110
// Description     : ASW POS debug status 4 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4 (AHB_GAMA_ASW_BASE_ADDR + (0x110<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {1'b0,intp_comp_cnt[2:0],
//                    intp_vcnt[11:0],
//                    1'b0,wp_plane_cnt[2:0],
//                    2'b00,lut_pop_cnt[9:0]}
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST4_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5
// Register Offset : 0x114
// Description     : ASW POS debug status 5 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5 (AHB_GAMA_ASW_BASE_ADDR + (0x114<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {4'h0,intp_hcnt[11:0],
//                    4'h0,wp_hcnt[11:0]}
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_POS_ST5_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0
// Register Offset : 0x118
// Description     : ASW DBC debug status 0 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0 (AHB_GAMA_ASW_BASE_ADDR + (0x118<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {1'b0,ctb_fsm_cur[2:0], //31-28(IDLE-0,RD_YC0-1,RD_YC1-2,YC_OUT-3,TST_UP-4,YC_RDY-5,YC_WB0-6,YC_WB1-7)
//                    1'b0,cread_fsm_cur[2:0], //27-24(IDLE-0,RY-1,GU_W-2,GU-3,BV_W-4,BV-5,A_W-6,A-7)
//                    cache3_sets_aval,cache2_sets_aval,cache1_sets_aval,cache0_sets_aval, //23-20
//                    1'b0,cache2_sets_busy,cache1_sets_busy,cache0_sets_busy, //19-16
//                    1'b0,cload_fsm_cur[2:0], //15-12
//                    1'b0,cache_tags2_aval,cache_tags1_aval,cache_tags0_aval, //11-8
//                    2'b00,cache_tags_rpt[1:0], //7-4
//                    1'b0,c_up_fsm_cur[2:0]} //3-0(IDLE-0,RY_UP-1,GU_W-2,GU_UP-3,BV_W-4,BV_UP-5)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1
// Register Offset : 0x11c
// Description     : ASW DBC debug status 1 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1 (AHB_GAMA_ASW_BASE_ADDR + (0x11c<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {i_cs_pos_y[11:0],
//                    i_cs_pos_x[11:0],
//                    4'h0,
//                    1'b0,i_cs_pos_line_end,o_cs_pos_rdy,i_cs_pos_vld}
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST1_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2
// Register Offset : 0x120
// Description     : ASW DBC debug status 2 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2 (AHB_GAMA_ASW_BASE_ADDR + (0x120<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {i_cr_pos_y[11:0],
//                    i_cr_pos_x[11:0],
//                    1'b0,i_asw_fr_lnbuf_full,i_filter_parms_rdy,o_filter_parms_vld,
//                    i_cr_pos_line_end,i_cr_pos_line_start,o_cr_pos_rdy,i_cr_pos_vld}
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST2_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3
// Register Offset : 0x124
// Description     : ASW DBC debug status 3 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3 (AHB_GAMA_ASW_BASE_ADDR + (0x124<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {filter_h_cnt[11:0], //
//                    1'b0,tile_vcnt[2:0], //19-16
//                    3'b000,cload_set_cnt[8:0], //15-4
//                    cld_fsm_cur} //3-0(IDLE-0,TS_CHK-1,YC_OUT-2,REQ_W0~9-3~12,FAKE_REQ-15)
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_DBC_ST3_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0
// Register Offset : 0x128
// Description     : ASW LBC debug status 0 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0 (AHB_GAMA_ASW_BASE_ADDR + (0x128<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {2'b00,fr_cw_line_cnt[1:0], //31-28
//                    fr_cw_comp_pt, //27-24
//                    fr_cwreq_cnt[7:0], //23-16
//                    2'b00,cl_line_done,prev_cl_vld, 15-12
//                    planar_stamp1[1:0],planar_stamp0[1:0], //11-8
//                    unpack_force_ln_done,unpack_ln_done,cl_rdma_seg_rdy,cl_nxt_vld, //7-4
//                    i_cload_scmd[1:0],o_cload_sready,i_cload_svalid} //3-0
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1
// Register Offset : 0x12c
// Description     : ASW LBC debug status 1 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1 (AHB_GAMA_ASW_BASE_ADDR + (0x12c<<0))

#define AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {{14{1'b0}},o_to_dc_seg_done,i_dc_seg_buf_rdy, //31-16
//                    3'b000,fr_cr_buf_pty, //15-12
//                    2'b00,fr_cr_comp_pt[1:0], //11-8
//                    fr_crreq_cnt[7:0]} //7-0
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_ASW_ASW_DBG_LBC_ST1_VAL_0    (BIT_(0))

#endif
