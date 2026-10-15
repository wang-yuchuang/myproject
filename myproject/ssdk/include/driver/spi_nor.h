/**
 * @file spi_nor.h
 * @brief spi norflash api header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef SPI_NOR_H_
#define SPI_NOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <arch/irq.h>
#include <spinlock.h>
#include <types.h>
#include "driver/clk.h"
#include "driver/flash.h"

#undef BIT
#define BIT(nr) (1u << (nr))

#if defined(CONFIG_CDNS_OSPI_MST_SLV)
#define SNOR_READID_DUMMY_LSB (24u)
#define SNOR_OPCODE_PROTO_LSB (16u)
#define SNOR_DTR_PROTO       BIT(12)
#else
#define SNOR_READID_DUMMY_LSB (24u)
#define SNOR_OPCODE_PROTO_LSB (24u)
#define SNOR_DTR_PROTO       BIT(16)
#endif

#define SNOR_INST_LANS_PROTO_LSB  (8u)
#define SNOR_INST_LANS_PROTO_MASK (0xFu)
#define SNOR_INST_SINGLE_LANS     (0u << SNOR_INST_LANS_PROTO_LSB)
#define SNOR_INST_DUAL_LANS       (1u << SNOR_INST_LANS_PROTO_LSB)
#define SNOR_INST_QUAD_LANS       (2u << SNOR_INST_LANS_PROTO_LSB)
#define SNOR_INST_OCTAL_LANS      (3u << SNOR_INST_LANS_PROTO_LSB)
#define SNOR_INST_LANS(x)         (((x) >> SNOR_INST_LANS_PROTO_LSB) & \
                                        (SNOR_INST_LANS_PROTO_MASK))

#define SNOR_ADDR_LANS_PROTO_LSB  (4u)
#define SNOR_ADDR_LANS_PROTO_MASK (0xFu)
#define SNOR_ADDR_SINGLE_LANS     (0u << SNOR_ADDR_LANS_PROTO_LSB)
#define SNOR_ADDR_DUAL_LANS       (1u << SNOR_ADDR_LANS_PROTO_LSB)
#define SNOR_ADDR_QUAD_LANS       (2u << SNOR_ADDR_LANS_PROTO_LSB)
#define SNOR_ADDR_OCTAL_LANS      (3u << SNOR_ADDR_LANS_PROTO_LSB)
#define SNOR_ADDR_LANS(x)         (((x) >> SNOR_ADDR_LANS_PROTO_LSB) & \
                                        (SNOR_ADDR_LANS_PROTO_MASK))


#define SNOR_DATA_LANS_PROTO_LSB  (0u)
#define SNOR_DATA_LANS_PROTO_MASK (0xFu)
#define SNOR_DATA_SINGLE_LANS     (0u << SNOR_DATA_LANS_PROTO_LSB)
#define SNOR_DATA_DUAL_LANS       (1u << SNOR_DATA_LANS_PROTO_LSB)
#define SNOR_DATA_QUAD_LANS       (2u << SNOR_DATA_LANS_PROTO_LSB)
#define SNOR_DATA_OCTAL_LANS      (3u << SNOR_DATA_LANS_PROTO_LSB)
#define SNOR_DATA_LANS(x)         (((x) >> SNOR_DATA_LANS_PROTO_LSB) & \
                                        (SNOR_DATA_LANS_PROTO_MASK))


#define SNOR_PROTO_1_1_1                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_SINGLE_LANS | SNOR_DATA_SINGLE_LANS)
#define SNOR_PROTO_1_1_2                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_SINGLE_LANS | SNOR_DATA_DUAL_LANS)
#define SNOR_PROTO_1_1_4                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_SINGLE_LANS | SNOR_DATA_QUAD_LANS)
#define SNOR_PROTO_1_1_8                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_SINGLE_LANS | SNOR_DATA_OCTAL_LANS)
#define SNOR_PROTO_1_2_2                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_DUAL_LANS | SNOR_DATA_DUAL_LANS)
#define SNOR_PROTO_1_4_4                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_QUAD_LANS | SNOR_DATA_QUAD_LANS)
#define SNOR_PROTO_1_8_8                                                       \
    (SNOR_INST_SINGLE_LANS | SNOR_ADDR_OCTAL_LANS | SNOR_DATA_OCTAL_LANS)
#define SNOR_PROTO_2_2_2                                                       \
    (SNOR_INST_DUAL_LANS | SNOR_ADDR_DUAL_LANS | SNOR_DATA_DUAL_LANS)
#define SNOR_PROTO_4_4_4                                                       \
    (SNOR_INST_QUAD_LANS | SNOR_ADDR_QUAD_LANS | SNOR_DATA_QUAD_LANS)
#define SNOR_PROTO_8_8_8                                                       \
    (SNOR_INST_OCTAL_LANS | SNOR_ADDR_OCTAL_LANS | SNOR_DATA_OCTAL_LANS)
#define SNOR_PROTO_1_1_1_DTR (SNOR_PROTO_1_1_1 | SNOR_DTR_PROTO)
#define SNOR_PROTO_1_2_2_DTR (SNOR_PROTO_1_2_2 | SNOR_DTR_PROTO)
#define SNOR_PROTO_1_4_4_DTR (SNOR_PROTO_1_4_4 | SNOR_DTR_PROTO)
#define SNOR_PROTO_4_4_4_DTR (SNOR_PROTO_4_4_4 | SNOR_DTR_PROTO)
#define SNOR_PROTO_1_1_8_DTR (SNOR_PROTO_1_1_8 | SNOR_DTR_PROTO)
#define SNOR_PROTO_1_8_8_DTR (SNOR_PROTO_1_8_8 | SNOR_DTR_PROTO)
#define SNOR_PROTO_8_8_8_DTR (SNOR_PROTO_8_8_8 | SNOR_DTR_PROTO)

#if defined(CONFIG_CDNS_OSPI_MST_SLV)
#define SNOR_PROTO_DTR_MASK 0x1fffu
#define SNOR_PROTO_MASK     0xfffu
#else
#define SNOR_PROTO_DTR_MASK 0x1ffffu
#define SNOR_PROTO_MASK     0xffffu
#endif

#define SNOR_PROTO_DTR(x) ((x) & SNOR_PROTO_DTR_MASK)
#define SNOR_PROTO(x) ((x) & SNOR_PROTO_MASK)

#define SECTOR_4K_POST      (12)
#define SECTOR_32K_POST     (15)
#define SECTOR_64K_POST     (16)
#define SECTOR_128K_POST    (17)
#define SECTOR_256K_POST    (18)
#define SECTOR_POST_TO_SIZE(n) (1U << (n))

#define SPINOR_SECTOR_4K_SIZE (0x1000)
#define SPINOR_SECTOR_32K_SIZE (0x8000)
#define SPINOR_SECTOR_64K_SIZE (0x10000)
#define SPINOR_SECTOR_128K_SIZE (0x20000)
#define SPINOR_SECTOR_256K_SIZE (0x40000)

#define SWITCH_DEVICE_MAX_NUM (4u)

typedef enum {
    SPI_NOR_SECTOR_4KB = 0,
    SPI_NOR_SECTOR_32KB,
    SPI_NOR_SECTOR_64KB,
    SPI_NOR_SECTOR_128KB,
    SPI_NOR_SECTOR_256KB,
    SPI_NOR_SECTOR_TYPE_MAX,
} spi_nor_sector_type;
#define SECTOR_TYPE_TO_POST(type)   ((type) ? (14 + (type)) : 12)
#define SECTOR_TYPE_TO_SIZE(type)   SECTOR_POST_TO_SIZE(SECTOR_TYPE_TO_POST(type))

struct spi_nor;

// error code
#ifndef SDRV_ERROR_STATUS

#define SDRV_ERROR_STATUS(group, code) (-((group) * (1000U) + (code)))
#define SDRV_STATUS_GROUP_COMMON 0
#define SDRV_STATUS_GROUP_XSPI 37
enum {
    SDRV_STATUS_OK = SDRV_ERROR_STATUS(SDRV_STATUS_GROUP_COMMON, 0),             /* Common status for OK. */
    SDRV_STATUS_FAIL = SDRV_ERROR_STATUS(SDRV_STATUS_GROUP_COMMON, 1),           /* Common status for fail. */
    SDRV_STATUS_BUSY = SDRV_ERROR_STATUS(SDRV_STATUS_GROUP_COMMON, 2),           /* Common status for busy. */
    SDRV_STATUS_TIMEOUT = SDRV_ERROR_STATUS(SDRV_STATUS_GROUP_COMMON, 3),        /* Common status for timeout. */
    SDRV_STATUS_INVALID_PARAM = SDRV_ERROR_STATUS(SDRV_STATUS_GROUP_COMMON, 4),  /* Common status for invalid paramemt. */
};

#endif  // SDRV_ERROR_STATUS

/**
* @brief sub group
*/
typedef enum {
    SPI_NOR_COMMON = 0, // ret = OK
    SPI_NOR_INIT,
    SPI_NOR_ERASE,
    SPI_NOR_WRITE,
    SPI_NOR_READ,
    SPI_NOR_CANCEL,
} spi_nor_error_group_t;

/**
* @brief Common error code:
*/
typedef enum {
    SPI_NOR_OK = 0,
    SPI_NOR_FAIL,
    SPI_NOR_BUSY,
    SPI_NOR_TIMEOUT,
    SPI_NOR_UNSUPPORT,
    SPI_NOR_UNREACHABLE,
    SPI_NOR_ADDRESS_INVALID,
    SPI_NOR_LENGTH_INVALID,
    SPI_NOR_COMMON_CODE_MAX,
} spi_nor_common_error_code_t;

/**
* @brief error code for init
*/
typedef enum {
    SPI_NOR_INIT_TRAIN_E = SPI_NOR_COMMON_CODE_MAX,
} spi_nor_init_error_code_t;

#define SPI_NOR_OK_STATUS                     SDRV_STATUS_OK
#define SPI_NOR_ERR_CODE(g, c)                (((((g) & 0xF) << 5) | ((c) & 0x1F)) * !!(c))
#define SPI_NOR_ERR_STATUS(g, c)              (int)(SDRV_ERROR_STATUS(SDRV_STATUS_GROUP_XSPI, SPI_NOR_ERR_CODE((g), (c)))  * !!(c))

enum spi_nor_xfer_mode {
    SPI_NOR_XFER_POLLING_MODE = 0,
    SPI_NOR_XFER_INTERRUPT_MODE,
    SPI_NOR_XFER_DMA_MODE,
};

enum spi_nor_device_mode {
    SPI_NOR_DEV_SINGLE_MODE = 0,
    SPI_NOR_DEV_LOCKSTEP_MODE,
    SPI_NOR_DEV_PARALLEL_MODE,
};


#define SPI_NOR_MAX_ID_LEN 8

enum spi_nor_proto_type {
    SPI_NOR_PROTO_TYPE_READ = 0,
    SPI_NOR_PROTO_TYPE_WRITE,
    SPI_NOR_PROTO_TYPE_ERASE,
};

enum spi_memory_type {
    SPI_AUTO_DETECT = 0,
    SPI_NOR_FLASH,
    SPI_NAND_FLASH,
    SPI_HYPERFLASH,
    SPI_HYPERRAM,
    SPI_HYPERBUS_MCP,
};

struct spi_nor_cmd {
    uint16_t opcode;
    uint8_t inst_type;
    uint8_t dummy;
    uint8_t addr_bytes; /* 0 bytes(no need addr) 3 bytes or 4 bytes */
};

struct hyperbus_ca {
    uint16_t addr;
    uint16_t data;
};

struct hyperflash_cmd {
    char *name;
    bool is_read;
    uint32_t num;
    struct hyperbus_ca ca[];
};

enum spi_nor_ops {
    SPI_NOR_OPS_READ = 1,
    SPI_NOR_OPS_WRITE,
    SPI_NOR_OPS_ERASE,
    SPI_NOR_OPS_LOCK,
    SPI_NOR_OPS_UNLOCK,
};

typedef int (*flash_init_t)(void *);

/**
 * @brief spi norflash indirect/direct(r/w) flags.
 */
enum spi_nor_rw_para {
    SPI_NOR_DIRECT_READ = 0,
    SPI_NOR_INDIRECT_READ,
    SPI_NOR_DIRECT_WRITE,
    SPI_NOR_INDIRECT_WRITE,
};

struct spi_nor_config {
    uint8_t id;
    uint8_t cs;
    uint32_t baudrate;
    enum spi_nor_xfer_mode xfer_mode;
    enum spi_nor_device_mode dev_mode;
    enum spi_memory_type mem_type;     /**< flash device type */
    bool async_mode;
    bool hyperbus_mode;
    bool sw_rst;
    flash_init_t flash_init;           /**< customer init flash(sip) */
    uint32_t force_size;               /**< spi nor force size */
};

struct spi_nor_erase_map {
    uint32_t erase_proto;
    uint32_t erase_size; // in bytes
    uint32_t post;
};

struct spi_nor_erase_cmd {
    flash_addr_t addr;
    const struct spi_nor_erase_map *map;
};

struct flash_info {
    const char *name;
    uint8_t flash_id[SPI_NOR_MAX_ID_LEN];

    uint32_t read_proto;
    uint32_t write_proto;
    uint32_t erase_proto;
    uint32_t erase_proto_list[SPI_NOR_SECTOR_TYPE_MAX]; /**< support falsh erase transmission protos */

    uint8_t read_dummy;
    uint8_t write_dummy;
    uint8_t status_dummy;

    uint32_t sector_size;
    uint16_t page_size;

    flash_size_t size;
    struct spi_nor_erase_map
        erase_map[SPI_NOR_SECTOR_TYPE_MAX];   /**< Automatic filling */

    int (*default_init)(struct spi_nor *nor);
    int (*octal_dtr_enable)(struct spi_nor *nor, bool enable);
    int (*quad_enable)(struct spi_nor *nor, bool enable);
    int (*enter_quad)(struct spi_nor *nor, bool enable);
    int (*set_4byte_addr_mode)(struct spi_nor *nor, bool enable);
};

struct spi_nor_host_ops {
    int (*prepare)(struct spi_nor *flash_handle, enum spi_nor_ops ops);
    void (*unprepare)(struct spi_nor *flash_handle, enum spi_nor_ops ops);
    int (*reg_read)(struct spi_nor *flash_handle, struct spi_nor_cmd *cmd,
                    flash_addr_t addr, uint8_t *buf, flash_size_t length);
    int (*reg_write)(struct spi_nor *flash_handle, struct spi_nor_cmd *cmd,
                     flash_addr_t addr, const uint8_t *buf,
                     flash_size_t length);
    int (*read)(struct spi_nor *flash_handle, flash_addr_t addr, uint8_t *buf,
                flash_size_t length);
    int (*write)(struct spi_nor *flash_handle, flash_addr_t addr,
                 const uint8_t *buf, flash_size_t length);
    int (*erase)(struct spi_nor *flash_handle, flash_addr_t offset);
    int (*complex_erase)(struct spi_nor *nor,
                         struct spi_nor_erase_cmd *cmd); /**< spi nor erase flash cmd operation */
    int (*training)(struct spi_nor *flash_handle, flash_addr_t addr,
                    uint8_t *buf, const uint8_t *pattern, flash_size_t length);
    int (*cancel)(struct spi_nor *flash_handle);
    void (*drv_main_function)(struct spi_nor *nor);
    void (*deinit)(struct spi_nor *nor);
    void (*cache_flush)(struct spi_nor *flash_handle, flash_addr_t addr,
                        flash_size_t length); /**< spi nor host flush cache operation */
    int (*enable_rfd)(struct spi_nor *flash_handle,
                      uint8_t mask);  /**< spi nor host enable rfd operation */
    void (*xip_proto_setup)(struct spi_nor
                            *flash_handle);  /**< spi nor xip mode proto setup */
    int (*enter_xip_mode)(struct spi_nor
                          *flash_handle);  /**< spi nor enter xip mode */
};

struct hyperbus_host_ops {
    int (*read16)(struct spi_nor *flash_handle, flash_addr_t addr, uint16_t *buf);
    int (*write16)(struct spi_nor *flash_handle, flash_addr_t addr, uint16_t *buf);
    void (*set_pre_transaction)(struct spi_nor *flash_handle, bool is_read,
                                bool is_direct, uint32_t num, struct hyperbus_ca **ca_array);
    void (*hyperram_en)(struct spi_nor *flash_handle, uint8_t cs,
                        uint8_t read_dummy, uint8_t write_dummy);
    int (*read)(struct spi_nor *flash_handle, flash_addr_t addr, uint8_t *buf,
                flash_size_t length);
    int (*write)(struct spi_nor *flash_handle, flash_addr_t addr,
                 const uint8_t *buf, flash_size_t length);
    void (*cache_flush)(struct spi_nor *flash_handle, flash_addr_t addr,
                        flash_size_t length); /**< hyper flush cache operation */
};

struct spi_nor_host {
    uint8_t id;
    addr_t base;
    unsigned int irq;
    clk_node_t *clk;
    unsigned long ref_clk_hz;
    bool xip_mode;                     /**< device is xip mode */

    struct spi_nor_host_ops *ops;
    struct hyperbus_host_ops *hyper_ops;

    struct spi_nor *dev;
    void *priv_data;
    struct spi_nor *nor_tab[SWITCH_DEVICE_MAX_NUM];  /**< spi_nor binded */
    flash_size_t total_size;             /**< all flash size */
};

/**
 * @brief dma operation for xspi.
 */
typedef void (*flash_dma_config)(struct spi_nor *nor, flash_addr_t addr,
                                 uint8_t *buf,
                                 bool is_read_flag, uint32_t len, uint32_t burst_width, uint32_t burst_len);
typedef void (*flash_dma_stop)(void);
typedef void (*flash_dma_stop_signal)(struct spi_nor *nor);

struct spi_nor {
    uint8_t id;
    uint8_t cs;
    uint32_t baudrate;
    uint32_t offset_address;           /**< spi nor offset address */
    irq_state_t irq_state;
    spin_lock_t spin_lock;
    uint8_t record_wr_buff[96] __ALIGNED(32);
    enum spi_memory_type mem_type;
    enum spi_nor_device_mode dev_mode;
    struct flash_info info;           /**< flash info */
    bool phy_en;
    bool dqs_en;
    bool octal_dtr_en;
    bool async_mode;
    bool hyperbus_mode;
    bool sw_rst;
    bool mst_slv_mode;
    uint32_t reg_proto;

    uint8_t addr_width;
    uint8_t cmd_bytes;

    enum spi_nor_xfer_mode xfer_mode;
    struct flash_xfer_info xfer_info;
    struct flash_xfer_info xfer_info_bottom;
    flash_notification event_handler;

    flash_dma_config dma_xfer_config;  /**< dma config callback */
    flash_dma_stop dma_xfer_stop;  /**< dma stop callback */
    flash_dma_stop_signal
    dma_stop;           /**< dma single channel stop callback */

    struct spi_nor_host *host;
    void    *ops;                       /**< flash_ops_t */
    void *parent;
};

typedef struct {
    flash_addr_t start_addr;
    flash_size_t size;
    uint8_t cmd[4];
    bool lock_en;
} protect_config_t;


typedef struct {
    int (*fls_wait_idle)(struct spi_nor *nor);
    int (*fls_write_enable)(struct spi_nor *nor, bool enable);
    int (*fls_init)(struct spi_nor *nor, struct spi_nor_host *host,
                    const struct spi_nor_config *flash_config);
    void (*fls_deinit)(struct spi_nor *nor);
    int (*fls_read)(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                    flash_size_t size);
    int (*fls_write)(struct spi_nor *nor, flash_addr_t addr,
                     const uint8_t *buf, flash_size_t size);
    int (*fls_erase)(struct spi_nor *nor, flash_addr_t addr,
                     flash_size_t size);
    int (*fls_cancel)(struct spi_nor *nor);
    void (*fls_main_function)(struct spi_nor *nor);
} flash_ops_t;

#if defined(CONFIG_OSPI) || defined(CONFIG_XSPI)
extern flash_ops_t spi_nor_ops;
#endif
#ifdef CONFIG_HYPERBUS_MODE
extern flash_ops_t hyperbus_ops;
#endif
/**
 * @brief spi norflash get status.
 * @param[in] nor spi norflash instance contex handle.
 * @retval 0: success
 * @retval other: failed
 */
static inline int spi_nor_wait_idle(struct spi_nor *nor)
{
    if (nor && nor->ops) {
        return ((flash_ops_t *)(nor->ops))->fls_wait_idle(nor);
    }

    return SDRV_STATUS_INVALID_PARAM;
}

/**
 * @brief spi norflash write enable op.
 * @param[in] nor spi norflash instance contex handle.
 * @param[in] enable spi norflash write enable or not .
 * @retval 0: success
 * @retval other: failed
 */
static inline int spi_nor_write_enable(struct spi_nor *nor, bool enable)
{
    if (nor && nor->ops) {
        return ((flash_ops_t *)(nor->ops))->fls_write_enable(nor, enable);
    }

    return SDRV_STATUS_INVALID_PARAM;
}

/**
  * @brief spi norflash host initialize function.
  * @details This function used for before any other spi nor APIs.
  * @param[in] host_config spi norflash host config data, will not used after function return, so this param can be in stack.
  * @return void* The host platform data.
  * @retval Null: failed
  * @retval other: success.
 */
//void *spi_nor_host_init(const struct spi_nor_host_config *host_config);

/**
 * @brief spi norflash instance initialization interface.
 * @details This function must used after spi_nor_host_init and before other APIs.
 * @param[in, out] flash_handle spi norflash instance contex handle, the data memory need alloc by user.
 * @param[in] flash_config spi norflash device config data, will not used after function return, so this param can be in stack.
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static inline int spi_nor_init(struct spi_nor *flash_handle,
                               struct spi_nor_host *host,
                               const struct spi_nor_config *flash_config)
{
    if (flash_config->hyperbus_mode) {
#ifdef CONFIG_HYPERBUS_MODE
        return hyperbus_ops.fls_init(flash_handle, host, flash_config);
#else
        return SDRV_STATUS_INVALID_PARAM;
#endif
    }
    else {
#if defined(CONFIG_OSPI) || defined(CONFIG_XSPI)
        return spi_nor_ops.fls_init(flash_handle, host, flash_config);
#else
        return SDRV_STATUS_INVALID_PARAM;
#endif
    }
}

/**
 * @brief spi norflash instance destruction interface.
 * @param[in] flash_handle spi norflash instance contex handle.
 */
static inline void spi_nor_deinit(struct spi_nor *flash_handle)
{
    if (flash_handle && flash_handle->ops) {
        ((flash_ops_t *)(flash_handle->ops))->fls_deinit(flash_handle);
    }

    if (flash_handle && flash_handle->host->ops->deinit) {
        flash_handle->host->ops->deinit(flash_handle);
    }
}

/**
 * @brief spi norflash read interface.
 * @details When the buffer and read size aligned with cacheline size will provide best performance.
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] addr spi norflash addr.
 * @param[in] buf Read buffer, alloc by user.
 * @param[in] size Read size, need aligned with 4bytes for dma limit.
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static inline int spi_nor_read(struct spi_nor *flash_handle, flash_addr_t addr,
                               uint8_t *buf,
                               flash_size_t size)
{
    if (flash_handle && flash_handle->ops) {
        return ((flash_ops_t *)(flash_handle->ops))->fls_read(flash_handle, addr, buf,
                size);
    }

    return SDRV_STATUS_INVALID_PARAM;
}

/**
 * @brief spi norflash write interface.
 * @details When the buffer and write size aligned with cacheline size will provide best performance.
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] addr spi norflash addr.
 * @param[in] buf Write buffer, alloc by user.
 * @param[in] size Read size, need aligned with 4bytes for dma limit.
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static inline int spi_nor_write(struct spi_nor *flash_handle, flash_addr_t addr,
                                const uint8_t *buf, flash_size_t size)
{
    if (flash_handle && flash_handle->ops) {
        return ((flash_ops_t *)(flash_handle->ops))->fls_write(flash_handle, addr, buf,
                size);
    }

    return SDRV_STATUS_INVALID_PARAM;
}
/**
 * @brief spi norflash erase interface.
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] addr spi norflash addr.
 * @param[in] size Erase size, need aligned with sector_size what get by spi_nor_get_info API.
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static inline int spi_nor_erase(struct spi_nor *flash_handle, flash_addr_t addr,
                                flash_size_t size)
{
    if (flash_handle && flash_handle->ops) {
        return ((flash_ops_t *)(flash_handle->ops))->fls_erase(flash_handle, addr,
                size);
    }

    return SDRV_STATUS_INVALID_PARAM;
}

/**
 * @brief spi norflash async cancel interface.
 * @details Used for cancel the last tansfer what like read write or erase.
 * @param[in] flash_handle spi norflash instance contex handle.
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static inline int spi_nor_cancel(struct spi_nor *flash_handle)
{
    if (flash_handle && flash_handle->ops) {
        return ((flash_ops_t *)(flash_handle->ops))->fls_cancel(flash_handle);
    }

    return SDRV_STATUS_INVALID_PARAM;
}

/**
 * @brief spi norflash async main function interface.
 * @details When use async mode, this function need be called cyclically.
 * @param[in] flash_handle spi norflash instance contex handle.
 */
static inline void spi_nor_main_function(struct spi_nor *flash_handle)
{
    if (flash_handle && flash_handle->ops) {
        ((flash_ops_t *)(flash_handle->ops))->fls_main_function(flash_handle);
    }
}

/**
 * @brief Used for setup spi norflash async mode notification handler.
 * @details When use async mode, user can selet passive notification mode or active query mode.
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] handler flash notification handler.
 */
static inline void spi_nor_setup_handler(struct spi_nor *flash_handle,
        flash_notification handler)
{
    flash_handle->event_handler = handler;
    return;
}

/**
 * @brief Used for get spi norflash async tansfer result.
 * @param[in] flash_handle spi norflash instance contex handle.
 * @return enum flash_opt_result
 * @retval FLASH_OPT_COMPLETE: last transfer success
 * @retval FLASH_OPT_FAILED: last tansfer failed
 * @retval FLASH_OPT_PENDING: last tansfer not complete
 * @retval FLASH_OPT_INCONSISTENT: last tansfer data error
 * @retval FLASH_OPT_INVALID: last tansfer invalid
 */
static inline enum flash_opt_result spi_nor_get_result(
    struct spi_nor *flash_handle)
{
    if (flash_handle) {
        return flash_handle->xfer_info.opt_result;
    }
    else {
        return FLASH_OPT_INVALID;
    }
}

/**
 * @brief Used for get spi norflash info what like sector size, page size, and ect.
 * @param[in] flash_handle spi norflash instance contex handle.
 * @return struct flash_info* Norflash info data.
 */
static inline struct flash_info *spi_nor_get_info(struct spi_nor
        *flash_handle)
{
    if (flash_handle) {
        return &(flash_handle->info);
    }
    else {
        return NULL;
    }
}

/**
 * @brief spi nor read register
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] cmd spinor cmd ptr
 * @param[in] addr flash read address
 * @param[in] buf read to buf addr
 * @param[in] length read size
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
int spi_nor_reg_read(struct spi_nor *flash_handle,
                     struct spi_nor_cmd *cmd, flash_addr_t addr,
                     uint8_t *buf, flash_size_t length);

/**
 * @brief spi nor write register
 * @param[in] flash_handle spi norflash instance contex handle.
 * @param[in] cmd spinor cmd ptr
 * @param[in] addr flash read address
 * @param[in] buf write from buf addr
 * @param[in] length write size
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
int spi_nor_reg_write(struct spi_nor *flash_handle,
                      struct spi_nor_cmd *cmd, flash_addr_t addr,
                      uint8_t *buf, flash_size_t length);

static inline int spi_nor_enable_rfd(struct spi_nor *flash_handle,
                                     uint8_t mask)
{
    int ret;

    if (flash_handle && flash_handle->host->ops->enable_rfd) {
        flash_handle->host->ops->prepare(flash_handle, SPI_NOR_OPS_LOCK);
        ret = flash_handle->host->ops->enable_rfd(flash_handle, mask);
        flash_handle->host->ops->unprepare(flash_handle, SPI_NOR_OPS_LOCK);

        if (ret) {
            return SDRV_STATUS_FAIL;
        }
    }
    else {
        return SDRV_STATUS_INVALID_PARAM;
    }

    return SDRV_STATUS_OK;
}

static inline int spi_nor_xip_mode_enter(struct spi_nor *flash_handle)
{
    int ret;

    if (flash_handle && flash_handle->host->ops->enter_xip_mode) {
        ret = flash_handle->host->ops->enter_xip_mode(flash_handle);

        if (ret) {
            return SDRV_STATUS_FAIL;
        }

        return SDRV_STATUS_OK;
    }

    return FLASH_OPT_INVALID;
}

static inline struct spi_nor *spi_nor_get_flash_handle(struct spi_nor_host
        *host,
        flash_addr_t addr, flash_size_t size)
{
    struct spi_nor *nor;

    for (uint8_t i = 0; i < SWITCH_DEVICE_MAX_NUM ; i++) {
        nor = host->nor_tab[i];

        if (!nor) {
            continue;
        }

        if ((addr >= nor->offset_address) &&
                (addr + size) <= (nor->offset_address + nor->info.size)) {
            return nor;
        }
    }

    return NULL;
}

static inline int spi_nor_host_read(struct spi_nor_host *host,
                                    flash_addr_t addr,
                                    uint8_t *buf, flash_size_t size)
{
    struct spi_nor *flash_handle = spi_nor_get_flash_handle(host, addr, size);

    if (flash_handle) {
        addr -= flash_handle->offset_address;
        return spi_nor_read(flash_handle, addr, buf, size);
    }

    return SDRV_STATUS_FAIL;
}

static inline int spi_nor_host_erase(struct spi_nor_host *host,
                                     flash_addr_t addr,
                                     flash_size_t size)
{
    struct spi_nor *flash_handle = spi_nor_get_flash_handle(host, addr, size);

    if (flash_handle) {
        addr -= flash_handle->offset_address;
        return spi_nor_erase(flash_handle, addr, size);
    }

    return SDRV_STATUS_FAIL;
}

static inline int spi_nor_host_write(struct spi_nor_host *host,
                                     flash_addr_t addr,
                                     const uint8_t *buf, flash_size_t size)
{
    struct spi_nor *flash_handle = spi_nor_get_flash_handle(host, addr, size);

    if (flash_handle) {
        addr -= flash_handle->offset_address;
        return spi_nor_write(flash_handle, addr, buf, size);
    }

    return SDRV_STATUS_FAIL;
}

#ifdef __cplusplus
}
#endif
#endif /* SPI_NOR_H_ */
