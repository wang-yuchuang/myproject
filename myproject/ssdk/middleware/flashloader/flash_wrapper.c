#include <debug.h>
#include <param.h>

#include <flash_wrapper.h>

static flash_wrapper_t flash_wrapper_entry;

#if CONFIG_READ_BACK_BUFFER
#pragma section="FLASH_BUF"
uint8_t *g_rb_buffer = __section_begin("FLASH_BUF");
#endif

flash_wrapper_t *flash_wrapper_init(struct spi_nor *device)
{
    struct flash_info *flash_info;

    flash_info = spi_nor_get_info(device);
    if (flash_info == NULL) {
        ssdk_printf(SSDK_ERR, "Get flash info fail\n");
        return NULL;
    }

    flash_wrapper_entry.device = device;
    flash_wrapper_entry.sector_size = flash_info->sector_size;
    flash_wrapper_entry.page_size = flash_info->page_size;
    flash_wrapper_entry.flash_size = flash_info->size;

    return &flash_wrapper_entry;
}

int flash_wrapper_read(flash_wrapper_t *flash_wrapper, flash_addr_t addr,
                       uint8_t *buf, flash_size_t size)
{
    if (!IS_ALIGNED(addr, 4)) {
        ssdk_printf(SSDK_ERR, "Read address unaligned:0x%x\n", addr);
        return -1;
    }

    return spi_nor_read(flash_wrapper->device, addr, buf, size);
}

int flash_wrapper_write(flash_wrapper_t *flash_wrapper, flash_addr_t addr,
                        const uint8_t *buf, flash_size_t size)
{
    if (!IS_ALIGNED(addr, 4)) {
        ssdk_printf(SSDK_ERR, "Write address unaligned:0x%x\n", addr);
        return -1;
    }

    return spi_nor_write(flash_wrapper->device, addr, buf, size);
}

int flash_wrapper_erase(flash_wrapper_t *flash_wrapper, flash_addr_t addr,
                        flash_size_t size)
{
    uint32_t sector_size = flash_wrapper->sector_size;
    uint32_t erase_addr;
    uint32_t erase_len;

#if CONFIG_READ_BACK_BUFFER
    if (CONFIG_READ_BACK_BUFFER_SIZE < sector_size) {
        ssdk_printf(SSDK_ERR, "Config read back buffer < sector size.\n");
        return -1;
    }

    while (size) {
        if (!IS_ALIGNED(addr, sector_size)) {
            erase_addr = ROUNDDOWN(addr, sector_size);
            erase_len = MIN(ROUNDUP(addr, sector_size) - addr, size);
        }
        else {
            erase_addr = addr;
            erase_len = size > sector_size ? (size - size % sector_size) : size;
        }

        if (erase_len < sector_size) {
            if (spi_nor_read(flash_wrapper->device, erase_addr,
                             g_rb_buffer, sector_size)) {
                return -1;
            }

            if (spi_nor_erase(flash_wrapper->device, erase_addr,
                              sector_size) < 0) {
                return -1;
            }

            memset(g_rb_buffer + addr % sector_size, 0xff, erase_len);

            if (spi_nor_write(flash_wrapper->device, erase_addr,
                              g_rb_buffer, sector_size)) {
                return -1;
            }
        }
        else {
            if (spi_nor_erase(flash_wrapper->device, erase_addr, erase_len)) {
                return -1;
            }
        }

        addr += erase_len;
        size -= erase_len;
    }
    return 0;
#else
    if (IS_ALIGNED(addr, sector_size) && IS_ALIGNED(size, sector_size)) {
        return spi_nor_erase(flash_wrapper->device, addr, size);
    }

    // this case only for hyper flash with sf core
    erase_addr = ROUNDUP(addr, sector_size);
    erase_len = sector_size;
    if ((erase_addr + erase_len) <= flash_wrapper->flash_size) {
        return spi_nor_erase(flash_wrapper->device, erase_addr, erase_len);
    }
    return 0;
#endif
}