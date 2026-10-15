#ifndef _AP_AHB_GAMA_IRQST_AB1_REG_H
#define _AP_AHB_GAMA_IRQST_AB1_REG_H

//--------------------------------------------------------------------------
// Component          : semidrive.com, d_ip_gama, gama_irqst, r0p0
// Memory Map         : AHB
// Address Block Name : AB1
// Description        : Address Block 1 of gama_irqst
//--------------------------------------------------------------------------
#define AHB_GAMA_IRQST_AB1_BASE_ADDR 0x1000


//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0
// Register Offset : 0x0
// Description     : GAMA interrupt raw status register of vector 0
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x0<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_FIELD_SIZE 17


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL
// Field Width     : 17
// Field Start Bit : 0
// Field End Bit   : 16
// Description     : bit16 = AHB LB R/W request time-out happen
//                   bit15 = DMA read-from-LB request time-out happen
//                   bit14 = DMA write-to-LB request time-out happen
//                   bit13 = whether sync id counter value is not 0
//                   bit12 = stall instruction appeared
//                   bit11 = EPU undefined intruction appeared
//                   bit10 = undefined self-defined-DMA instruction appeared
//                   bit9 = undefined config-type instruction appeared
//                   bit8 = undefined load-store-type intruction appeared
//                   bit7 = undefined vector-arithmetic-type instruction appeared
//                   bit6 = illegal vector-arithmetic-type intrucition appeared (wrong LMUL/SEW)
//                   bit5 = IDEC instruction crashed, arithmetic instruction appeared and become another intruction before MAC accepted it
//                   bit4 = arithmetic exception invalid flag detected
//                   bit3 = arithmetic exception divided-by-0 flag detected
//                   bit2 = arithmetic exception overflow flag detected
//                   bit1 = arithmetic exception underflow flag detected
//                   bit0 = arithmetic exception inexact flag detected
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0
// Register Offset : 0x4
// Description     : GAMA interrupt status register of vector 0
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x4<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_FIELD_SIZE 17


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL
// Field Width     : 17
// Field Start Bit : 0
// Field End Bit   : 16
// Description     : interrupt status, refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0
// Register Offset : 0x8
// Description     : GAMA interrupt mask register of vector 0
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x8<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_FIELD_SIZE 17


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL
// Field Width     : 17
// Field Start Bit : 0
// Field End Bit   : 16
// Description     : 1 = mask
//                   refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0
// Register Offset : 0xc
// Description     : GAMA interrupt polarity register of vector 0
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0xc<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_FIELD_SIZE 17


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL
// Field Width     : 17
// Field Start Bit : 0
// Field End Bit   : 16
// Description     : 0=posedge trigger,1=negedge
//                   refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0
// Register Offset : 0x10
// Description     : GAMA interrupt clear register of vector 0
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x10<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_FIELD_SIZE 17


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL
// Field Width     : 17
// Field Start Bit : 0
// Field End Bit   : 16
// Description     : refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0
// Register Offset : 0x14
// Description     : GAMA interrupt set register of vector 0
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x14<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_FIELD_SIZE 17


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL
// Field Width     : 17
// Field Start Bit : 0
// Field End Bit   : 16
// Description     : refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1
// Register Offset : 0x20
// Description     : GAMA interrupt raw status register of vector 1
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x20<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : bit11 = last line lost of i_csi_line_cnt when CSI handshake enable
//                   bit10 = first line lost of i_csi_line_cnt when CSI handshake enable only mode
//                   bit9 = filter results output frame done
//                   bit8 = when DC handshake enable and a new frame start from DC coming, ASW-DC buffer not-empty is detected
//                   bit7 = when DC handshake enable, DONE from DC is not fit with ASW-DC buffer read pointer
//                   bit6 = for new frame start coming too fast, it is discarded by ASW
//                   bit5 = unsupported source/destination image format was configured
//                   bit4 = ASW cache base address setting not met requirement
//                   bit3 = ASW adress parameters shadow load done
//                   bit2 = ASW cfg parameters except for adress messages shadow load done
//                   bit1 = ASW frame start  has already triggered
//                   bit0 = ASW filter results write-out-DMA frame done
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_RAW_ST_1_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1
// Register Offset : 0x24
// Description     : GAMA interrupt status register of vector 1
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x24<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : interrupt status, refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_ST_1_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1
// Register Offset : 0x28
// Description     : GAMA interrupt mask register of vector 1
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x28<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : 1 = mask
//                   refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_MSK_1_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1
// Register Offset : 0x2c
// Description     : GAMA interrupt polarity register of vector 1
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x2c<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : 0=posedge trigger,1=negedge
//                   refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_PTY_1_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1
// Register Offset : 0x30
// Description     : GAMA interrupt clear register of vector 1
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x30<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_CLR_1_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1
// Register Offset : 0x34
// Description     : GAMA interrupt set register of vector 1
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1 (AHB_GAMA_IRQST_AB1_BASE_ADDR + (0x34<<0))

#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_FIELD_SIZE 12


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL
// Field Width     : 12
// Field Start Bit : 0
// Field End Bit   : 11
// Description     : refer to corresponding comments in raw status register
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_AB1_GAMA_INT_SET_1_VAL_0    (BIT_(0))

#endif
