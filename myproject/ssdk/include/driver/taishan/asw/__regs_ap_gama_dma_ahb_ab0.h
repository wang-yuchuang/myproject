#ifndef _AP_AHB_GAMA_DMA_AB0_REG_H
#define _AP_AHB_GAMA_DMA_AB0_REG_H

//--------------------------------------------------------------------------
// Component          : semidrive.com, d_ip_gama, gama_dma, r0p0
// Memory Map         : AHB
// Address Block Name : AB0
// Description        : Address Block 0 of asw
//--------------------------------------------------------------------------
#define AHB_GAMA_DMA_BASE_ADDR 0x0


//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_SCR_CFG
// Register Offset : 0x0
// Description     : RDMA Source configuration
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_SCR_CFG (AHB_GAMA_DMA_BASE_ADDR + (0x0<<0))

#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_SCR_CFG_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_SCR_CFG_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_STRIDE_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1
// Register Offset : 0x40
// Description     : RDMA Source configuration
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1 (AHB_GAMA_DMA_BASE_ADDR + (0x40<<0))

#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_1_STRIDE_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2
// Register Offset : 0x80
// Description     : RDMA Source configuration
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2 (AHB_GAMA_DMA_BASE_ADDR + (0x80<<0))

#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_SCR_CFG_2_STRIDE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_DST_CFG
// Register Offset : 0x4
// Description     : RDMA destination configuration.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_DST_CFG (AHB_GAMA_DMA_BASE_ADDR + (0x4<<0))

#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DST_CFG_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DST_CFG_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_STRIDE_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1
// Register Offset : 0x44
// Description     : RDMA destination configuration.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1 (AHB_GAMA_DMA_BASE_ADDR + (0x44<<0))

#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_1_STRIDE_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2
// Register Offset : 0x84
// Description     : RDMA destination configuration.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2 (AHB_GAMA_DMA_BASE_ADDR + (0x84<<0))

#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DST_CFG_2_STRIDE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_SCR_CFG
// Register Offset : 0x8
// Description     : WDMA Source configuration
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_SCR_CFG (AHB_GAMA_DMA_BASE_ADDR + (0x8<<0))

#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_SCR_CFG_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_SCR_CFG_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_STRIDE_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1
// Register Offset : 0x48
// Description     : WDMA Source configuration
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1 (AHB_GAMA_DMA_BASE_ADDR + (0x48<<0))

#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_SCR_CFG_1_STRIDE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_DST_CFG
// Register Offset : 0xc
// Description     : WDMA destination configuration.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_DST_CFG (AHB_GAMA_DMA_BASE_ADDR + (0xc<<0))

#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_DST_CFG_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_DST_CFG_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_STRIDE_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1
// Register Offset : 0x4c
// Description     : WDMA destination configuration.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1 (AHB_GAMA_DMA_BASE_ADDR + (0x4c<<0))

#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_PRIORITY_FIELD_OFFSET 30
#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_PRIORITY_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_BL_FIELD_OFFSET 20
#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_BL_FIELD_SIZE 4

#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_FIELD_SIZE 20


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_PRIORITY
// Field Width     : 2
// Field Start Bit : 30
// Field End Bit   : 31
// Description     : DMA internal arbitration priority, the lower the
//                   higher
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_PRIORITY_1    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_PRIORITY_0    (BIT_(30))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_BL
// Field Width     : 4
// Field Start Bit : 20
// Field End Bit   : 23
// Description     : burst  length  configuration  when  source
//                   address points to external memory
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_BL_3    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_BL_2    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_BL_1    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_BL_0    (BIT_(20))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE
// Field Width     : 20
// Field Start Bit : 0
// Field End Bit   : 19
// Description     : signed  stride  value.  When  0  loop
//                   mode  is  disabled;  when  grater  than  0  increase  source  address  by
//                   SRC_STRIDE after each loop
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DST_CFG_1_STRIDE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH
// Register Offset : 0x10
// Description     : Command FIFO depth for 2 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH (AHB_GAMA_DMA_BASE_ADDR + (0x10<<0))

#define AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Data FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 8
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_DEPTH_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1
// Register Offset : 0x50
// Description     : Command FIFO depth for 2 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1 (AHB_GAMA_DMA_BASE_ADDR + (0x50<<0))

#define AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Data FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 8
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_1_DEPTH_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2
// Register Offset : 0x90
// Description     : Command FIFO depth for 2 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2 (AHB_GAMA_DMA_BASE_ADDR + (0x90<<0))

#define AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Data FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 8
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_CFIFO_DEPTH_2_DEPTH_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH
// Register Offset : 0x14
// Description     : Data FIFO depth for 8 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH (AHB_GAMA_DMA_BASE_ADDR + (0x14<<0))

#define AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Command FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 2
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_DEPTH_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1
// Register Offset : 0x54
// Description     : Data FIFO depth for 8 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1 (AHB_GAMA_DMA_BASE_ADDR + (0x54<<0))

#define AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Command FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 2
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_1_DEPTH_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2
// Register Offset : 0x94
// Description     : Data FIFO depth for 8 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2 (AHB_GAMA_DMA_BASE_ADDR + (0x94<<0))

#define AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Command FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 2
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_DFIFO_DEPTH_2_DEPTH_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL
// Register Offset : 0x18
// Description     : RDMA control.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL (AHB_GAMA_DMA_BASE_ADDR + (0x18<<0))

#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_PROT_FIELD_OFFSET 4
#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_PROT_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_CACHE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_CACHE_FIELD_SIZE 4


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_PROT
// Field Width     : 2
// Field Start Bit : 4
// Field End Bit   : 5
// Description     : prot
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_PROT_1    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_PROT_0    (BIT_(4))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_CACHE
// Field Width     : 4
// Field Start Bit : 0
// Field End Bit   : 3
// Description     : cache
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_CACHE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_CACHE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_CACHE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_CACHE_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1
// Register Offset : 0x58
// Description     : RDMA control.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1 (AHB_GAMA_DMA_BASE_ADDR + (0x58<<0))

#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_PROT_FIELD_OFFSET 4
#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_PROT_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_CACHE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_CACHE_FIELD_SIZE 4


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_PROT
// Field Width     : 2
// Field Start Bit : 4
// Field End Bit   : 5
// Description     : prot
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_PROT_1    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_PROT_0    (BIT_(4))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_CACHE
// Field Width     : 4
// Field Start Bit : 0
// Field End Bit   : 3
// Description     : cache
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_CACHE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_CACHE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_CACHE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_1_CACHE_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2
// Register Offset : 0x98
// Description     : RDMA control.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2 (AHB_GAMA_DMA_BASE_ADDR + (0x98<<0))

#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_PROT_FIELD_OFFSET 4
#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_PROT_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_CACHE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_CACHE_FIELD_SIZE 4


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_PROT
// Field Width     : 2
// Field Start Bit : 4
// Field End Bit   : 5
// Description     : prot
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_PROT_1    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_PROT_0    (BIT_(4))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_CACHE
// Field Width     : 4
// Field Start Bit : 0
// Field End Bit   : 3
// Description     : cache
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_CACHE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_CACHE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_CACHE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_AXI_CTRL_2_CACHE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW
// Register Offset : 0x1c
// Description     : framebuffer lower limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW (AHB_GAMA_DMA_BASE_ADDR + (0x1c<<0))

#define AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : Low limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_LIMIT_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1
// Register Offset : 0x5c
// Description     : framebuffer lower limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1 (AHB_GAMA_DMA_BASE_ADDR + (0x5c<<0))

#define AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : Low limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_1_LIMIT_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2
// Register Offset : 0x9c
// Description     : framebuffer lower limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2 (AHB_GAMA_DMA_BASE_ADDR + (0x9c<<0))

#define AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : Low limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_LOW_2_LIMIT_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH
// Register Offset : 0x20
// Description     : framebuffer High limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH (AHB_GAMA_DMA_BASE_ADDR + (0x20<<0))

#define AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_LIMIT_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1
// Register Offset : 0x60
// Description     : framebuffer High limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1 (AHB_GAMA_DMA_BASE_ADDR + (0x60<<0))

#define AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_1_LIMIT_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2
// Register Offset : 0xa0
// Description     : framebuffer High limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2 (AHB_GAMA_DMA_BASE_ADDR + (0xa0<<0))

#define AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_RDMA_ADDR_HIGH_2_LIMIT_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_FB_CTRL
// Register Offset : 0x24
// Description     : Frame buffer mode ctrl
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_FB_CTRL (AHB_GAMA_DMA_BASE_ADDR + (0x24<<0))

#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_WRAP_RANGE_FIELD_OFFSET 1
#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_WRAP_RANGE_FIELD_SIZE 3

#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_CIRCULAR_EN_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_CIRCULAR_EN_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_FB_CTRL_WRAP_RANGE
// Field Width     : 3
// Field Start Bit : 1
// Field End Bit   : 3
// Description     : Wrapper range.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_WRAP_RANGE_2    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_WRAP_RANGE_1    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_WRAP_RANGE_0    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_FB_CTRL_CIRCULAR_EN
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : 1: Circular buffer mode.
//                   0: Normal buffer mode.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_CIRCULAR_EN    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1
// Register Offset : 0x64
// Description     : Frame buffer mode ctrl
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1 (AHB_GAMA_DMA_BASE_ADDR + (0x64<<0))

#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_WRAP_RANGE_FIELD_OFFSET 1
#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_WRAP_RANGE_FIELD_SIZE 3

#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_CIRCULAR_EN_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_CIRCULAR_EN_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_WRAP_RANGE
// Field Width     : 3
// Field Start Bit : 1
// Field End Bit   : 3
// Description     : Wrapper range.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_WRAP_RANGE_2    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_WRAP_RANGE_1    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_WRAP_RANGE_0    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_CIRCULAR_EN
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : 1: Circular buffer mode.
//                   0: Normal buffer mode.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_1_CIRCULAR_EN    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2
// Register Offset : 0xa4
// Description     : Frame buffer mode ctrl
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2 (AHB_GAMA_DMA_BASE_ADDR + (0xa4<<0))

#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_WRAP_RANGE_FIELD_OFFSET 1
#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_WRAP_RANGE_FIELD_SIZE 3

#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_CIRCULAR_EN_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_CIRCULAR_EN_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_WRAP_RANGE
// Field Width     : 3
// Field Start Bit : 1
// Field End Bit   : 3
// Description     : Wrapper range.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_WRAP_RANGE_2    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_WRAP_RANGE_1    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_WRAP_RANGE_0    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_CIRCULAR_EN
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : 1: Circular buffer mode.
//                   0: Normal buffer mode.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_FB_CTRL_2_CIRCULAR_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_RDMA_CTRL
// Register Offset : 0x1000
// Description     : RDMA control
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_RDMA_CTRL (AHB_GAMA_DMA_BASE_ADDR + (0x1000<<0))

#define AP_AHB_GAMA_DMA_RDMA_CTRL_CFG_LOAD_FIELD_OFFSET 1
#define AP_AHB_GAMA_DMA_RDMA_CTRL_CFG_LOAD_FIELD_SIZE 1

#define AP_AHB_GAMA_DMA_RDMA_CTRL_ARB_SEL_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_RDMA_CTRL_ARB_SEL_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_CTRL_CFG_LOAD
// Field Width     : 1
// Field Start Bit : 1
// Field End Bit   : 1
// Description     : rdma config load
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_CTRL_CFG_LOAD    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_RDMA_CTRL_ARB_SEL
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : arbitor mode select
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_RDMA_CTRL_ARB_SEL    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH
// Register Offset : 0x2000
// Description     : Command FIFO depth for 2 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH (AHB_GAMA_DMA_BASE_ADDR + (0x2000<<0))

#define AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Command FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 2
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_DEPTH_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1
// Register Offset : 0x2040
// Description     : Command FIFO depth for 2 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1 (AHB_GAMA_DMA_BASE_ADDR + (0x2040<<0))

#define AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Command FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 2
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_CFIFO_DEPTH_1_DEPTH_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH
// Register Offset : 0x2004
// Description     : Data FIFO depth for 8 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH (AHB_GAMA_DMA_BASE_ADDR + (0x2004<<0))

#define AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Data FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 8
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_DEPTH_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1
// Register Offset : 0x2044
// Description     : Data FIFO depth for 8 words.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1 (AHB_GAMA_DMA_BASE_ADDR + (0x2044<<0))

#define AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_FIELD_SIZE 10


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH
// Field Width     : 10
// Field Start Bit : 0
// Field End Bit   : 9
// Description     : Data FIFO Depth for each DMA channel.
//                   The real FIFO depth = DEPTH * 8
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_DFIFO_DEPTH_1_DEPTH_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL
// Register Offset : 0x2008
// Description     : WDMA control.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL (AHB_GAMA_DMA_BASE_ADDR + (0x2008<<0))

#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CHN_RST_FIELD_OFFSET 7
#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CHN_RST_FIELD_SIZE 1

#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_BUFAB_CFG_FIELD_OFFSET 6
#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_BUFAB_CFG_FIELD_SIZE 1

#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_PROT_FIELD_OFFSET 4
#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_PROT_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CACHE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CACHE_FIELD_SIZE 4


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CHN_RST
// Field Width     : 1
// Field Start Bit : 7
// Field End Bit   : 7
// Description     : channel reset
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CHN_RST    (BIT_(7))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_BUFAB_CFG
// Field Width     : 1
// Field Start Bit : 6
// Field End Bit   : 6
// Description     : complete mode sel
//                   0=AXI B channel transaction
//                   1=data out from FIFO
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_BUFAB_CFG    (BIT_(6))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_PROT
// Field Width     : 2
// Field Start Bit : 4
// Field End Bit   : 5
// Description     : prot
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_PROT_1    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_PROT_0    (BIT_(4))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CACHE
// Field Width     : 4
// Field Start Bit : 0
// Field End Bit   : 3
// Description     : cache
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CACHE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CACHE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CACHE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_CACHE_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1
// Register Offset : 0x2048
// Description     : WDMA control.
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1 (AHB_GAMA_DMA_BASE_ADDR + (0x2048<<0))

#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CHN_RST_FIELD_OFFSET 7
#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CHN_RST_FIELD_SIZE 1

#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_BUFAB_CFG_FIELD_OFFSET 6
#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_BUFAB_CFG_FIELD_SIZE 1

#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_PROT_FIELD_OFFSET 4
#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_PROT_FIELD_SIZE 2

#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CACHE_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CACHE_FIELD_SIZE 4


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CHN_RST
// Field Width     : 1
// Field Start Bit : 7
// Field End Bit   : 7
// Description     : channel reset
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CHN_RST    (BIT_(7))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_BUFAB_CFG
// Field Width     : 1
// Field Start Bit : 6
// Field End Bit   : 6
// Description     : complete mode sel
//                   0=AXI B channel transaction
//                   1=data out from FIFO
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_BUFAB_CFG    (BIT_(6))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_PROT
// Field Width     : 2
// Field Start Bit : 4
// Field End Bit   : 5
// Description     : prot
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_PROT_1    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_PROT_0    (BIT_(4))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CACHE
// Field Width     : 4
// Field Start Bit : 0
// Field End Bit   : 3
// Description     : cache
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CACHE_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CACHE_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CACHE_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_AXI_CTRL_1_CACHE_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW
// Register Offset : 0x200c
// Description     : framebuffer lower limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW (AHB_GAMA_DMA_BASE_ADDR + (0x200c<<0))

#define AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : Low limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_LIMIT_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1
// Register Offset : 0x204c
// Description     : framebuffer lower limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1 (AHB_GAMA_DMA_BASE_ADDR + (0x204c<<0))

#define AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : Low limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_LOW_1_LIMIT_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH
// Register Offset : 0x2010
// Description     : framebuffer High limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH (AHB_GAMA_DMA_BASE_ADDR + (0x2010<<0))

#define AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_LIMIT_0    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1
// Register Offset : 0x2050
// Description     : framebuffer High limit addr
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1 (AHB_GAMA_DMA_BASE_ADDR + (0x2050<<0))

#define AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_FIELD_SIZE 32


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT
// Field Width     : 32
// Field Start Bit : 0
// Field End Bit   : 31
// Description     : limit address
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_31    (BIT_(31))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_30    (BIT_(30))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_29    (BIT_(29))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_28    (BIT_(28))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_27    (BIT_(27))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_26    (BIT_(26))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_25    (BIT_(25))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_24    (BIT_(24))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_23    (BIT_(23))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_22    (BIT_(22))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_21    (BIT_(21))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_20    (BIT_(20))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_19    (BIT_(19))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_18    (BIT_(18))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_17    (BIT_(17))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_16    (BIT_(16))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_15    (BIT_(15))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_14    (BIT_(14))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_13    (BIT_(13))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_12    (BIT_(12))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_11    (BIT_(11))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_10    (BIT_(10))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_9    (BIT_(9))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_8    (BIT_(8))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_7    (BIT_(7))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_6    (BIT_(6))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_5    (BIT_(5))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_4    (BIT_(4))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_3    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_2    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_1    (BIT_(1))
#define BIT_AP_AHB_GAMA_DMA_WDMA_ADDR_HIGH_1_LIMIT_0    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_FB_CTRL
// Register Offset : 0x2014
// Description     : Frame buffer mode ctrl
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_FB_CTRL (AHB_GAMA_DMA_BASE_ADDR + (0x2014<<0))

#define AP_AHB_GAMA_DMA_WDMA_FB_CTRL_WRAP_RANGE_FIELD_OFFSET 1
#define AP_AHB_GAMA_DMA_WDMA_FB_CTRL_WRAP_RANGE_FIELD_SIZE 3

#define AP_AHB_GAMA_DMA_WDMA_FB_CTRL_CIRCULAR_EN_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_FB_CTRL_CIRCULAR_EN_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_FB_CTRL_WRAP_RANGE
// Field Width     : 3
// Field Start Bit : 1
// Field End Bit   : 3
// Description     : Wrapper range.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_WRAP_RANGE_2    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_WRAP_RANGE_1    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_WRAP_RANGE_0    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_FB_CTRL_CIRCULAR_EN
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : 1: Circular buffer mode.
//                   0: Normal buffer mode.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_CIRCULAR_EN    (BIT_(0))
//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1
// Register Offset : 0x2054
// Description     : Frame buffer mode ctrl
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1 (AHB_GAMA_DMA_BASE_ADDR + (0x2054<<0))

#define AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_WRAP_RANGE_FIELD_OFFSET 1
#define AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_WRAP_RANGE_FIELD_SIZE 3

#define AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_CIRCULAR_EN_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_CIRCULAR_EN_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_WRAP_RANGE
// Field Width     : 3
// Field Start Bit : 1
// Field End Bit   : 3
// Description     : Wrapper range.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_WRAP_RANGE_2    (BIT_(3))
#define BIT_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_WRAP_RANGE_1    (BIT_(2))
#define BIT_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_WRAP_RANGE_0    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_CIRCULAR_EN
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : 1: Circular buffer mode.
//                   0: Normal buffer mode.
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_FB_CTRL_1_CIRCULAR_EN    (BIT_(0))

//--------------------------------------------------------------------------
// Register Name   : REG_AP_AHB_GAMA_DMA_WDMA_CTRL
// Register Offset : 0x3000
// Description     : WDMA control
//--------------------------------------------------------------------------
#define REG_AP_AHB_GAMA_DMA_WDMA_CTRL (AHB_GAMA_DMA_BASE_ADDR + (0x3000<<0))

#define AP_AHB_GAMA_DMA_WDMA_CTRL_CFG_LOAD_FIELD_OFFSET 1
#define AP_AHB_GAMA_DMA_WDMA_CTRL_CFG_LOAD_FIELD_SIZE 1

#define AP_AHB_GAMA_DMA_WDMA_CTRL_ARB_SEL_FIELD_OFFSET 0
#define AP_AHB_GAMA_DMA_WDMA_CTRL_ARB_SEL_FIELD_SIZE 1


//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_CTRL_CFG_LOAD
// Field Width     : 1
// Field Start Bit : 1
// Field End Bit   : 1
// Description     : rdma config load
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_CTRL_CFG_LOAD    (BIT_(1))

//--------------------------------------------------------------------------
// Field Name      : AP_AHB_GAMA_DMA_WDMA_CTRL_ARB_SEL
// Field Width     : 1
// Field Start Bit : 0
// Field End Bit   : 0
// Description     : arbitor mode select
//--------------------------------------------------------------------------
#define BIT_AP_AHB_GAMA_DMA_WDMA_CTRL_ARB_SEL    (BIT_(0))

#endif
