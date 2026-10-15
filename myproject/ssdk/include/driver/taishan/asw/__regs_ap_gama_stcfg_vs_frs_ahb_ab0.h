#ifndef _AP_AHB_GAMA_STCFG_VS_FRS_AB0_REG_H
#define _AP_AHB_GAMA_STCFG_VS_FRS_AB0_REG_H

//--------------------------------------------------------------------------
// Component          : semidrive.com, d_ip_gama, gama_stcfg_vs_frs, r0p0
// Memory Map         : AHB
// Address Block Name : AB0
// Description        : Address Block 0 of gama_stcfg_vs_frs
//--------------------------------------------------------------------------
#define AHB_GAMA_STCFG_VS_FRS_BASE_ADDR 0x0


//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS
// Register Offset : 0x4
// Description     : floating point accrued exception.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_NV_FIELD_OFFSET 4
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_NV_FIELD_SIZE 1

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_DZ_FIELD_OFFSET 3
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_DZ_FIELD_SIZE 1

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_OF_FIELD_OFFSET 2
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_OF_FIELD_SIZE 1

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_UF_FIELD_OFFSET 1
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_UF_FIELD_SIZE 1

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_NX_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_NX_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_NV
// Field Width     : 1
// Field Start Bit : 4
// Field End Bit   : 4
// Description     : invalid operation.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_NV    (BIT_(4))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_DZ
// Field Width     : 1
// Field Start Bit : 3
// Field End Bit   : 3
// Description     : divide by zero.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_DZ    (BIT_(3))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_OF
// Field Width     : 1
// Field Start Bit : 2
// Field End Bit   : 2
// Description     : overflow
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_OF    (BIT_(2))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_UF
// Field Width     : 1
// Field Start Bit : 1
// Field End Bit   : 1
// Description     : Underflow
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_UF    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_NX
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : Inexact
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FFLAGS_NX    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FRM
// Register Offset : 0x8
// Description     : floating point dynamic rounding mode .
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FRM (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FRM_FRM_FIELD_OFFSET 5
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FRM_FRM_FIELD_SIZE 3


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FRM_FRM
// Field Width     : 3
// Field Start Bit : 5
// Field End Bit   : 7
// Description     : 000: RNE, Round to Nearest, ties to Even.
//                   001: RTZ , Round towards Zero
//                   010: RDN ,Round Down(Towards -∞)
//                   011: RUP, Round Up ( towards + ∞)
//                   100: RMM, Round to nearest, ties to Max magnitude.
//                   101: Invalid.
//                   110: Invalid:
//                   111: In instruction's rm field , select dynamic rounding mode; In Rounding Mode register, Invalid.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FRM_FRM_2    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FRM_FRM_1    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_FRM_FRM_0    (BIT_(5))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR
// Register Offset : 0xc
// Description     : floating point control and status register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0xc<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_FRM_FIELD_OFFSET 5
#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_FRM_FIELD_SIZE 3

#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_NV_FIELD_OFFSET 4
#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_NV_FIELD_SIZE 1

#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_DZ_FIELD_OFFSET 3
#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_DZ_FIELD_SIZE 1

#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_OF_FIELD_OFFSET 2
#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_OF_FIELD_SIZE 1

#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_UF_FIELD_OFFSET 1
#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_UF_FIELD_SIZE 1

#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_NX_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_NX_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_FRM
// Field Width     : 3
// Field Start Bit : 5
// Field End Bit   : 7
// Description     : 000: RNE, Round to Nearest, ties to Even.
//                   001: RTZ , Round towards Zero
//                   010: RDN ,Round Down(Towards -∞)
//                   011: RUP, Round Up ( towards + ∞)
//                   100: RMM, Round to nearest, ties to Max magnitude.
//                   101: Invalid.
//                   110: Invalid:
//                   111: In instruction's rm field , select dynamic rounding mode; In Rounding Mode register, Invalid.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_FRM_2    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_FRM_1    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_FRM_0    (BIT_(5))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_NV
// Field Width     : 1
// Field Start Bit : 4
// Field End Bit   : 4
// Description     : Invalid Operation.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_NV    (BIT_(4))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_DZ
// Field Width     : 1
// Field Start Bit : 3
// Field End Bit   : 3
// Description     : Divide by zero.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_DZ    (BIT_(3))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_OF
// Field Width     : 1
// Field Start Bit : 2
// Field End Bit   : 2
// Description     : Overflow.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_OF    (BIT_(2))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_UF
// Field Width     : 1
// Field Start Bit : 1
// Field End Bit   : 1
// Description     : Underflow
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_UF    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_NX
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : Inexact
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMAM_CFG_FCSR_NX    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART
// Register Offset : 0x20
// Description     : Vector start position
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x20<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : vector strart position.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_VSTART_ADDR_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL
// Register Offset : 0x3080
// Description     : vl
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x3080<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : Vector length.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VL_VL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE
// Register Offset : 0x3084
// Description     : Vector data type register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x3084<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_FIELD_OFFSET 6
#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_FIELD_SIZE 26

#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VSEW_FIELD_OFFSET 3
#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VSEW_FIELD_SIZE 3

#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VLMUL_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VLMUL_FIELD_SIZE 3


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES
// Field Width     : 26
// Field Start Bit : 6
// Field End Bit   : 31
// Description     : Reserved .
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_25    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_24    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_23    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_22    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_21    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_20    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_19    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_18    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_17    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_16    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_15    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_14    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_13    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_12    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_11    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_10    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_9    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_8    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_7    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_6    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_5    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_4    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_3    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_2    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_1    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_RES_0    (BIT_(6))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VSEW
// Field Width     : 3
// Field Start Bit : 3
// Field End Bit   : 5
// Description     : Selected element width setting.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VSEW_2    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VSEW_1    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VSEW_0    (BIT_(3))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VLMUL
// Field Width     : 3
// Field Start Bit : 0
// Field End Bit   : 2
// Description     : Vector register group multiplier setting.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VLMUL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VLMUL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VTYPE_VLMUL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB
// Register Offset : 0x3088
// Description     : Vlenb register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x3088<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB_LENB_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB_LENB_FIELD_SIZE 5


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB_LENB
// Field Width     : 5
// Field Start Bit : 0
// Field End Bit   : 4
// Description     : VLEN/8
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB_LENB_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB_LENB_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB_LENB_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB_LENB_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GMA_CFG_VLENB_LENB_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE
// Register Offset : 0x3100
// Description     : Msize.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x3100<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_HSIZE_FIELD_OFFSET 6
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_HSIZE_FIELD_SIZE 6

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_VSIZE_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_VSIZE_FIELD_SIZE 6


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_HSIZE
// Field Width     : 6
// Field Start Bit : 6
// Field End Bit   : 11
// Description     : Row size.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_HSIZE_5    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_HSIZE_4    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_HSIZE_3    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_HSIZE_2    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_HSIZE_1    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_HSIZE_0    (BIT_(6))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_VSIZE
// Field Width     : 6
// Field Start Bit : 0
// Field End Bit   : 5
// Description     : column size
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_VSIZE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_VSIZE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_VSIZE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_VSIZE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_VSIZE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_MSIZE_VSIZE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP
// Register Offset : 0x3104
// Description     : gama stack pointer.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x3104<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : Stack pointer addr.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_CFG_SP_SP_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS
// Register Offset : 0x10000
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10000<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1
// Register Offset : 0x10004
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10004<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_1_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2
// Register Offset : 0x10008
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10008<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_2_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3
// Register Offset : 0x1000c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1000c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_3_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4
// Register Offset : 0x10010
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10010<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_4_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5
// Register Offset : 0x10014
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10014<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_5_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6
// Register Offset : 0x10018
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10018<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_6_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7
// Register Offset : 0x1001c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1001c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_7_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8
// Register Offset : 0x10020
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10020<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_8_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9
// Register Offset : 0x10024
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10024<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_9_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10
// Register Offset : 0x10028
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10028<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_10_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11
// Register Offset : 0x1002c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1002c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_11_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12
// Register Offset : 0x10030
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10030<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_12_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13
// Register Offset : 0x10034
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10034<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_13_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14
// Register Offset : 0x10038
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10038<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_14_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15
// Register Offset : 0x1003c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1003c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_15_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16
// Register Offset : 0x10040
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10040<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_16_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17
// Register Offset : 0x10044
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10044<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_17_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18
// Register Offset : 0x10048
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10048<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_18_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19
// Register Offset : 0x1004c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1004c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_19_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20
// Register Offset : 0x10050
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10050<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_20_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21
// Register Offset : 0x10054
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10054<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_21_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22
// Register Offset : 0x10058
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10058<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_22_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23
// Register Offset : 0x1005c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1005c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_23_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24
// Register Offset : 0x10060
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10060<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_24_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25
// Register Offset : 0x10064
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10064<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_25_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26
// Register Offset : 0x10068
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10068<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_26_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27
// Register Offset : 0x1006c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1006c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_27_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28
// Register Offset : 0x10070
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10070<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_28_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29
// Register Offset : 0x10074
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10074<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_29_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30
// Register Offset : 0x10078
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10078<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_30_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31
// Register Offset : 0x1007c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1007c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_31_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32
// Register Offset : 0x10080
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10080<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_32_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33
// Register Offset : 0x10084
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10084<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_33_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34
// Register Offset : 0x10088
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10088<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_34_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35
// Register Offset : 0x1008c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1008c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_35_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36
// Register Offset : 0x10090
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10090<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_36_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37
// Register Offset : 0x10094
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10094<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_37_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38
// Register Offset : 0x10098
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10098<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_38_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39
// Register Offset : 0x1009c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1009c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_39_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40
// Register Offset : 0x100a0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100a0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_40_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41
// Register Offset : 0x100a4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100a4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_41_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42
// Register Offset : 0x100a8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100a8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_42_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43
// Register Offset : 0x100ac
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100ac<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_43_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44
// Register Offset : 0x100b0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100b0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_44_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45
// Register Offset : 0x100b4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100b4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_45_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46
// Register Offset : 0x100b8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100b8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_46_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47
// Register Offset : 0x100bc
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100bc<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_47_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48
// Register Offset : 0x100c0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100c0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_48_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49
// Register Offset : 0x100c4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100c4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_49_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50
// Register Offset : 0x100c8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100c8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_50_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51
// Register Offset : 0x100cc
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100cc<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_51_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52
// Register Offset : 0x100d0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100d0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_52_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53
// Register Offset : 0x100d4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100d4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_53_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54
// Register Offset : 0x100d8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100d8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_54_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55
// Register Offset : 0x100dc
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100dc<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_55_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56
// Register Offset : 0x100e0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100e0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_56_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57
// Register Offset : 0x100e4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100e4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_57_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58
// Register Offset : 0x100e8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100e8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_58_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59
// Register Offset : 0x100ec
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100ec<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_59_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60
// Register Offset : 0x100f0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100f0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_60_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61
// Register Offset : 0x100f4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100f4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_61_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62
// Register Offset : 0x100f8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100f8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_62_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63
// Register Offset : 0x100fc
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x100fc<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_63_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64
// Register Offset : 0x10100
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10100<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_64_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65
// Register Offset : 0x10104
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10104<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_65_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66
// Register Offset : 0x10108
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10108<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_66_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67
// Register Offset : 0x1010c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1010c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_67_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68
// Register Offset : 0x10110
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10110<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_68_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69
// Register Offset : 0x10114
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10114<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_69_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70
// Register Offset : 0x10118
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10118<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_70_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71
// Register Offset : 0x1011c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1011c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_71_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72
// Register Offset : 0x10120
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10120<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_72_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73
// Register Offset : 0x10124
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10124<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_73_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74
// Register Offset : 0x10128
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10128<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_74_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75
// Register Offset : 0x1012c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1012c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_75_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76
// Register Offset : 0x10130
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10130<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_76_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77
// Register Offset : 0x10134
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10134<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_77_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78
// Register Offset : 0x10138
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10138<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_78_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79
// Register Offset : 0x1013c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1013c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_79_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80
// Register Offset : 0x10140
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10140<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_80_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81
// Register Offset : 0x10144
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10144<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_81_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82
// Register Offset : 0x10148
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10148<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_82_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83
// Register Offset : 0x1014c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1014c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_83_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84
// Register Offset : 0x10150
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10150<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_84_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85
// Register Offset : 0x10154
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10154<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_85_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86
// Register Offset : 0x10158
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10158<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_86_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87
// Register Offset : 0x1015c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1015c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_87_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88
// Register Offset : 0x10160
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10160<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_88_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89
// Register Offset : 0x10164
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10164<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_89_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90
// Register Offset : 0x10168
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10168<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_90_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91
// Register Offset : 0x1016c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1016c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_91_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92
// Register Offset : 0x10170
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10170<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_92_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93
// Register Offset : 0x10174
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10174<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_93_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94
// Register Offset : 0x10178
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10178<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_94_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95
// Register Offset : 0x1017c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1017c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_95_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96
// Register Offset : 0x10180
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10180<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_96_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97
// Register Offset : 0x10184
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10184<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_97_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98
// Register Offset : 0x10188
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10188<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_98_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99
// Register Offset : 0x1018c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1018c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_99_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100
// Register Offset : 0x10190
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10190<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_100_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101
// Register Offset : 0x10194
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10194<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_101_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102
// Register Offset : 0x10198
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x10198<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_102_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103
// Register Offset : 0x1019c
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1019c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_103_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104
// Register Offset : 0x101a0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101a0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_104_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105
// Register Offset : 0x101a4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101a4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_105_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106
// Register Offset : 0x101a8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101a8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_106_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107
// Register Offset : 0x101ac
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101ac<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_107_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108
// Register Offset : 0x101b0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101b0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_108_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109
// Register Offset : 0x101b4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101b4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_109_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110
// Register Offset : 0x101b8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101b8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_110_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111
// Register Offset : 0x101bc
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101bc<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_111_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112
// Register Offset : 0x101c0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101c0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_112_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113
// Register Offset : 0x101c4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101c4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_113_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114
// Register Offset : 0x101c8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101c8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_114_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115
// Register Offset : 0x101cc
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101cc<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_115_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116
// Register Offset : 0x101d0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101d0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_116_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117
// Register Offset : 0x101d4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101d4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_117_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118
// Register Offset : 0x101d8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101d8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_118_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119
// Register Offset : 0x101dc
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101dc<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_119_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120
// Register Offset : 0x101e0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101e0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_120_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121
// Register Offset : 0x101e4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101e4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_121_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122
// Register Offset : 0x101e8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101e8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_122_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123
// Register Offset : 0x101ec
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101ec<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_123_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124
// Register Offset : 0x101f0
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101f0<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_124_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125
// Register Offset : 0x101f4
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101f4<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_125_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126
// Register Offset : 0x101f8
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101f8<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_126_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127
// Register Offset : 0x101fc
// Description     : VS register.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x101fc<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : VS register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_VS_127_REG_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS
// Register Offset : 0x11000
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11000<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1
// Register Offset : 0x11004
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11004<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_1_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2
// Register Offset : 0x11008
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11008<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_2_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3
// Register Offset : 0x1100c
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1100c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_3_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4
// Register Offset : 0x11010
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11010<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_4_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5
// Register Offset : 0x11014
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11014<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_5_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6
// Register Offset : 0x11018
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11018<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_6_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7
// Register Offset : 0x1101c
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1101c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_7_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8
// Register Offset : 0x11020
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11020<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_8_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9
// Register Offset : 0x11024
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11024<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_9_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10
// Register Offset : 0x11028
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11028<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_10_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11
// Register Offset : 0x1102c
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1102c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_11_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12
// Register Offset : 0x11030
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11030<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_12_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13
// Register Offset : 0x11034
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11034<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_13_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14
// Register Offset : 0x11038
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11038<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_14_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15
// Register Offset : 0x1103c
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1103c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_15_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16
// Register Offset : 0x11040
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11040<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_16_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17
// Register Offset : 0x11044
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11044<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_17_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18
// Register Offset : 0x11048
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11048<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_18_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19
// Register Offset : 0x1104c
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1104c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_19_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20
// Register Offset : 0x11050
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11050<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_20_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21
// Register Offset : 0x11054
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11054<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_21_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22
// Register Offset : 0x11058
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11058<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_22_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23
// Register Offset : 0x1105c
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1105c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_23_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24
// Register Offset : 0x11060
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11060<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_24_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25
// Register Offset : 0x11064
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11064<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_25_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26
// Register Offset : 0x11068
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11068<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_26_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27
// Register Offset : 0x1106c
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1106c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_27_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28
// Register Offset : 0x11070
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11070<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_28_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29
// Register Offset : 0x11074
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11074<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_29_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30
// Register Offset : 0x11078
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x11078<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_30_REG_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31
// Register Offset : 0x1107c
// Description     : GAMA_FRS
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31 (AHB_GAMA_STCFG_VS_FRS_BASE_ADDR + (0x1107c<<0))

#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_FIELD_OFFSET 0
#define AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : float register.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_STCFG_VS_FRS_GAMA_FRS_31_REG_0    (BIT_(0))

#endif
