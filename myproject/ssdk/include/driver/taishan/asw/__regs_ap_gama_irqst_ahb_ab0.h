#ifndef _AP_AHB_GAMA_IRQST_AB0_REG_H
#define _AP_AHB_GAMA_IRQST_AB0_REG_H

//--------------------------------------------------------------------------
// Component          : semidrive.com, d_ip_gama, gama_irqst, r0p0
// Memory Map         : AHB
// Address Block Name : AB0
// Description        : Address Block 0 of gama_irqst
//--------------------------------------------------------------------------
#define AHB_GAMA_IRQST_BASE_ADDR 0x0


//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS
// Register Offset : 0x0
// Description     : EPU sync status
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS (AHB_GAMA_IRQST_BASE_ADDR + (0x0<<0))

#define AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_FIELD_OFFSET 16
#define AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_FIELD_SIZE 16

#define AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_FIELD_SIZE 8


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT
// Field Width     : 16
// Field Start Bit : 16
// Field End Bit   : 31
// Description     : The  instruction ID equals  Sync instruction  SYCN ID, the numuber of instructions in the IB and PIPLINE.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_15    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_14    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_13    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_12    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_11    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_10    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_9    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_8    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_7    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_CNT_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID
// Field Width     : 8
// Field Start Bit : 0
// Field End Bit   : 7
// Description     : Sync ID  for  the Sync instruction
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_EPU_SYNC_STATUS_SYNC_ID_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_EPU_STALL_STATUS
// Register Offset : 0x4
// Description     : EPU stall instruction return status.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_EPU_STALL_STATUS (AHB_GAMA_IRQST_BASE_ADDR + (0x4<<0))

#define AP_AHB_GAMA_IRQST_EPU_STALL_STATUS_STALL_FLAG_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_EPU_STALL_STATUS_STALL_FLAG_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_EPU_STALL_STATUS_STALL_FLAG
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : When Stall instruction, this bit is set to 1,
//                   When Resum instruction , this bits is cleared to 0.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_EPU_STALL_STATUS_STALL_FLAG    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_HW_MODE
// Register Offset : 0x10
// Description     : GAMA hardware mode register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_HW_MODE (AHB_GAMA_IRQST_BASE_ADDR + (0x10<<0))

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_FIELD_OFFSET 26
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_FIELD_SIZE 6

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_FIELD_OFFSET 20
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_FIELD_SIZE 6

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_FIELD_OFFSET 14
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_FIELD_SIZE 6

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_LB_REQ_PRIORITY_FIELD_OFFSET 12
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_LB_REQ_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_EN_FIELD_OFFSET 11
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_EN_FIELD_SIZE 1

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_EN_FIELD_OFFSET 10
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_EN_FIELD_SIZE 1

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_EN_FIELD_OFFSET 9
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_EN_FIELD_SIZE 1

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_ASW_EN_FIELD_OFFSET 2
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_ASW_EN_FIELD_SIZE 1

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_LB_LAYOUT_TYPE_FIELD_OFFSET 1
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_LB_LAYOUT_TYPE_FIELD_SIZE 1

#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_EPU_CTRL_MODE_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_HW_MODE_EPU_CTRL_MODE_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO
// Field Width     : 6
// Field Start Bit : 26
// Field End Bit   : 31
// Description     : DMA write-to-LB request time-out time setting
//                   =(setting value+1)*16cycles
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_5    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_4    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_3    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_2    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_1    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_0    (BIT_(26))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO
// Field Width     : 6
// Field Start Bit : 20
// Field End Bit   : 25
// Description     : DMA read-from-LB request time-out time setting
//                   =(setting value+1)*16cycles
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_5    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_4    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO
// Field Width     : 6
// Field Start Bit : 14
// Field End Bit   : 19
// Description     : AHB r/w LB request time-out time setting
//                   =(setting value+1)*16cycles
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_5    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_4    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_3    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_2    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_1    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_0    (BIT_(14))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_LB_REQ_PRIORITY
// Field Width     : 2
// Field Start Bit : 12
// Field End Bit   : 13
// Description     : the rightmost has the highest priority
//                   0={prefetch rd,dma rd,dma wr,asw results rd,asw results wr,STORE wr,LOAD rd,AHB rd,AHB wr,cache write,cache read}
//                   1={dma rd,dma wr,asw results rd,asw results wr,prefetch rd,STORE wr,LOAD rd,cache write,cache read,AHB rd,AHB wr}
//                   2={asw results rd,asw results wr,cache write,cache read,dma rd,dma wr,prefetch rd,STORE wr,LOAD rd,AHB rd,AHB wr}
//                   3={asw results rd,asw results wr,cache write,cache read,STORE wr,LOAD rd,prefetch rd,dma rd,dma wr,AHB rd,AHB wr}
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_LB_REQ_PRIORITY_1    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_LB_REQ_PRIORITY_0    (BIT_(12))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_EN
// Field Width     : 1
// Field Start Bit : 11
// Field End Bit   : 11
// Description     : DMA write-to-LB request time-out detection enable
//                   1=enable
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_WR_LB_TO_EN    (BIT_(11))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_EN
// Field Width     : 1
// Field Start Bit : 10
// Field End Bit   : 10
// Description     : DMA read-from-LB request time-out detection enable
//                   1=enable
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_DMA_RD_LB_TO_EN    (BIT_(10))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_EN
// Field Width     : 1
// Field Start Bit : 9
// Field End Bit   : 9
// Description     : AHB r/w LB request time-out detection enable
//                   1=enable
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_AHB_RW_LB_TO_EN    (BIT_(9))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_ASW_EN
// Field Width     : 1
// Field Start Bit : 2
// Field End Bit   : 2
// Description     : 1=IQ/IDEC only for ASW
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_ASW_EN    (BIT_(2))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_LB_LAYOUT_TYPE
// Field Width     : 1
// Field Start Bit : 1
// Field End Bit   : 1
// Description     : 0=layout A
//                   1=layout B
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_LB_LAYOUT_TYPE    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_HW_MODE_EPU_CTRL_MODE
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : 0=decoder I debug mode
//                   1=decoder I normal mode
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_HW_MODE_EPU_CTRL_MODE    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_DSYNC_STATUS0
// Register Offset : 0x20
// Description     : DSYNC intruction status.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_DSYNC_STATUS0 (AHB_GAMA_IRQST_BASE_ADDR + (0x20<<0))

#define AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM_FIELD_OFFSET 16
#define AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM_FIELD_SIZE 7

#define AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM_FIELD_SIZE 7


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM
// Field Width     : 7
// Field Start Bit : 16
// Field End Bit   : 22
// Description     : The DASL & DASS instructrion which operation Index =1 number  are not done.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX1_NUM_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM
// Field Width     : 7
// Field Start Bit : 0
// Field End Bit   : 6
// Description     : The DASL & DASS instructrion which operation Index =0 number  are not done.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS0_INDEX0_NUM_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_DSYNC_STATUS1
// Register Offset : 0x24
// Description     : DSYNC intruction status.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_DSYNC_STATUS1 (AHB_GAMA_IRQST_BASE_ADDR + (0x24<<0))

#define AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM_FIELD_OFFSET 16
#define AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM_FIELD_SIZE 7

#define AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM_FIELD_SIZE 7


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM
// Field Width     : 7
// Field Start Bit : 16
// Field End Bit   : 22
// Description     : The DASL & DASS instructrion which operation Index =3 number  are not done.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX3_NUM_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM
// Field Width     : 7
// Field Start Bit : 0
// Field End Bit   : 6
// Description     : The DASL & DASS instructrion which operation Index =2 number  are not done.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS1_INDEX2_NUM_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_DSYNC_STATUS2
// Register Offset : 0x28
// Description     : DSYNC intruction status.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_DSYNC_STATUS2 (AHB_GAMA_IRQST_BASE_ADDR + (0x28<<0))

#define AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM_FIELD_OFFSET 16
#define AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM_FIELD_SIZE 7

#define AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM_FIELD_SIZE 7


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM
// Field Width     : 7
// Field Start Bit : 16
// Field End Bit   : 22
// Description     : The DASL & DASS instructrion which operation Index =5 number  are not done.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX5_NUM_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM
// Field Width     : 7
// Field Start Bit : 0
// Field End Bit   : 6
// Description     : The DASL & DASS instructrion which operation Index =4 number  are not done.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS2_INDEX4_NUM_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_DSYNC_STATUS3
// Register Offset : 0x2c
// Description     : DSYNC intruction status.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_DSYNC_STATUS3 (AHB_GAMA_IRQST_BASE_ADDR + (0x2c<<0))

#define AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM_FIELD_OFFSET 16
#define AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM_FIELD_SIZE 7

#define AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM_FIELD_SIZE 7


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM
// Field Width     : 7
// Field Start Bit : 16
// Field End Bit   : 22
// Description     : The DASL & DASS instructrion which operation Index =7 number  are not done.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM_6    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM_5    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM_4    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM_3    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM_2    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM_1    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX7_NUM_0    (BIT_(16))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM
// Field Width     : 7
// Field Start Bit : 0
// Field End Bit   : 6
// Description     : The DASL & DASS instructrion which operation Index =6 number  are not done.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_DSYNC_STATUS3_INDEX6_NUM_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0
// Register Offset : 0x80
// Description     : GAMA debug status 0 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0 (AHB_GAMA_IRQST_BASE_ADDR + (0x80<<0))

#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {1'b0,o_lmul_counter[2:0],
//                    o_mac3_rdy,o_mac2_rdy,o_mac1_rdy,o_mac0_rdy,
//                    2'b00,o_mac_slide_ilp_stall,o_mac_ilp_stall,
//                    2'b00,o_proc_inst_rdy,i_dec_inst_vld,
//                    1'b0,dcfg_inst_rdy,dmas_inst_rdy,dmal_inst_rdy,
//                    dma_inst_vld,dcfg_inst_vld,dmas_inst_vld,dmal_inst_vld,
//                    dsync_inst_rdy,dsync_inst_vld,cfg_inst_rdy,cfg_inst_vld,
//                    ls_inst_rdy,ls_inst_vld,idec_di_rdy,idec_di_vld}
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST0_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1
// Register Offset : 0x84
// Description     : GAMA debug status 1 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1 (AHB_GAMA_IRQST_BASE_ADDR + (0x84<<0))

#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : idec_inst[31:0]
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST1_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2
// Register Offset : 0x88
// Description     : GAMA debug status 2 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2 (AHB_GAMA_IRQST_BASE_ADDR + (0x88<<0))

#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : vs_pm_flag[31:0]
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST2_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3
// Register Offset : 0x8c
// Description     : GAMA debug status 3 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3 (AHB_GAMA_IRQST_BASE_ADDR + (0x8c<<0))

#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : vs_l_flag[31:0]
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST3_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4
// Register Offset : 0x90
// Description     : GAMA debug status 4 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4 (AHB_GAMA_IRQST_BASE_ADDR + (0x90<<0))

#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : vs_s_flag[31:0]
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST4_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5
// Register Offset : 0x94
// Description     : GAMA debug status 5 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5 (AHB_GAMA_IRQST_BASE_ADDR + (0x94<<0))

#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : frs_l_flag[31:0]
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST5_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6
// Register Offset : 0x98
// Description     : GAMA debug status 6 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6 (AHB_GAMA_IRQST_BASE_ADDR + (0x98<<0))

#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : frs_s_flag[31:0]
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST6_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7
// Register Offset : 0x9c
// Description     : GAMA debug status 7 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7 (AHB_GAMA_IRQST_BASE_ADDR + (0x9c<<0))

#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {8'h00,
//                    2'b00,w_asw_iq_rdy,w_asw_iq_vld,
//                    2'b00,ib_fifo_rdy_i,ib_fifo_vld_o,
//                    2'b00,pre_di_rdy,pre_di_vld,
//                    1'b0,ib_fifo_entries[6:0],
//                    2'b00,ib_fifo_full,ib_fifo_empty}
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST7_VAL_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8
// Register Offset : 0xa0
// Description     : GAMA debug status 8 register
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8 (AHB_GAMA_IRQST_BASE_ADDR + (0xa0<<0))

#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_FIELD_OFFSET 0
#define AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : {5'd0,
//                    o_asw_fr_rd_ack,
//                    o_asw_fr_wr_ack,
//                    o_asw_cl_wr_ack,
//                    o_asw_cr_rd_ack,
//                    o_pref_rd_ack,
//                    o_dma_rd_ack,
//                    o_dma_wr_ack,
//                    o_epu_store_wr_ack,
//                    o_epu_load_rd_ack,
//                    o_ahb_rd_ack,
//                    o_ahb_wr_ack,
//                    5'd0,
//                    o_asw_fr_rd_req,
//                    o_asw_fr_wr_req,
//                    o_asw_cl_wr_req,
//                    o_asw_cr_rd_req,
//                    o_pref_rd_req,
//                    o_dma_rd_req,
//                    o_dma_wr_req,
//                    o_epu_store_wr_req,
//                    o_epu_load_rd_req,
//                    o_ahb_rd_req,
//                    o_ahb_wr_req}
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_IRQST_GAMA_DBG_ST8_VAL_0    (BIT_(0))

#endif
