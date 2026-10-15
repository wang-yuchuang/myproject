//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2011  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//		This file should be modified by some customers according to their SOC configuration.
//--=========================================================================--

#ifndef _VPU_CONFIG_H_
#define _VPU_CONFIG_H_
#include "config.h"

#define BODA7503_CODE 0x7503
#define CODA7542_CODE 0x7542
#define BODA950_CODE 0x9500
#define CODA960_CODE 0x9600
#define CODA980_CODE 0x9800
#define WAVE320_CODE 0x3200
#define WAVE410_CODE 0x4100   /* Wave410 version 1: Single vcore */
#define WAVE4102_CODE 0x4102  /* Wave410 version 2: Multi vcore */
#define WAVE420_CODE 0x4200   /* Wave420   */
#define WAVE420L_CODE 0x4201  /* Wave420L  */
#define WAVE420DU_CODE 0x4202 /* Wave420 dual */
#define WAVE420SN_CODE 0x4203 /* Wave420 SNIDER */
#define WAVE412_CODE 0x4120
#define WAVE510_CODE 0x5100
#define WAVE512_CODE 0x5120
#define CODA7Q_CODE 0x7101

#define WAVE517_CODE 0x5170
#define WAVE537_CODE 0x5370
#define WAVE511_CODE 0x5110
#define WAVE521_CODE 0x5210
#define WAVE521C_CODE 0x521c
#define WAVE521C_DUAL_CODE 0x521d // wave521 dual core
#define WAVE521E1_CODE 0x5211

#define ENC_STREAM_BUF_COUNT 5
#define ENC_STREAM_BUF_SIZE 0xF00000

#define WAVE517_WORKBUF_SIZE (2 * 1024 * 1024)
#define WAVE521ENC_WORKBUF_SIZE (128 * 1024) // HEVC 128K, AVC 40K
#define WAVE521DEC_WORKBUF_SIZE (1784 * 1024)

#define PRODUCT_CODE_W_SERIES(x)                                                                   \
    (x == WAVE410_CODE || x == WAVE4102_CODE || x == WAVE420_CODE || x == WAVE412_CODE ||          \
     x == CODA7Q_CODE || x == WAVE420L_CODE || x == WAVE510_CODE || x == WAVE420SN_CODE ||         \
     x == WAVE420DU_CODE || x == WAVE512_CODE || x == WAVE521C_CODE)

#define PRODUCT_CODE_NOT_W_SERIES(x)                                                               \
    (x == BODA7503_CODE || x == CODA7542_CODE || x == BODA950_CODE || x == CODA960_CODE ||         \
     x == CODA980_CODE || x == WAVE320_CODE)

#define MAX_INST_HANDLE_SIZE 48 /* DO NOT CHANGE THIS VALUE */
#define MAX_NUM_INSTANCE 8
#define MAX_NUM_VPU_CORE 3  // coda988, wave412, wave521c
#define MAX_NUM_VCORE 1

#define MAX_ENC_AVC_PIC_WIDTH 4096
#define MAX_ENC_AVC_PIC_HEIGHT 2304
#define MAX_ENC_PIC_WIDTH 4096
#define MAX_ENC_PIC_HEIGHT 2304
#define MIN_ENC_PIC_WIDTH 96
#define MIN_ENC_PIC_HEIGHT 16

// for WAVE420
#define W4_MIN_ENC_PIC_WIDTH 256
#define W4_MIN_ENC_PIC_HEIGHT 128
#define W4_MAX_ENC_PIC_WIDTH 8192
#define W4_MAX_ENC_PIC_HEIGHT 8192

#define MAX_DEC_PIC_WIDTH 4096
#define MAX_DEC_PIC_HEIGHT 2304

#define W5_MIN_ENC_PIC_WIDTH 256
#define W5_MIN_ENC_PIC_HEIGHT 128
#define W5_MAX_ENC_PIC_WIDTH 8192
#define W5_MAX_ENC_PIC_HEIGHT 8192

#define MAX_CTU_NUM 0x4000 // CTU num for max resolution = 8192x8192/(64x64)
#define MAX_SUB_CTU_NUM (MAX_CTU_NUM * 4)
#define MAX_MB_NUM 0x40000 // MB num for max resolution = 8192x8192/(16x16)

//  Application specific configuration
#define VPU_ENC_TIMEOUT 1000
#define VPU_DEC_TIMEOUT 1000

#ifdef HAPS_SIM
#define VPU_BUSY_CHECK_TIMEOUT (5000 * 10000)
#else
#define VPU_BUSY_CHECK_TIMEOUT 5000
#endif

// codec specific configuration
#define VPU_REORDER_ENABLE 1 // it can be set to 1 to handle reordering DPB in host side.
#define CBCR_INTERLEAVE                                                                            \
    1 //[default 1 for BW checking with CnMViedo Conformance] 0 (chroma separate mode), 1 (chroma
      //interleave mode) // if the type of tiledmap uses the kind of MB_RASTER_MAP. must set to
      //enable CBCR_INTERLEAVE
#define VPU_ENABLE_BWB 1

#define HOST_ENDIAN VDI_128BIT_LITTLE_ENDIAN
#define VPU_FRAME_ENDIAN HOST_ENDIAN
#define VPU_STREAM_ENDIAN HOST_ENDIAN
#define VPU_USER_DATA_ENDIAN HOST_ENDIAN
#define VPU_SOURCE_ENDIAN HOST_ENDIAN
#define DRAM_BUS_WIDTH 16

#define USE_SRC_PRP_AXI 0
#define USE_SRC_PRI_AXI 1
#define DEFAULT_SRC_AXI USE_SRC_PRP_AXI

#define WAVE5_UPPER_PROC_AXI_ID 0x0

#define WAVE5_PROC_AXI_ID 0x0
#define WAVE5_PRP_AXI_ID 0x0
#define WAVE5_FBD_Y_AXI_ID 0x0
#define WAVE5_FBC_Y_AXI_ID 0x0
#define WAVE5_FBD_C_AXI_ID 0x0
#define WAVE5_FBC_C_AXI_ID 0x0
#define WAVE5_SEC_AXI_ID 0x0
#define WAVE5_PRI_AXI_ID 0x0

/************************************************************************/
/* VPU COMMON MEMORY                                                    */
/************************************************************************/
#define VLC_BUF_NUM (3)
#define REPORT_QUEUE_COUNT (16)

#ifdef SUPPORT_TESTCASE_CQ_16
#define WAVE5_ENC_SRC_BUF_NUM (20 + COMMAND_QUEUE_DEPTH)
#else
#define WAVE5_ENC_SRC_BUF_NUM 20 // [FIX ME]
#endif

#define ONE_TASKBUF_SIZE_FOR_W5DEC_CQ (8 * 1024 * 1024)   /* upto 8Kx4K, need 8Mbyte per task*/
#define ONE_TASKBUF_SIZE_FOR_W5ENC_CQ (8 * 1024 * 1024)   /* upto 8Kx8K, need 8Mbyte per task.*/
#define ONE_TASKBUF_SIZE_FOR_W511DEC_CQ (8 * 1024 * 1024) /* upto 8Kx8K, need 8Mbyte per task.*/

/* FW will return one TASKBUF size base on MaxCPB (according to the SPEC), but
 * this size will be quite big depend on profile/level.*/
/* ex) main10, 8kx8k = 180Mbytes will be returned */
/* Thus, if host can set size limitation for one TASKBUF size. (but, small size
 * limitation can cause processing error)  */
#define ONE_TASKBUF_MAX_SIZE_LIMIT_DEC (8 * 1024 * 1024)
#define ONE_TASKBUF_MAX_SIZE_LIMIT_ENC (20 * 1024 * 1024)

#define W5_REMAP_INDEX0 0
#define W5_REMAP_INDEX1 1
#define W5_REMAP_MAX_SIZE (1024 * 1024)

#define WAVE5_MAX_CODE_BUF_SIZE (2 * 1024 * 1024)
#define WAVE5_TEMPBUF_OFFSET WAVE5_MAX_CODE_BUF_SIZE
#define WAVE5_TEMPBUF_SIZE (1024 * 1024)
#define WAVE5_SIZE_COMMON (WAVE5_MAX_CODE_BUF_SIZE + WAVE5_TEMPBUF_SIZE)

#define COMMAND_QUEUE_DEPTH 4
#define ONE_TASKBUF_SIZE_FOR_CQ (8 * 1024 * 1024) /* upto 8Kx4K, need 8Mbyte per task*/
#if defined(WAVE420SN) || defined(WAVE420DU)
#define SIZE_COMMON                                                                                \
    ((2 * 1024 * 1024) + (1 * 1024 * 1024) +                                                       \
     (4 * 1024 * 1024)) /* added one more DEFAULT_TEMPBUF_SIZE + shared buf for multi instance */
#else
#define SIZE_COMMON (2 * 1024 * 1024)
#endif

//=====4. VPU REPORT MEMORY  ======================//
#define SIZE_REPORT_BUF (0x10000)

#define STREAM_END_SIZE 0
#define STREAM_END_SET_FLAG 0
#define STREAM_END_CLEAR_FLAG -1

#define EXPLICIT_END_SET_FLAG -2
#define UPDATE_NEW_BS_BUF 0

#define USE_BIT_INTERNAL_BUF 1
#define USE_IP_INTERNAL_BUF 1
#define USE_DBKY_INTERNAL_BUF 1
#define USE_DBKC_INTERNAL_BUF 1
#define USE_OVL_INTERNAL_BUF 1
#define USE_BTP_INTERNAL_BUF 1
#define USE_ME_INTERNAL_BUF 1
#define USE_SCALER_INTERNAL_BUF 1
/* WAVE410 only */
#define USE_BPU_INTERNAL_BUF 1
#define USE_VCE_IP_INTERNAL_BUF 1
#define USE_VCE_LF_ROW_INTERNAL_BUF 1

/* WAVE420 only */
#define USE_IMD_INTERNAL_BUF 1
#define USE_RDO_INTERNAL_BUF 1
#define USE_LF_INTERNAL_BUF 1

#endif /* _VPU_CONFIG_H_ */
