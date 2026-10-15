/*
 * errcode.h
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * error code common define
 *
 * Revision History:
 * -----------------
 */

#ifndef _SYS_ERRCODE_H_
#define _SYS_ERRCODE_H_

#define ERR_HANDLE  int

#define ERRCODE_OK 0
#define ERRCODE_NG -1

#define ERRCODE_DEBUG 0
#if ERRCODE_DEBUG
#define ERRCODE_DEBUG_PRINT(x...) printf(x)
#else
#define ERRCODE_DEBUG_PRINT(x...)
#endif

/*
 * Module ID definition
 */
typedef enum {
	MOD_ID_ADC                  = 0,	/* Analog-to-Digital Converter */
	MOD_ID_CANFD                = 1,	/* CANFD (Controller Area Network with Flexible Data-Rate) Interface */
	MOD_ID_CE                   = 2,	/* Crypto Engine */
	MOD_ID_CKGEN_AP             = 3,	/* Clock Generator For Modules in AP Domain */
	MOD_ID_CKGEN_DISP           = 4,	/* Clock Generator For Display in AP Domain */
	MOD_ID_CKGEN_SAFETY         = 5,	/* Clock Generator For Modules in Safety Domain  */
	MOD_ID_CKGEN_XPU            = 6,	/* Clock Generator For Processing Unit in AP Domain */
	MOD_ID_CSI                  = 7,	/* CMOS Sensor Interface */
	MOD_ID_CSSYS                = 8,	/* CoreSight  */
	MOD_ID_DAP                  = 9,	/* Debug Access Port  */
	MOD_ID_DC                   = 10,	/* Display Controller  */
	MOD_ID_DDR                  = 11,	/* DDR Controller */
	MOD_ID_DMA                  = 12,	/* Direct Memory Access Controller */
	MOD_ID_DP                   = 13,	/* Display Processor  */
	MOD_ID_EFUSEC               = 14,	/* E-Fuse Controller  */
	MOD_ID_ETHERNET             = 15,	/* 10/100/1000Mbps Ethernet Controller  */
	MOD_ID_FTBU                 = 16,	/* Fixed Translation Buffer Unit */
	MOD_ID_G2D                  = 17,	/* 2D Graphics Processing Unit */
	MOD_ID_GIC                  = 18,	/* Generic Interrupt Controller  */
	MOD_ID_GPIO                 = 19,	/* General Purpose Input/Output Controller  */
	MOD_ID_GPU                  = 20,	/* 3D Graphics Processing Unit  */
	MOD_ID_I2C                  = 21,	/* I2C (Inter-Integrated Circuit) Interface */
	MOD_ID_I2S                  = 22,	/* I2S (Inter-IC Sound) Interface */
	MOD_ID_IRAM                 = 23,	/* Internal RAM  */
	MOD_ID_LVDS                 = 24,	/* LVDS Display Interface  */
	MOD_ID_MAILBOX              = 25,	/* Mailbox Message Buffer  */
	MOD_ID_MIPI_CSI             = 26,	/* Mobile Industry Processor Interface CSI */
 	MOD_ID_MIPI_DSI             = 27,	/* Mobile Industry Processor Interface DSI */
	MOD_ID_JPU                  = 28,	/* Motion JPEG Codec  */
	MOD_ID_MSHC                 = 29,	/* SD/eMMC Controller  */
	MOD_ID_OSC_RTC              = 30,	/* Crystal-less Oscillator in RTC Domain   */
	MOD_ID_OSC_SAFETY           = 31,	/* Crystal-less Oscillator in Safety Domain  */
	MOD_ID_OSPI                 = 32,	/* Octal SPI (Serial Peripheral Interface) */
	MOD_ID_PCIE                 = 33,	/* PCIe  */
	MOD_ID_PINCTRL              = 34,	/* Control Registers For Pin  */
	MOD_ID_PLL                  = 35,	/* PLL */
	MOD_ID_PMU                  = 36,	/* Power Management Unit */
	MOD_ID_PVT_SNS              = 37,	/* PVT Sensor */
	MOD_ID_PWM                  = 38,	/* Pulse Width Modulation Controller  */
	MOD_ID_ROM                  = 39,	/* Boot ROM */
	MOD_ID_RSTGEN               = 40,	/* Reset Generation Controller */
	MOD_ID_RTC                  = 41,	/* Real-Time Clock */
	MOD_ID_SAFETY_CORE          = 42,	/* Safety Processor */
	MOD_ID_SCR                  = 43,	/* System Control Register */
	MOD_ID_SEC_STORAGE          = 44,	/* Secure Storage  */
	MOD_ID_SECURE_CORE          = 45,	/* Secure Processor */
	MOD_ID_SID                  = 46,	/* Stream ID Configuration Register */
	MOD_ID_SMMU                 = 47,	/* System MMU */
	MOD_ID_SPDIF                = 48,	/* Sony/Philips Digital Interface  */
	MOD_ID_SPI                  = 49,	/* Serial Peripheral Interface */
	MOD_ID_SSID                 = 50,	/* Sub-Stream ID Configuration Register */
	MOD_ID_SYS_CNT              = 51,	/* System Counter */
	MOD_ID_TMR                  = 52,	/* Generic Timer */
	MOD_ID_UART                 = 53,	/* Universal Asynchronous Receiver and Transmitter Interface  */
	MOD_ID_USB                  = 54,	/* USB Controller */
	MOD_ID_VDSP                 = 55,	/* Vision DSP */
	MOD_ID_VPU                  = 56,	/* Video Processing Unit */
	MOD_ID_WDT                  = 57,	/* Watchdog Timer */
	MOD_ID_XTAL                 = 58,	/* Crystal Oscillator  */
	MOD_ID_OTA                  = 59,	/* OTA Upgrade  */
	MOD_ID_RESET                = 60,	/* System Reset */
	MOD_ID_SEM                  = 61,	/* System Error Management */
	MOD_ID_STARTUP              = 62,	/* System Startup */
	MOD_ID_SYS_SERVICE          = 63,	/* System Service */
	MOD_ID_SYS_SERVICE_STR_MMC  = 64,	/* System Service-storage eMMC */
	MOD_ID_SYS_SERVICE_STR_OSPI = 65,	/* System Service-storage OSPI */
	MOD_ID_SYS_SERVICE_CAMERA   = 66,	/* System Service-camera */
	MOD_ID_SYS_SERVICE_AUDIO    = 67,	/* System Service-audio */
	MOD_ID_SYS_SERVICE_DISPLAY  = 68,	/* System Service-display */
	MOD_ID_SYS_SERVICE_COMM     = 69,	/* System Service-communication */
	MOD_ID_SYS_SERVICE_AUDIOHUB = 70,	/* System Service-audio hub */
	MOD_ID_SYS_SERVICE_HSM      = 71,	/* System Service-hsm */
	MOD_ID_SYS_SERVICE_SHMEM    = 72,	/* System Service-share memory */
	MOD_ID_SYS_SERVICE_STR      = 73,	/* System Service-str */
	MOD_ID_MAX,
} mod_id_t;

#if 0
/*
 * Error code definition
 */
typedef enum {
	EFOTA_NET             = 0,	/* OTA upgrade failed due to network abnormal */
	EFOTA_PAK             = 1,	/* OTA upgrade failed due to update package abnormal */
	EFOTA_WRI_EMMC        = 2,	/* OTA upgrade failed due to writting eMMC error */
	EFOTA_WRI_OSPI        = 3,	/* OTA upgrade failed due to writting OSPI Nor-Flash error */
	EFOTA_TIMEOUT         = 4,	/* OTA upgrade failed due to timeout */
	ERESET_OVER_THERMAL   = 5,	/* System reset due to over thermal */
	ERESET_WTD            = 6,	/* System reset due to watchdog */
	ESERVICE_STORE        = 7,	/* Storage service abnormal */
	ESERVICE_CAMERA       = 8,	/* Camera service abnormal */
	ESERVICE_AUDIO        = 9,	/* Audio service abnormal */
	ESERVICE_COMM         = 10,	/* Communication service abnormal */
	ESERVICE_DISPLAY      = 11,	/* Display service abnormal */
	EMODULE_I2C           = 12,	/* I2C error */
	EMODULE_UART          = 13,	/* Uart error */
	EMODULE_SPI           = 14,	/* SPI error */
	ESEM_WDT4_OVFLOW      = 15,	/* SEM detect watchdog4 o4erflow */
	ESEM_WDT5_OVFLOW      = 16,	/* SEM detect watchdog5 overflow */
	ESEM_WDT6_OVFLOW      = 17,	/* SEM detect watchdog6 overflow */
	ESTARTUP_AP1_PRELOAD  = 18,	/* AP1 preload startup failed */
	ESTARTUP_AP2_PRELOAD  = 19,	/* AP2 preload startup failed */
	ERRCODE_MAX,
} errcode_t;
#endif

/**
 * @brief Audio MOD code definition
 *
 */
typedef enum {
    AUDIO_MANAGER_SVC = 0,
    VIRT_I2S_HOST_SVC = 1,
    AUDIO_HUB_SVC = 2,
    SND_DRIVER = 3,
} AUDIO_SVC_MOD;

/**
 * @brief Audio error code definition
 *
 */
typedef enum {
    RET_OK = 0,
    RET_PATH_NOT_NEED_CHANGE = 1,
    RET_PATH_NOT_STARTED = 2,
    RET_PATH_VOL_CHANGED = 3,
    RET_PATH_EXCLUDED = 4,
    RET_PATH_DUCKED = 5,
    RET_ERR_PATH_INVALID_VOLUME = 6,
    RET_ERR_INVALID_PATH = 7,
    RET_ERR_INIT_FAILED = 8,
    RET_ERR_SYNC_FAILED = 9,
    RET_ERR_SETVOL_FAILED = 10,
    RET_ERR_MUTE_FAILED = 11,
    RET_ERR_PATH_INACTIVE = 12,
    RET_ERR_CHECK_FAILED = 13,
    RET_ERR_NULL_FUNC = 14,
    RET_ERR_PARAM_FAILED = 15,
    RET_ERR_VI2S_DEV_NOT_EXIST = 16,
    RET_ERR_VI2S_ID_NOT_SUPPORT = 17,
    RET_ERR_VI2S_SEND_MSG = 18,
    RET_ERR_VI2S_INIT_FAILED = 19,
    RET_ERR_VI2S_OPEN_FAILED = 20,
    RET_ERR_VI2S_START_FAILED = 21,
    RET_ERR_VI2S_STOP_FAILED = 22,
    RET_ERR_SND_UNDERRUN = 23,
    RET_ERR_SND_OVERRUN = 24,

    RET_ERR_UNKNOWN = 255,
} AM_RESULT;

typedef enum {
	CAMSERVICE_MOD,
	CSI_MOD,
	MIPI_MOD,
	SERDES_MOD,
}CAM_SERVICE_MOD;
typedef enum {
	ERROR_UNILINK_RECV_FAIL = 1,
	ERROR_UNILINK_SEND_FAIL = 2,
	ERROR_UNILINK_SOCKET_START_FAIL = 3,
	ERROR_UNILINK_BIND_PORT_FAIL = 4,
	ERROR_UNILINK_SET_TIMEOUT_FAIL = 5,
	ERROR_UNILINK_SEND_BUF_FAIL = 6,
	ERROR_MBOX_SEND_FAIL = 7,
	ERROR_CAM_UNSUPPORT_FORMAT = 8,
	ERROR_CAM_INVALID_PARAM = 9,
	ERROR_CAM_ALREADY_INUSE = 10,
	ERROR_CAM_INIT_FAIL = 11,
	ERROR_CAM_GET_RES_FAIL = 12,
	ERROR_CAM_POWER_FAIL = 13,
	ERROR_CAM_I2C_FAIL = 14,
	ERROR_CAM_ENUM_BUSFMT_FAIL = 15,
	ERROR_CAM_ENUM_FRAMESIZE_FAIL = 16,
	ERROR_CSI_INIT_FAIL = 17,
	ERROR_CSI_INVALID_ID = 18,
	ERROR_CSI_OPEN_FAIL = 19,
	ERROR_CSI_GETRES_FAIL = 20,
	ERROR_CSI_CFG_FAIL = 21,
	ERROR_CSI_ACTIVEQ_EMPTY = 22,
	ERROR_CSI_ACTIVEQ_FULL = 23,
	ERROR_CSI_VBUF_EMPTY = 24,
	ERROR_CSI_DQ_TIMEOUT = 25,
	ERROR_CSI_UNSUPPORTED_BUS_TYPE = 26,
	ERROR_CSI_UNSUPPORTED_FIELD_TYPE = 27,
	ERROR_CSI_UNSUPPORTED_IMAGE_SIZE = 28,
	ERROR_CSI_UNSUPPORTED_CROP_SIZE = 29,
	ERROR_CSI_UNSUPPORTED_OUTFMT = 30,
	ERROR_MIPI_CFG_FAIL = 31,
	ERROR_MIPI_CHECK_VERSION_FAIL = 32,
	ERROR_MIPI_UNSUPPORTED_FREQ = 33,
	ERROR_MIPI_GETRES_FAIL = 34,
	ERROR_CAMSERVICE_LOCAL_USER_NULL = 35,
	ERROR_CAMSERVICE_REMOTE_USER_NULL = 36,
	ERROR_CAMSERVICE_REQ_DMA_FAIL = 37,
	ERROR_CAMSERVICE_WAIT_FRMIDLE_FAIL = 38,
	ERROR_CAMSERVICE_WAIT_DMADONE_FAIL = 39,
	ERROR_CAMSERVICE_DMATRANSFER_FAIL = 40,
	ERROR_CAMSERVICE_DMACHAN_NULL = 41,
	ERROR_CAMSERVICE_NOENTRY = 42,
	ERROR_CAMSERVICE_INVALID_OP = 43,
	ERROR_CAMSERVICE_INVALID_BUF_INDEX = 44,
	ERROR_CAMSERVICE_PEND_BUFQ_NULL = 45,
	ERROR_CAMSERVICE_INVALID_CAM_STS = 46,
	ERROR_CAMSERVICE_QBUF_NOT_TIMELY = 47,
	ERROR_CAMSERVICE_MBOX_REQ_CHANNEL_FAIL = 48,
	ERROR_CAMSERVICE_IOCTRL_FAIL = 49,
	ERROR_CAMSERVICE_CAM_NOT_ENABLE = 50,
	ERROR_CAMSERVICE_LOCAL_PEND_EVENTQ_EMPTY = 51,
	ERROR_CAMSERVICE_DQ_TIMEOUT = 52,
	ERROR_CAMSERVICE_DONEBUF_NULL = 53,
	ERROR_CAMSERVICE_PROBE_FAIL = 54,
    ERROR_CAMSERVICE_INIT_FAIL = 55,
    ERROR_CAMSERVICE_DCF_NOTIFY_ERROR = 56,
}CAM_RESULT;

/*
 * SPI err code definition
 */
typedef enum {
    ESPI_CONFIG         = 0,
    ESPI_INSTANCE       = 1,
    ESPI_SLAVENUM       = 2,
    ESPI_BPW            = 3,
    ESPI_CSGPIO         = 4,
    ESPI_MSGLEN         = 5,
    ESPI_TIMEOUT        = 6,
    ESPI_BUSY           = 7,
    ESPI_IRQ            = 8,
    ESPI_DMA_RX_CHAN    = 9,
    ESPI_DMA_RX_DESC    = 10,
    ESPI_DMA_TX_CHAN    = 11,
    ESPI_DMA_TX_DESC    = 12,
    ESPI_MAX            = 255,
} spi_errcode_t;

/*
 * I2C err code definition
 */
typedef enum {
    EI2C_CONFIG         = 0,
    EI2C_INSTANCE       = 1,
    EI2C_BUSCONFIG      = 2,
    EI2C_WRITE          = 3,
    EI2C_READ           = 4,
    EI2C_XFER           = 5,
    EI2C_MAX            = 255,
} i2c_errcode_t;

/*
 * OTA err code definition
 * userdata: update_cmd
 */
typedef enum {
    OTA_ERR_NO_ERR                     = 0x00,
    OTA_ERR_CREATE_IPCC_DEV_FAILED     = 0x01,
    OTA_ERR_CREATE_IPCC_CHAN_FAILED    = 0x02,
    OTA_ERR_CREATE_THREAD_FAILED       = 0x03,
    OTA_ERR_RECV_INVALID_LEN           = 0x04,
    OTA_ERR_RECV_INVALID_FLG1          = 0x05,
    OTA_ERR_RECV_INVALID_FLG2          = 0x06,
    OTA_ERR_RECV_CRC_FAILED            = 0x07,
    OTA_ERR_SEND_INVALID_LEN           = 0x08,
    OTA_ERR_SEND_DATA_FAILED           = 0x09,
    OTA_ERR_MSG_UNKNOWN_CMD            = 0x0a,
    OTA_ERR_MSG_CALLBACK_INVALID_PARA  = 0x0b,
    OTA_ERR_PTDEV_PARA_ERROR           = 0x0c,
    OTA_ERR_CMD_ALLOC_FAILED           = 0x0d,
    OTA_ERR_GENERATE_BOOT_INFO         = 0x0e,
    OTA_ERR_STOP_WDG_FAILED            = 0x10,
    OTA_ERR_INVALID_DATA_LEN           = 0x11,
    OTA_ERR_INVALID_PART_INDEX         = 0x12,
    OTA_ERR_INVALID_PART_INFO          = 0x13,
    OTA_ERR_INVALID_COPY_PARA          = 0x14,
    OTA_ERR_INVALID_CAPACITY           = 0x15,
    OTA_ERR_INVALID_TOTAL_LEN          = 0x16,
    OTA_ERR_INVALID_WRITE_CNT          = 0x17,
    OTA_ERR_INVALID_WRITE_PARA         = 0x18,
    OTA_ERR_INVALID_READ_LEN           = 0x19,
    OTA_ERR_INVALID_SEEK_ADDR          = 0x1a,
    OTA_ERR_INVALID_BOOT_INFO          = 0x1b,
    OTA_ERR_DATA_CRC_FAILED            = 0x1c,
    OTA_ERR_PART_CHK_FAILED            = 0x1d,
    OTA_ERR_PART_DEV_NULL              = 0x1e,
    OTA_ERR_PART_ENTRY_NULL            = 0x1f,
    OTA_ERR_PART_NAME_NULL             = 0x20,
    OTA_ERR_PART_GET_OFF_FAILED        = 0x21,
    OTA_ERR_PART_GET_SZ_FAILED         = 0x22,
    OTA_ERR_PART_READ_TABLE_FAILED     = 0x23,
    OTA_ERR_PART_ATTR_UNKNOWN          = 0x24,
    OTA_ERR_PART_ATTR_SUCC_INVALID     = 0x25,
    OTA_ERR_PART_ATTR_SUCC_FAILED      = 0x26,
    OTA_ERR_PART_ATTR_RETRY_INVALID    = 0x27,
    OTA_ERR_PART_ATTR_RETRY_FAILED     = 0x28,
    OTA_ERR_PART_ATTR_UNBOOT_FAILED    = 0x29,
    OTA_ERR_PART_ATTR_ACTIVE_FAILED    = 0x2a,
    OTA_ERR_PART_ATTR_UPDATE_FAILED    = 0x2b,
    OTA_ERR_WRITE_BUF_ALLOC_FAILED     = 0x2c,
    OTA_ERR_WRITE_CNT_INVALID          = 0x2d,
    OTA_ERR_STORAGE_DEV_NULL           = 0x2e,
    OTA_ERR_STORAGE_LOAD_FAILED        = 0x2f,
    OTA_ERR_STORAGE_FREE_FAILED        = 0x30,
    OTA_ERR_STORAGE_READ_FAILED        = 0x31,
    OTA_ERR_STORAGE_ERASE_FAILED       = 0x32,
    OTA_ERR_STORAGE_WRITE_FAILED       = 0x33,
    OTA_ERR_STORAGE_WRITE1_FAILED      = 0x34,
    OTA_ERR_STORAGE_WRITE2_FAILED      = 0x35,
    OTA_ERR_STORAGE_DEV_MAPPED_NULL    = 0x36,
    OTA_ERR_HANDOVER_OSPI_FAILED       = 0x37,
    OTA_ERR_CFG_NOT_EXIST              = 0x38,
    OTA_ERR_CFG_CALLOC_UM              = 0x39,
    OTA_ERR_STORAGE_TYPE_ERRPR         = 0x40,
    OTA_ERR_START_IPCC_ERRPR           = 0x41,
    OTA_ERR_BASE_PARTITION_ERROR       = 0x42,
    OTA_ERR_PTDEV_ALLOC_FAILED         = 0x43,
    OTA_ERR_UNKNOWN_ERROR              = 0x44,
    OTA_ERR_MAX,
} errcode_ota_t;

/*
 * Reboot err code definition
 * userdata: wdg sem sig
 */
typedef enum {
    RESET_ERR_NO_ERR                   = 0x00,
    RESET_ERR_HW_WATCHDOG              = 0x01,
    RESET_ERR_SW_PANIC                 = 0x02,
    RESET_ERR_UNHANDLE_SIG             = 0x03,
    RESET_ERR_INVALID_ARGS             = 0x10,
    RESET_ERR_VERIFY_IMG_FAILED        = 0x11,
    RESET_ERR_SEEK_IMG_FAILED          = 0x12,
    RESET_ERR_BAD_HANDLE               = 0x13,
    RESET_ERR_UNKNOWN                  = 0x14,
    RESET_ERR_INVALID_FORMAT           = 0x15,
    RESET_ERR_MAX,
} errcode_reset_t;

/*
 * Startup err code definition
 * userdata: cpu_id
 */
typedef enum {
    STARTUP_ERR_NO_ERR                 = 0x00,
    STARTUP_ERR_INVALID_ARGS           = 0x01,
    STARTUP_ERR_VERIFY_IMG_FAILED      = 0x02,
    STARTUP_ERR_SEEK_IMG_FAILED        = 0x03,
    STARTUP_ERR_BAD_HANDLE             = 0x04,
    STARTUP_ERR_UNKNOWN                = 0x05,
    STARTUP_ERR_NO_ENTRY               = 0x06,
    STARTUP_ERR_INVALID_FORMAT         = 0x07,
    STARTUP_ERR_MAX,
} errcode_startup_t;

/*
 * mmc err code definition
 */
typedef enum {
    MMC_ERR_NO_ERR = 0x00,
    MMC_ERR_TIMEOUT,
    MMC_ERR_STATUS_BUSY,
    MMC_ERR_CMD,
    MMC_ERR_DAT,
    MMC_ERR_MISC,
    MMC_ERR_CMD_ABORT,
    MMC_ERR_DAT_ABORT,
    MMC_ERR_TUNING,
    MMC_ERR_CARD_STATUS,
    MMC_ERR_SWITCH_FUNC,
    MMC_ERR_CMD_RSP,
    MMC_ERR_DEV_TYPE,
    MMC_ERR_UNKNOWN,
    MMC_ERR_MAX,
} errcode_mmc_t;

/*
 * ospi err code definition
 */
typedef enum {
    OSPI_ERR_NO_ERR = 0x00,
    OSPI_ERR_TIMEOUT,
    OSPI_ERR_STATUS_BUSY,
    OSPI_ERR_INVALID_ARGUMENT,
    OSPI_ERR_DMA_REQ_FAIL,
    OSPI_ERR_DMA_TRANS_SETUP_FAIL,
    OSPI_ERR_TRAINING_FAIL,
    OSPI_ERR_MAX,
} errcode_ospi_t;

/*
 * display err code definition
 *
 */
typedef enum {
    DISP_ERRCODE_PANEL_IFTYPE = 1,
    DISP_ERRCODE_PROBE = 2,
    DISP_ERRCODE_SETCLK = 3,
    DISP_ERRCODE_CLEAR_FLC = 4,
    DISP_ERRCODE_VSYNC_TIMEOUT = 5,
    DISP_ERRCODE_TCON_UNDERRUN = 6,
    DISP_ERRCODE_INPUT_NBUFS = 7,
    DISP_ERRCODE_INPUT_BUF = 8,
    DISP_ERRCODE_LAYER_TYPE = 9,
} disp_errcode_t;

typedef enum {
    DSI_ERRCODE_OPEN = 1,
    DSI_ERRCODE_CONFIG = 2,
    DSI_ERRCODE_COLORCODING = 3,
    DSI_ERRCODE_OUTPUT_FREQ = 4,
    DSI_ERRCODE_DPHY_LOCK = 5,
} dsi_errcode_t;

typedef enum {
    G2D_ERRCODE_ADDR_ERR = 1,
    G2D_ERRCODE_RESINFO_ERR = 2,
    G2D_ERRCODE_INS_ERR = 3,
    G2D_ERRCODE_LAYER_TYPE = 4,
    G2D_ERRCODE_PORTER_DUF = 5,
    G2D_ERRCODE_LAYER_INDEX = 6,
} g2d_errcode_t;

/*
 * UART err code definition
 */
typedef enum
{
    UART_ERR_NO_ERR = 0U,
    UART_ERR_OCCUPIED = 1U,
    UART_ERR_NOTFIND = 2U,
    UART_ERR_NOINSTANCE = 3U,
    UART_ERR_CONFIGERR = 4U,
    UART_ERR_IRQNOTFIND = 5U,
    UART_ERR_MAX,
} uart_errcode_t;

/*
 * COMM DCF&IPCC err code definition
 */
enum {
    COM_DCF_ERRCODE_FIRST=0x2000,
    COM_DCF_NO_HANDLE = COM_DCF_ERRCODE_FIRST,
    COM_DCF_NOT_READY,
    COM_DCF_ERRCODE_LAST=0x2fff,
};

enum {
    COM_IPCC_ERRCODE_FIRST=0x1000,
    COM_IPCC_MALLOC_FAIL = COM_IPCC_ERRCODE_FIRST,
    COM_IPCC_THREAD_FAIL,
    COM_IPCC_INVALID_ARGS,
    COM_IPCC_HAL_CREATE_NO_CLIENT,
    COM_IPCC_HAL_CREATE_NO_CHAN,
    COM_IPCC_HAL_CREATE_NO_POOL,
    COM_IPCC_HAL_RECV_MSG_DROP,
    COM_IPCC_HAL_SEND_FAIL,
    COM_IPCC_QUEUE_POP_TIMEOUT,
    COM_IPCC_QUEUE_RECV_MSG_TOO_LONG,
    COM_IPCC_RPMSG_NO_EPT,
    COM_IPCC_RPMSG_INIT_FAIL,
    COM_IPCC_RPMSG_CREATE_NODEV,
    COM_IPCC_RPMSG_CREATE_TOO_MUCH_EPT,
    COM_IPCC_RPMSG_CREATE_EXIST,
    COM_IPCC_RPMSG_SEND_NOT_READY,
    COM_IPCC_RPMSG_SEND_NO_BUF,
    COM_IPCC_RPMSG_SEND_TOO_LONG,
    COM_IPCC_RPMSG_COPY_TOO_LONG,
    COM_IPCC_RPMSG_COPY_NO_BUF,
    COM_IPCC_RPC_CLIENT_INIT_FAIL,
    COM_IPCC_RPC_CALL_FAIL_SEND,
    COM_IPCC_RPC_CALL_FAIL_RECV,
    COM_IPCC_RPC_CALL_FAIL,
    COM_IPCC_RPC_NO_CALL,
    COM_IPCC_RPC_REPLY_FAIL,
    COM_IPCC_RPC_SERVER_INIT_UNSUPPORTED,
    COM_IPCC_IPCC_PROBE_ALLOC_FAIL,
    COM_IPCC_IPCC_PROBE_DEV_BUSY,
    COM_IPCC_IPCC_PROBE_RPMSG_FAIL,
    COM_IPCC_IPCC_PROBE_BIND_FAIL,
    COM_IPCC_IPCC_GET_NODEV,
    COM_IPCC_IPCC_GET_TIMEOUT,
    COM_IPCC_IPCC_RX_NOBUF,
    COM_IPCC_IPCC_RX_DROP,
    COM_IPCC_IPCC_CHAN_EPT_FAIL,
    COM_IPCC_IPCC_CHAN_POOL_FAIL,
    COM_IPCC_IPCC_SEND_FAIL,
    COM_IPCC_IPCC_RECV_FAIL,
    COM_IPCC_IPCC_IMPORT_FAIL,
    COM_IPCC_IPCC_ECHO_RECV_FAIL,
    COM_IPCC_IPCC_ECHO_IMPORT_FAIL,
    COM_IPCC_ERRCODE_LAST=0x1fff,
};

/*
 * tmr err code definition
 */
typedef enum
{
    TMR_GLOBAL_CASCASE_MODE_ERR = 0U,
    TMR_DEV_STATUS_ERR,
    TMR_ERR_MAX,
} timer_errcode_t;

/*
 * eth err code definition
 */
typedef enum
{
    ETH_DWMAC_DMA_RESET_ERR = 0U,
    ETH_EQOS_ALLOC_DESCS_ERR,
    ETH_TX_LEN_ERR,
    ETH_TX_TIMEOUT_ERR,
    ETH_EQOS_DESC3_OWN_ERR,
    ETH_RX_DESC_DES3_LEN_ERR,
    ETH_IRQ_STATUS_ERR,
    EHT_POLL_REG_BITS_TIMEOUT_ERR,
    ETH_DWMAC_DMA_RESET_TIMEOUT_ERR,
    ETH_POLL_MDIO_BUSY_TIMEOUT_ERR,
    ETH_ERR_MAX,
} eth_errcode_t;

/*
 * pwm err code definition
 */
typedef enum {
    PWM_ERR_NO_ERR = 0U,
    PWM_ERR_OCCUPIED = 1U,
    PWM_ERR_NOTFOUND = 2U,
    PWM_ERR_INVALID_CHN = 3U,
    PWM_ERR_INVALID_OUT = 4U,
    PWM_ERR_INVALID_DRIVE_MODE = 5U,
    PWM_ERR_INVALID_ALIGN_MODE = 6U,
    PWM_ERR_INVALID_GROUP_MODE = 7U,
    PWM_ERR_INVALID_OVF_VAL = 8U,
    PWM_ERR_INVALID_DUTY = 9U,
    PWM_ERR_MAX,
} pwm_errcode_t;

/*
 * DMA err code definition
 */
typedef enum
{
    DMA_ERR_NO_ERR = 0U,
    DMA_ERR_NOT_SUPPORT = 1U,
    DMA_ERR_NO_CHAN_AVAILABLE = 2U,
    DMA_ERR_INVALID_ADDR = 3U,
    DMA_ERR_NO_LLI_RES_AVAILABLE = 4U,
    DMA_ERR_INVALID_ARGUMENTS = 5U,
    DMA_ERR_INVALID_STATE = 6U,
    DMA_ERR_INVALID_MUX_ID = 7U,
    DMA_ERR_INTERRUPT = 8U,
    DMA_ERR_MAX,
} dma_errcode_t;

/*
 * CAN err code definition
 */
typedef enum
{
    CAN_ERR_NO_RPDEV = 0x01,
    CAN_ERR_NO_RPCHAN = 0x02,
    CAN_ERR_NO_THREAD = 0x03,
    CAN_ERR_RX_FAIL = 0x04,
    CAN_ERR_TX_FAIL = 0x05,
    CAN_ERR_TIME_OUT = 0x06,
    CAN_ERR_NOT_OK = 0x07,
    CAN_ERR_BUSY = 0x08,
    CAN_ERR_MAX,
} can_errcode_t;

/*
 * shm error code definition
 */
typedef enum {
    SHM_INIT_FAIL = 0u,
    SHM_NOT_READY,
    SHM_ALLOC_FAIL,
    SHM_FIND_FAIL,
    SHM_LOCK_FAIL,
} shm_errcode_t;

#endif //_SYS_ERRCODE_H_
