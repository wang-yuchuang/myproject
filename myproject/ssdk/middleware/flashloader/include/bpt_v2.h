#ifndef BPT_V2_H_
#define BPT_V2_H_

#define SFS_NORMAL_IMG_ADDR         0x70
#define SFS_BAKEUP_IMG_ADDR         0x74
#define SFS_THIRD_IMG_ADDR          0x78

#define SFS_BASE                    0x0
#define SFS_SIZE                    0x80

#define BPT_TAG_OFFSET              0x0
#define BPT_IIB_OFFSET              0x20
#define BPT_CRC_CHECKSUM_OFFSET     0xFEC

#define BPT_IIB_TARGET_OFFSET       0x19
#define BPT_IIB_IMG_SIZE_OFFSET     0x28
#define BPT_IIB_LOAD_ADDR_OFFSET    0x2C

#define BPT_IIB_SIZE                124
#define BPT_IIB_NUM                 8
#define BPT_TAG_VAL                 0x42505402
#define BPT_IIB_TAG_VAL             0xEAE2

#define BPT_HEDAER_LEN              0x1000
#define BPT_PAGE_SIZE               512

#define BPT_RCP_TAG                 0xEAF0
#define BPT_RCP_SIZE                0x414

#define BPT_CORE_CR5_SF             0
#define BPT_CORE_CR5_SP0            2
#define BPT_CORE_CR5_SP1            3
#define BPT_CORE_CR5_SX0            4
#define BPT_CORE_CR5_SX1            5
#define BPT_CORE_CR5_SX             6
#define BPT_CORE_CR5_SP             7

#define BPT_PAGE_SN                 0x0100

typedef struct bpt_rcp {
  uint16_t          tag;
  uint16_t          size;
  uint8_t           rot_id;
  uint8_t           key_type;
  uint8_t           reserved[10];
  uint8_t           key[1028];
} bpt_rcp_t;

typedef struct bpt_v2 {
  uint32_t          tag;
  uint16_t          reserved1;
  uint16_t          size;
  uint32_t          sec_ver;
  uint8_t           digest_alg;
  uint8_t           reserved2[19];
  uint8_t           iib[BPT_IIB_NUM][BPT_IIB_SIZE];
  bpt_rcp_t         rcp;
  uint8_t           signature[512];
  uint8_t           key_wraps[512];
  uint8_t           reserved3[984];
  uint32_t          crc;
  uint32_t          sn;
  uint32_t          sn_inversion;
  uint8_t           reserved4[8];
} bpt_v2_t;



typedef struct bpt_iib {
  uint16_t          tag;
  uint16_t          size;
  uint32_t          reserved1;
  uint8_t           dbg_ctrl[8];
  uint8_t           did[8];
  uint8_t           img_type;
  uint8_t           target_core;
  uint8_t           dec_ctrl;
  uint8_t           reserved2;
  uint8_t           iv[8];
  uint32_t          img_page;
  uint32_t          img_size;
  uint32_t          load_addr;
  uint32_t          reserved3;
  uint32_t          entry;
  uint32_t          reserved4;
  uint8_t           hash[64];
} bpt_iib_t;

int BptUpdate(flash_wrapper_t *flash_wrapper, void *img_base,
              uint32_t link_base, uint32_t img_size, uint8_t core);

#endif
