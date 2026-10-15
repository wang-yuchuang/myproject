#include <driver/flash.h>
#include <driver/spi_nor.h>

#define FLASH_ADDR_MASK(base)    ((base) & 0xFFFFFFF)

typedef struct flash_wrapper {
    struct spi_nor *device;
    uint32_t sector_size;
    uint32_t page_size;
    flash_size_t flash_size;
} flash_wrapper_t;

flash_wrapper_t *flash_wrapper_init(struct spi_nor *device);
int flash_wrapper_read(flash_wrapper_t *flash_wrapper, flash_addr_t addr,
                       uint8_t *buf, flash_size_t size);
int flash_wrapper_write(flash_wrapper_t *flash_wrapper, flash_addr_t addr,
                        const uint8_t *buf, flash_size_t size);
int flash_wrapper_erase(flash_wrapper_t *flash_wrapper, flash_addr_t addr,
                        flash_size_t size);